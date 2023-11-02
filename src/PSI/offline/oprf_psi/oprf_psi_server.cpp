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
#include "oprf_psi_server.h"

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>

#include "toolkits/util/include/xlog.h"
#include <iostream>
#include <chrono>

namespace oprf_psi_offline
{
    using namespace xsce_ose;
extern size_t PrintVecHash(const std::vector<uint8_t>& vec);
uint64_t GetRecevierSize(uint64_t sender_size, const std::string& ip, uint32_t port) {
    IOService ios;
    Endpoint ep(ios, ip, port, EpMode::Server, "Run");
    std::string run_name = std::to_string(port) + "Exchangesize";
    Channel ch = ep.addChannel(run_name, run_name);

    uint64_t receiver_size = 0;
    ch.recv(&receiver_size, 1);
    ch.send(sender_size);

    ch.close();
    ep.stop();
    ios.stop();
    return receiver_size;
}

SenderBaseOtResult SenderBaseOTs(
    uint64_t width, uint64_t inertalSeed, uint64_t inertalSeed1,
    std::string ip, uint32_t port) {
    IOService ios;
    Endpoint ep(ios, ip, port, EpMode::Server, "Run1");
    std::string run_name = std::to_string(port) + "Run";
    Channel ch = ep.addChannel(run_name, run_name);


    PRNG prng(oc::toBlock(inertalSeed, inertalSeed1));
    std::vector<block> ot_messages(width);
    BitVector choices(width);

    IknpOtExtReceiver ot_ext_receiver;
    ot_ext_receiver.genBaseOts(prng, ch);
    prng.get(choices.data(), choices.sizeBytes());
    ot_ext_receiver.receive(choices, ot_messages, prng, ch);

    ch.close();
    ep.stop();
    ios.stop();

    return SenderBaseOtResult{ot_messages, choices};
}

std::vector<block> ComputeSendSet(u64 h1_length_in_bytes, u64 sender_size, 
                    uint64_t commonSeed, uint64_t commonSeed1,
                    const std::vector<block> &sender_set) // sender_set: 用户数据
{
    const block common_seed = oc::toBlock(commonSeed, commonSeed1);
    PRNG common_prng(common_seed);
    block common_key;
    AES common_aes;
    common_prng.get((u8 *)&common_key, sizeof(block));
    common_aes.setKey(common_key);

    std::vector<block> send_set(sender_size);
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
    return send_set;
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
                                    const BitVector &choices, 
                                    std::unique_ptr<Channel, std::function<void(Channel*)>> &ch,
                                    std::vector<std::vector<u8> > &matrixC)
    {
        std::vector<u8> recv_matrix(height_in_bytes);

        for (u64 i = 0; i < w; ++i)
        {
            PRNG prng(ot_messages[i + w_left]);
            prng.get(matrixC[i].data(), matrixC[i].size());

            if (offline::USE_OFFLINE_CHANNEL) {
                if (offline::recv_data.size() < offline::cur_read_recv_data + height_in_bytes) {
                    std::cout << ">>>>>>>>>>Wrong recv data size:"
                                << offline::recv_data.size()
                                << ", current read:" << offline::cur_read_recv_data
                                << ", to read bytes:" << height_in_bytes;
                }
                memcpy(recv_matrix.data(), offline::recv_data.data() + offline::cur_read_recv_data, height_in_bytes);
                offline::cur_read_recv_data += height_in_bytes;
            } else {
                ch->recv(recv_matrix.data(), height_in_bytes);
            }

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

void ServerComputeHashOutputs(u64 width, u64 sender_size, uint64_t bucket2, 
        const std::vector<std::vector<u8> > &trans_hash_inputs, u64 hash_length_in_bytes,
        const std::string& ip, uint32_t port)
{
    // IOService ios;
    // Endpoint ep(ios, ip, port, EpMode::Server, "Run");
    // std::string output_name = std::to_string(port) + "output";
    // Channel ch = ep.addChannel(output_name, output_name);
    std::unique_ptr<IOService, std::function<void(IOService*)>> ios;
    std::unique_ptr<Endpoint, std::function<void(Endpoint*)>> ep;
    std::unique_ptr<Channel, std::function<void(Channel*)>> ch;
    if (!offline::USE_OFFLINE_CHANNEL) {
        ios = std::unique_ptr<IOService, std::function<void(IOService*)>>(new IOService(), [](IOService* p) {
            p->stop();
            delete p;
        });
        ep = std::unique_ptr<Endpoint, std::function<void(Endpoint*)>>(new Endpoint(*ios, ip, port, EpMode::Server, "Run"), [](Endpoint* p) {
            p->stop();
            delete p;
        });
        std::string output_name = std::to_string(port) + "output";
        auto tmp_ch = ep->addChannel(output_name, output_name);
        ch = std::unique_ptr<Channel, std::function<void(Channel*)>>(new Channel(std::move(tmp_ch)), [](Channel* p) {
            p->close();
            delete p;
        });
    }

    auto width_in_bytes = (width + 7) / 8;
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
            H.Final(hash_output);
            memcpy(sent_buff + (j - low) * hash_length_in_bytes, hash_output, hash_length_in_bytes);
        }
        // block tmp(*(block *)sent_buff);
        if (offline::USE_OFFLINE_CHANNEL) {
            // TODO: 提前resize
            if (offline::send_data.size() < offline::cur_write_send_data + sent_buff_vec.size()) {
                offline::send_data.resize(offline::cur_write_send_data + sent_buff_vec.size());
            }
            memcpy(offline::send_data.data() + offline::cur_write_send_data, sent_buff_vec.data(), sent_buff_vec.size());
            offline::cur_write_send_data += sent_buff_vec.size();
        } else {
            // ch->asyncSend(std::move(sent_buff_vec));
            auto tmp = sent_buff_vec;
            ch->asyncSend(std::move(tmp));
        }
    }
    // ch.close();
    // ep.stop();
    // ios.stop();
}

std::vector<std::vector<u8>> ServerExchangeMetrics(uint64_t commonSeed, uint64_t commonSeed1,
                    const std::string& ip, uint32_t port,
                    uint64_t height, uint64_t width,
                    uint64_t logHeight, uint64_t sender_size,
                    uint32_t bucket1, uint32_t hash1LengthInBytes,
                    const std::vector<block>& send_set,
                    const std::vector<block> &ot_messages,
                    const BitVector &choices) {
    const block common_seed = oc::toBlock(commonSeed, commonSeed1);
    PRNG common_prng(common_seed);
    block common_key;
    AES common_aes;
    common_prng.get((u8 *)&common_key, sizeof(block));
    common_aes.setKey(common_key);
    //TODO
    // IOService ios;
    // Endpoint ep(ios, ip, port, EpMode::Server, "Run");
    // std::string compute_name = std::to_string(port) + "compute";
    // Channel ch = ep.addChannel(compute_name, compute_name);
    std::unique_ptr<IOService, std::function<void(IOService*)>> ios;
    std::unique_ptr<Endpoint, std::function<void(Endpoint*)>> ep;
    std::unique_ptr<Channel, std::function<void(Channel*)>> ch;
    if (!offline::USE_OFFLINE_CHANNEL) {
        ios = std::unique_ptr<IOService, std::function<void(IOService*)>>(new IOService(), [](IOService* p) {
            p->stop();
            delete p;
        });
        ep = std::unique_ptr<Endpoint, std::function<void(Endpoint*)>>(new Endpoint(*ios, ip, port, EpMode::Server, "Run"), [](Endpoint* p) {
            p->stop();
            delete p;
        });
        std::string compute_name = std::to_string(port) + "compute";
        auto tmp_ch = ep->addChannel(compute_name, compute_name);
        ch = std::unique_ptr<Channel, std::function<void(Channel*)>>(new Channel(std::move(tmp_ch)), [](Channel* p) {
            p->close();
            delete p;
        });
    }

    auto height_in_bytes = (height + 7) / 8;
    auto location_in_bytes = (logHeight + 7) / 8;
    auto sender_size_in_bytes = (sender_size + 7) / 8;
    auto shift = (1 << logHeight) - 1;
    auto width_bucket1 = sizeof(block) / location_in_bytes;
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
    
    // ch.close();
    // ep.stop();
    // ios.stop();

    return trans_hash_inputs;
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


export std::vector<block> PreDealPSIIds(const std::vector<std::string>& ids);
int64_t OprfPsiServer::OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles, OptAlg* optAlg)
{
    uint64_t sender_size = neles;
    uint64_t receiver_size = rmtNeles;
    uint8_t *dataBuf = hashBuf;
    std::vector<block> sender_set;

    if (ids_.size() == 0) {
        if (hashLen < 128)
        {
            LOG_ERROR("Hash len is invalid, hash len:" << hashLen);
            //return false;
            return -1;
        }
        int dataByteLen = hashLen / (sizeof(uint8_t) * 8);
        sender_set.resize(sender_size);
        //prng should use 128 bit seed.
        // PRNG prng(oc::toBlock(inertalSeed, inertalSeed1));
        for (uint64_t i = 0; i < sender_size; ++i)
        {
            uint64_t *dataBase1;
            uint64_t *dataBase2;
            // senderSet[i] = prng.get<block>();
            if (8 == dataByteLen)
            {
                dataBase1 = (uint64_t *)(dataBuf + i * dataByteLen);
                sender_set[i] = oc::toBlock(*dataBase1);
            }
            else if (16 == dataByteLen)
            {
                dataBase1 = (uint64_t *)(dataBuf + i * dataByteLen);
                dataBase2 = (uint64_t *)(dataBuf + i * dataByteLen + 8);
                sender_set[i] = oc::toBlock(*dataBase1, *dataBase2);
            }
            else
            {
                sender_set[i] = oc::toBlock((i + 1) * 5);
            }
        }
    } else {
        sender_set = PreDealPSIIds(ids_);
        sender_size = sender_set.size();
        receiver_size = GetRecevierSize(sender_size, ip_, port_);
    }
    // const block block_common_seed = oc::toBlock(commonSeed, commonSeed1);
    auto sender_base_ot_result = SenderBaseOTs(width, inertalSeed, inertalSeed1, ip_, port_);
    auto send_set = ComputeSendSet(hash1LengthInBytes, sender_size, commonSeed, commonSeed1, sender_set);
    auto trans_hash_inputs = ServerExchangeMetrics(commonSeed, commonSeed1, ip_, port_, height, width, logHeight, sender_size,
                                    bucket1, hash1LengthInBytes, send_set, sender_base_ot_result.ot_messages,
                                    sender_base_ot_result.choices);

    ServerComputeHashOutputs(width, sender_size, bucket2, trans_hash_inputs, hashLengthInBytes, ip_, port_);
    //ch.close();
    //ep.stop();
    //ios.stop();
    return 0;
}

} // namespace oprf_psi_offline
