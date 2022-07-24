/**
* Copyright 2022 The XSCE Authors. All rights reserved.
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*     http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
 * @file oprf_psi_server.cpp
 * @author Created by haiyu.  2022:07:21,Thursday,00:10:48.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "oprf_psi/oprf_psi_server.h"

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>

#include "toolkits/util/include/xlog.h"

namespace oprf_psi
{
    using namespace util;

    /*
     * output: choices, ot_messages
     */
    void SenderBaseOTs(PRNG &prng, Channel &ch, BitVector &choices,
                       std::vector<block> &ot_messages)
    {
        IknpOtExtReceiver ot_ext_receiver;
        ot_ext_receiver.genBaseOts(prng, ch);
        prng.get(choices.data(), choices.sizeBytes());
        ot_ext_receiver.receive(choices, ot_messages, prng, ch);

        LOG_INFO("..Sender base OT finished");
    }

    /*
     * output send_set
     */
    void ComputeSendSet(u64 h1_length_in_bytes, u64 sender_size, const AES &common_aes,
                        const std::vector<block> &sender_set,
                        std::vector<block> &send_set)
    {
        RandomOracle h1(h1_length_in_bytes);
        u8 h1_output[h1_length_in_bytes];

        std::vector<block> aes_input(sender_size);
        std::vector<block> aes_output(sender_size);
        for (u64 i = 0; i < sender_size; ++i)
        {
            h1.Reset();
            h1.Update((u8 *)(sender_set.data() + i), sizeof(block));
            h1.Final(h1_output);

            aes_input[i] = *(block *)h1_output;
            send_set[i] = *(block *)(h1_output + sizeof(block));
        }

        common_aes.ecbEncBlocks(aes_input.data(), sender_size, aes_output.data());
        for (u64 i = 0; i < sender_size; ++i)
        {
            send_set[i] = send_set[i] ^ aes_output[i];
        }
    }

    /*
     * output: trans_locations
     */
    void ComputRandomLocations(PRNG &common_prng, AES &common_aes, block &common_key,
                               u64 sender_size, u64 bucket1, u64 w,
                               u64 location_in_bytes, u64 h1_length_in_bytes,
                               const std::vector<block> &send_set,
                               std::vector<std::vector<u8>> &trans_locations)
    {
        common_prng.get((u8 *)&common_key, sizeof(block));
        common_aes.setKey(common_key);

        block random_locations[bucket1];
        for (u64 low = 0; low < sender_size; low += bucket1)
        {
            auto up = low + bucket1 < sender_size ? low + bucket1 : sender_size;

            common_aes.ecbEncBlocks(send_set.data() + low, up - low, random_locations);

            for (u64 i = 0; i < w; ++i)
            {
                for (auto j = low; j < up; ++j)
                {
                    memcpy(trans_locations[i].data() + j * location_in_bytes, (u8 *)(random_locations + (j - low)) + i * location_in_bytes, location_in_bytes);
                }
            }
        }
    }

    /*
     * output: matrixC
     */
    void ExtendOTsAndComputeMatrixC(u64 height_in_bytes, u64 w, u64 w_left,
                                    const std::vector<block> &ot_messages,
                                    const BitVector &choices, Channel &ch,
                                    std::vector<std::vector<u8>> &matrixC)
    {
        std::vector<u8> recv_matrix(height_in_bytes);

        for (u64 i = 0; i < w; ++i)
        {
            PRNG prng(ot_messages[i + w_left]);
            prng.get(matrixC[i].data(), matrixC[i].size());

            ch.recv(recv_matrix.data(), height_in_bytes);

            if (choices[i + w_left])
            {
                for (u64 j = 0; j < height_in_bytes; ++j)
                {
                    matrixC[i][j] ^= recv_matrix[j];
                }
            }
        }
    }

    void ComputeHashInputs(u64 sender_size, u64 w, const std::vector<std::vector<u8>> &trans_locations,
                           u64 location_in_bytes, u64 shift, u64 w_left, const std::vector<std::vector<u8>> &matrixC,
                           std::vector<std::vector<u8>> &trans_hash_inputs)
    {
        for (u64 i = 0; i < w; ++i)
        {
            for (u64 j = 0; j < sender_size; ++j)
            {
                auto location = (*(u32 *)(trans_locations[i].data() + j * location_in_bytes)) & shift;

                trans_hash_inputs[i + w_left][j >> 3] |= (u8)((bool)(matrixC[i][location >> 3] & (1 << (location & 7)))) << (j & 7);
            }
        }
    }

    void OprfPsiServer::ComputeHashOutputs(u64 width_in_bytes, u64 sender_size, Timer &timer, const std::vector<std::vector<u8>> &trans_hash_inputs, u64 hash_length_in_bytes, Channel &ch)
    {
        RandomOracle H(hash_length_in_bytes);
        std::vector<std::vector<u8>> hash_inputs(bucket2, std::vector<u8>(width_in_bytes));

        for (u64 low = 0; low < sender_size; low += bucket2)
        {
            auto up = low + bucket2 < sender_size ? low + bucket2 : sender_size;

            for (auto j = low; j < up; ++j)
            {
                memset(hash_inputs[j - low].data(), 0, width_in_bytes);
            }

            for (u64 i = 0; i < width; ++i)
            {
                for (auto j = low; j < up; ++j)
                {
                    hash_inputs[j - low][i >> 3] |= (u8)((bool)(trans_hash_inputs[i][j >> 3] & (1 << (j & 7)))) << (i & 7);
                }
            }

            std::vector<u8> sent_buff_vec((up - low) * hash_length_in_bytes);

            u8 *sent_buff = sent_buff_vec.data();
            u8 hash_output[sizeof(block)];
            for (auto j = low; j < up; ++j)
            {
                H.Reset();
                H.Update(hash_inputs[j - low].data(), width_in_bytes);
                SaveOprfValues((char*)hash_inputs[j - low].data(), width_in_bytes);

                H.Final(hash_output);

                memcpy(sent_buff + (j - low) * hash_length_in_bytes, hash_output, hash_length_in_bytes);
            }

            block tmp(*(block *)sent_buff);
            ch.asyncSend(std::move(sent_buff_vec));
        }
        LOG_INFO("Sender hash outputs computed and sent");
        timer.setTimePoint("Sender hash outputs computed and sent");
    }

    void OprfPsiServer::Run(PRNG &prng, Channel &ch, block commonSeed,
                            const u64 &sender_size, const u64 &receiverSize,
                            const std::vector<block> &sender_set)
    {
        Timer timer;
        timer.setTimePoint("Sender start");

        auto height_in_bytes = (height + 7) / 8;
        auto width_in_bytes = (width + 7) / 8;
        auto location_in_bytes = (logHeight + 7) / 8;
        auto sender_size_in_bytes = (sender_size + 7) / 8;
        auto shift = (1 << logHeight) - 1;
        auto width_bucket1 = sizeof(block) / location_in_bytes;

        LOG_INFO("in snd run: height_in_bytes=" << height_in_bytes << ",width_in_bytes=" << width_in_bytes << ",location_in_bytes=" << location_in_bytes);
        LOG_INFO("sender_size_in_bytes=" << sender_size_in_bytes << ",width_bucket1=" << width_bucket1);

        //////////////////// Base OTs /////////////////////////////////
        std::vector<block> ot_messages(width);
        BitVector choices(width);
        SenderBaseOTs(prng, ch, choices, ot_messages);
        timer.setTimePoint("..Sender base OT finished");

        ////////////// Initialization //////////////////////

        PRNG common_prng(commonSeed);
        block common_key;
        AES common_aes;

        /////////// Transform input /////////////////////

        common_prng.get((u8 *)&common_key, sizeof(block));
        common_aes.setKey(common_key);

        //////////// Compute send_set
        std::vector<block> send_set(sender_size);
        ComputeSendSet(hash1LengthInBytes, sender_size, common_aes, sender_set, send_set);

        LOG_INFO("Sender set transformed");
        timer.setTimePoint("Sender set transformed");
        std::vector<std::vector<u8>> matrixC(width_bucket1, std::vector<u8>(height_in_bytes));
        std::vector<std::vector<u8>> trans_hash_inputs(width, std::vector<u8>(sender_size_in_bytes, 0));
        std::vector<std::vector<u8>> trans_locations(width_bucket1, std::vector<u8>(sender_size * location_in_bytes + sizeof(u32)));
        for (uint32_t w_left = 0; w_left < width; w_left += width_bucket1)
        {
            auto wRight = w_left + width_bucket1 < width ? w_left + width_bucket1 : width;
            auto w = wRight - w_left;

            //////////// Compute random locations (transposed) ////////////////
            ComputRandomLocations(common_prng, common_aes, common_key, sender_size, bucket1,
                                  w, location_in_bytes, hash1LengthInBytes, send_set,
                                  trans_locations);

            //////////////// Extend OTs and compute matrix C ///////////////////
            ExtendOTsAndComputeMatrixC(height_in_bytes, w, w_left, ot_messages, choices, ch, matrixC);

            ///////////////// Compute hash inputs (transposed) /////////////////////
            ComputeHashInputs(sender_size, w, trans_locations, location_in_bytes, shift,
                              w_left, matrixC, trans_hash_inputs);
        }

        LOG_INFO("Sender transposed hash input computed");
        timer.setTimePoint("Sender transposed hash input computed");

        /////////////////// Compute hash outputs ///////////////////////////

        ComputeHashOutputs(width_in_bytes, sender_size,
                           timer, trans_hash_inputs, hashLengthInBytes, ch);

        LOG_INFO("\n" << timer);
    }

    int64_t OprfPsiServer::OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles)
    {
        uint64_t senderSize = neles;
        uint64_t receiverSize = rmtNeles;
        uint8_t *dataBuf = hashBuf;

        std::string chName = "oprfPsi_000";
        if (hashLen < 128)
        {
            LOG_ERROR("Hash len is invalid, hash len:" << hashLen);
            return false;
        }
        int dataByteLen = hashLen / (sizeof(uint8_t) * 8);

        LOG_INFO("ipSender=" << ip_);
        LOG_INFO("dataByteLen=" << dataByteLen);
        LOG_INFO("common seed=" << std::hex << commonSeed << ":" << commonSeed1);
        LOG_INFO("chName=" << chName);
        LOG_INFO("oprfSenderSize=" << senderSize);

        IOService ios;
        Endpoint ep(ios, ip_, EpMode::Server, chName);

        LOG_INFO("before addChannel psi ");
        Channel ch = ep.addChannel();

        std::vector<block> senderSet(senderSize);
        PRNG prng(oc::toBlock(inertalSeed));
        for (uint64_t i = 0; i < senderSize; ++i)
        {
            uint64_t *dataBase1;
            uint64_t *dataBase2;
            // senderSet[i] = prng.get<block>();
            if (8 == dataByteLen)
            {
                dataBase1 = (uint64_t *)(dataBuf + i * dataByteLen);
                senderSet[i] = oc::toBlock(*dataBase1);
            }
            else if (16 == dataByteLen)
            {
                dataBase1 = (uint64_t *)(dataBuf + i * dataByteLen);
                dataBase2 = (uint64_t *)(dataBuf + i * dataByteLen + 8);
                senderSet[i] = oc::toBlock(*dataBase1, *dataBase2);
            }
            else
            {
                senderSet[i] = oc::toBlock((i + 1) * 5);
            }
        }
        LOG_INFO("sender size=" << senderSet.size());

        const block block_common_seed = oc::toBlock(commonSeed, commonSeed1);
        Run(prng, ch, block_common_seed, senderSize, receiverSize, senderSet);

        ch.close();
        ep.stop();
        ios.stop();
        return 0;
    }

} // namespace oprf_psi
