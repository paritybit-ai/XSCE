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
    using namespace xsce_ose;

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
                               std::vector<std::vector<u8> > &trans_locations)
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
                                    std::vector<std::vector<u8> > &matrixC)
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

    void ComputeHashInputs(u64 sender_size, u64 w, const std::vector<std::vector<u8> > &trans_locations,
                           u64 location_in_bytes, u64 shift, u64 w_left, const std::vector<std::vector<u8> > &matrixC,
                           std::vector<std::vector<u8> > &trans_hash_inputs)
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

    void OprfPsiServer::ComputeHashOutputs(u64 width_in_bytes, u64 sender_size, Timer &timer, const std::vector<std::vector<u8> > &trans_hash_inputs, u64 hash_length_in_bytes, Channel &ch)
    {
        RandomOracle H(hash_length_in_bytes);
        std::vector<std::vector<u8> > hash_inputs(bucket2, std::vector<u8>(width_in_bytes));

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
                SaveOprfValues((char *)hash_inputs[j - low].data(), width_in_bytes);

                H.Final(hash_output);

                memcpy(sent_buff + (j - low) * hash_length_in_bytes, hash_output, hash_length_in_bytes);
            }

            block tmp(*(block *)sent_buff);
            ch.asyncSend(std::move(sent_buff_vec));
        }
        LOG_INFO("Sender hash outputs computed and sent");
        timer.setTimePoint("Sender hash outputs computed and sent");
    }

    //oprf psi alg note according to figure 3 in oprf paper << Private Set Intersection in the Internet Setting From Lightweight Oblivious PRF>>   .Modified by wumingzi. 2022:08:01,Monday,21:47:07.
    /*
    1）D/A/B are all matrices of the same row/column: w means width and h means height.
    2）P1 is server/sender, P2 is client/reciever. P1 holds elements x,P2 holds elements y,
    3）two hash functions: H1 and H2: H1 converts inputs with any length to output with l1 bits,
        H2 converts inputs with w bits to output  with l2 bits,
    4) PRF function F: converts the output of H1 with l1 bits to  a w-dimension vector v(v[1],v[2],...v[w]) ,v[i]'s value is from 0 to h-1
    */
    //   .Modification over by wumingzi. 2022:08:01,Monday,21:47:11.

    void OprfPsiServer::Run(PRNG &prng, Channel &ch, block commonSeed,
                            const u64 &sender_size, const u64 &receiverSize,
                            const std::vector<block> &sender_set)
    {
        Timer timer;
        timer.setTimePoint("Sender start");

        //logHeight means the number of bits to hold the value of h
        //location_in_bytes means the location of row in matrix(A/D/B)
        //width_bucket1 means the number of rows in a block(128 bits)
        auto height_in_bytes = (height + 7) / 8;
        auto width_in_bytes = (width + 7) / 8;
        auto location_in_bytes = (logHeight + 7) / 8;
        auto sender_size_in_bytes = (sender_size + 7) / 8;
        auto shift = (1 << logHeight) - 1;
        auto width_bucket1 = sizeof(block) / location_in_bytes;

        LOG_INFO("in snd run: height_in_bytes=" << height_in_bytes << ",width_in_bytes=" << width_in_bytes << ",location_in_bytes=" << location_in_bytes);
        LOG_INFO("sender_size_in_bytes=" << sender_size_in_bytes << ",width_bucket1=" << width_bucket1);

        //  .Modified by wumingzi. 2022:08:01,Monday,22:14:14.
        /*
         server uses ot to get seed to generate matrix C,this is different with paper which transfer an entire column in each OT
         here only transfer a block in each OT and then use the block as seed to generate a column with h bits.
         w=width is the column size of matrxA/B/D. so here are width OT transfers. server is chooser,client is sender.
        */

        //TODO
        //ch.close();
        IOService ios;
        Endpoint ep(ios, ip_, port_, EpMode::Server, "Run");
        std::string run_name = std::to_string(port_) + "Run";
        ch = ep.addChannel(run_name, run_name);

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

        // calculate the key of PRF function F for each element  .Modified by wumingzi. 2022:08:01,Monday,22:22:16.
        //////////// Compute send_set
        std::vector<block> send_set(sender_size);
        ComputeSendSet(hash1LengthInBytes, sender_size, common_aes, sender_set, send_set);

        LOG_INFO("Sender set transformed");
        timer.setTimePoint("Sender set transformed");

        // matrixA/B/C/D are all tranposed to store in memory  .Modified by wumingzi. 2022:08:01,Monday,22:24:09.
        /*
        as an example of matrix D with h rows and w columns.
        original matrix is as below:(h rows, w column)
            d[1][1] d[1][2] ... d[1][w]
            d[2][1] d[2][2] ... d[2][w]

            d[3][1] d[3][2] ... d[3][w]
            d[4][1] d[4][2] ... d[4][w]
        
            d[h-1][1] d[h-1][2] ... d[h-1][w]
            d[h][1] d[h][2] ... d[h][w]

        transposed matrix is as below: (w rows ,h columns)
            d[1][1] d[2][1] ... d[h][1]
            d[1][2] d[2][2] ... d[h][2]

            d[1][3] d[2][3] ... d[h][3]
            d[1][4] d[2][4] ... d[h][4]
        
            d[1][w-1] d[2][h-1] ... d[h][w-1]
            d[1][w] d[2][h] ... d[h][w]

        */

        //trans_locations means the value of v[i], which location which row in matrixD to be set to 0 according to (c) of step 1
        //for transposed matrix, a block holds width_bucket1 rows, so the loop time = width/width_bucket1
        //matrx_delta is D
    
        //TODO
        ch.close();
        std::string compute_name = std::to_string(port_) + "compute";
        ch = ep.addChannel(compute_name, compute_name);

        std::vector<std::vector<u8> > matrixC(width_bucket1, std::vector<u8>(height_in_bytes));
        std::vector<std::vector<u8> > trans_hash_inputs(width, std::vector<u8>(sender_size_in_bytes, 0));
        std::vector<std::vector<u8> > trans_locations(width_bucket1, std::vector<u8>(sender_size * location_in_bytes + sizeof(u32)));
        for (uint32_t w_left = 0; w_left < width; w_left += width_bucket1)
        {
            auto wRight = w_left + width_bucket1 < width ? w_left + width_bucket1 : width;
            auto w = wRight - w_left;

            //////////// Compute random locations (transposed) ////////////////
            //send_set holds the the key of PRF fucntion F for calculating value v of each element of sender
            //for each element x,calculate the value v[i] and store them in trans_locations
            ComputRandomLocations(common_prng, common_aes, common_key, sender_size, bucket1,
                                  w, location_in_bytes, hash1LengthInBytes, send_set,
                                  trans_locations);

            //////////////// Extend OTs and compute matrix C ///////////////////
            //(a) of step 2 in paper,refer to note for ComputeMatrixAAndSentMatrix function of client party.
            ExtendOTsAndComputeMatrixC(height_in_bytes, w, w_left, ot_messages, choices, ch, matrixC);

            ///////////////// Compute hash inputs (transposed) /////////////////////
            // for each element x, calculate the value of C1[v[1]],C2[v[2]]....,Cw[v[w]] and store them in trans_hash_inputs
            ComputeHashInputs(sender_size, w, trans_locations, location_in_bytes, shift,
                              w_left, matrixC, trans_hash_inputs);
        }

        LOG_INFO("Sender transposed hash input computed");
        timer.setTimePoint("Sender transposed hash input computed");


        //TODO
        ch.close();
        std::string output_name = std::to_string(port_) + "output";
        ch = ep.addChannel(output_name, output_name);

        /////////////////// Compute hash outputs ///////////////////////////
        //for each element x,  use H2 to calculate oprf value ,H2(C1[v[1]],C2[v[2]]....,Cw[v[w]])
        //send all oprv value of x to client.
        ComputeHashOutputs(width_in_bytes, sender_size, timer, trans_hash_inputs, hashLengthInBytes, ch);

        //TODO
        ch.close();
        ep.stop();
        ios.stop();

        LOG_INFO("\n"
                 << timer);
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

        LOG_INFO("Server ip:" << ip_ << ", port:" << port_);
        LOG_INFO("dataByteLen=" << dataByteLen);
        LOG_INFO("common seed=" << std::hex << commonSeed << ":" << commonSeed1);
        LOG_INFO("server internal seed=" << std::hex << inertalSeed << ":" << inertalSeed1);
        LOG_INFO("chName=" << chName);
        LOG_INFO("oprfSenderSize=" << senderSize);

        LOG_INFO("senderSize=" << std::dec << senderSize << ",receiverSize=" << receiverSize);

        //IOService ios;
        //Endpoint ep(ios, ip_, port_, EpMode::Server, chName);

        LOG_INFO("before addChannel psi ");
        Channel ch; //   = ep.addChannel();

        std::vector<block> senderSet(senderSize);
        //prng should use 128 bit seed.
        PRNG prng(oc::toBlock(inertalSeed, inertalSeed1));
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

        //ch.close();
        //ep.stop();
        //ios.stop();
        return 0;
    }

} // namespace oprf_psi
