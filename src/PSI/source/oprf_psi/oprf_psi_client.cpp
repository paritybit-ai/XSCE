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
 * @file oprf_psi_client.cpp
 * @author Created by haiyu.  2022:07:21,Thursday,00:10:48.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "oprf_psi/oprf_psi_client.h"
#include <malloc.h>

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>

#include "toolkits/util/include/xlog.h"

namespace oprf_psi
{
using namespace xsce_ose;

int getMd5(unsigned char *input, uint32_t len, unsigned char *output)
{
    MD5_CTX x;
    MD5_Init(&x);
    MD5_Update(&x, (char *)input, len);
    MD5_Final(output, &x);
    return 0;
}
// size_t PrintVecHash(const std::vector<uint8_t>& vec) {
//     std::string rlt(16, 0);
//     getMd5((unsigned char *)vec.data(), vec.size(), (unsigned char *)rlt.data());
//     return std::hash<std::string>()(rlt);
// }

// size_t PrintVecHash(const std::vector<uint64_t>& vec) {
//     std::string rlt(16, 0);
//     getMd5((unsigned char *)vec.data(), vec.size() * sizeof(uint64_t), (unsigned char *)rlt.data());
//     return std::hash<std::string>()(rlt);
// }

int64_t GetSenderSize(uint64_t receiver_size, const std::string& ip, uint32_t port) {
    IOService ios;
    Endpoint ep(ios, ip, port, EpMode::Client, "Run");
    std::string run_name = std::to_string(port) + "Exchangesize";
    Channel ch = ep.addChannel(run_name, run_name);

    uint64_t sender_size = 0;
    ch.send(receiver_size);
    ch.recv(&sender_size, 1);

    ch.close();
    ep.stop();
    ios.stop();
    return sender_size;
}

std::vector<std::array<block, 2>> ReceiverBaseOTs(
    int width, uint64_t inertalSeed, uint64_t inertalSeed1,
    std::string ip, uint32_t port) {
    IOService ios;
    Endpoint ep(ios, ip, port, EpMode::Client, "Run1");
    std::string run_name = std::to_string(port) + "Run";
    Channel ch = ep.addChannel(run_name, run_name);


    PRNG prng(oc::toBlock(inertalSeed,inertalSeed1));
    std::vector<std::array<block, 2>> ot_messages(width);

    IknpOtExtSender ot_ext_sender;
    ot_ext_sender.genBaseOts(prng, ch);
    ot_ext_sender.send(ot_messages, prng, ch);

    ch.close();
    ep.stop();
    ios.stop();

    return ot_messages;
}

std::vector<block> ComputeRecvSet(u64 receiver_size, u64 h1_length_in_bytes, 
                        uint64_t commonSeed, uint64_t commonSeed1,
                        const std::vector<block> &receiver_set) // receiver_set: 用户数据
{
    const block common_seed = oc::toBlock(commonSeed, commonSeed1);
    PRNG common_prng(common_seed);
    block common_key;
    AES common_aes;
    common_prng.get((u8 *)&common_key, sizeof(block));
    common_aes.setKey(common_key);
    std::vector<block> recv_set(receiver_size);
    std::vector<block> aes_input(receiver_size);
    std::vector<block> aes_output(receiver_size);
    //RandomOracle is implemented by blake2 hash fucntion.
    RandomOracle h1(h1_length_in_bytes);
    u8 h1_output[h1_length_in_bytes];
    for (u64 i = 0; i < receiver_size; ++i)
    {
        h1.Reset();
        h1.Update((u8 *)(receiver_set.data() + i), sizeof(block));
        h1.Final(h1_output);
        aes_input[i] = *(block *)h1_output;
        recv_set[i] = *(block *)(h1_output + sizeof(block));
    }
    common_aes.ecbEncBlocks(aes_input.data(), receiver_size, aes_output.data());
    for (u64 i = 0; i < receiver_size; ++i)
    {
        recv_set[i] = recv_set[i] ^ aes_output[i];
    }
    return recv_set;
}
    
    /*
     * output: trans_locations
     */
    void ComputeRandomLocations(PRNG &common_prng, AES &common_aes, block &common_key, u64 receiver_size,
                                u64 bucket1, const std::vector<block> &recv_set, u64 location_in_bytes, u64 w,
                                std::vector<std::vector<u8>> &trans_locations)

    {
        common_prng.get((u8 *)&common_key, sizeof(block));
        common_aes.setKey(common_key);

        block randomLocations[bucket1];
        for (u64 low = 0; low < receiver_size; low += bucket1)
        {

            auto up = low + bucket1 < receiver_size ? low + bucket1 : receiver_size;

            common_aes.ecbEncBlocks(recv_set.data() + low, up - low, randomLocations);

            for (u64 i = 0; i < w; ++i)
            {
                for (auto j = low; j < up; ++j)
                {
                    memcpy(trans_locations[i].data() + j * location_in_bytes, (u8 *)(randomLocations + (j - low)) + i * location_in_bytes, location_in_bytes);
                }
            }
        }
    }

    /*
     * output: matrix_delta
     */
    void ComputMatrixDelta(u64 width_bucket1, u64 height_in_bytes, u64 w, u64 receiver_size, u64 shift,
                           u64 location_in_bytes, std::vector<std::vector<u8>> &trans_locations,
                           std::vector<std::vector<u8>> &matrix_delta)
    {
        for (u64 i = 0; i < width_bucket1; ++i)
        {
            memset(matrix_delta[i].data(), 255, height_in_bytes);
        }

        for (u64 i = 0; i < w; ++i)
        {
            for (u64 j = 0; j < receiver_size; ++j)
            {
                auto location = (*(u32 *)(trans_locations[i].data() + j * location_in_bytes)) & shift;

                matrix_delta[i][location >> 3] &= ~(1 << (location & 7));
            }
        }
    }

    /*
     * output matrixA
     */
    void ComputeMatrixAAndSentMatrix(std::vector<std::array<block, 2>> &ot_messages, u64 w, u64 w_left,
                                     u64 height_in_bytes, std::vector<std::vector<u8>> &matrix_delta,
                                     std::unique_ptr<Channel, std::function<void(Channel*)>> &ch,
                                     std::vector<std::vector<u8>> &matrixA)
    {
        for (u64 i = 0; i < w; ++i)
        {
            PRNG prng(ot_messages[i + w_left][0]);
            prng.get(matrixA[i].data(), height_in_bytes);

            std::vector<u8> sent_matrix_vec(height_in_bytes);
            prng.SetSeed(ot_messages[i + w_left][1]);
            prng.get(sent_matrix_vec.data(), height_in_bytes);

            for (u64 j = 0; j < height_in_bytes; ++j)
            {
                sent_matrix_vec[j] ^= matrixA[i][j] ^ matrix_delta[i][j];
            }

            if (offline::USE_OFFLINE_CHANNEL) {
                // TODO: 提前resize
                if (offline::send_data.size() < offline::cur_write_send_data + sent_matrix_vec.size()) {
                    offline::send_data.resize(offline::cur_write_send_data + sent_matrix_vec.size());
                }
                memcpy(offline::send_data.data() + offline::cur_write_send_data, sent_matrix_vec.data(), sent_matrix_vec.size());
                offline::cur_write_send_data += sent_matrix_vec.size();
            } else {
                ch->asyncSend(std::move(sent_matrix_vec));
            }
        }
    }

    /*
     * output: trans_hash_inputs
     */
    void ComputeHashInputs(u64 w, u64 receiver_size, const std::vector<std::vector<u8>> &trans_locations,
                           u64 location_in_bytes, u64 shift, u64 w_left, std::vector<std::vector<u8>> &matrixA,
                           std::vector<std::vector<u8>> &trans_hash_inputs)
    {
        for (u64 i = 0; i < w; ++i)
        {
            for (u64 j = 0; j < receiver_size; ++j)
            {
                auto location = (*(u32 *)(trans_locations[i].data() + j * location_in_bytes)) & shift;

                trans_hash_inputs[i + w_left][j >> 3] |= (u8)((bool)(matrixA[i][location >> 3] & (1 << (location & 7)))) << (j & 7);
            }
        }
    }

    /*
     * output: all_hashes
     */
std::unordered_map<u64, std::vector<std::pair<block, u32>>> ClientComputeHashOutputs(u64 hash_length_in_bytes, uint64_t bucket2, u64 width, u64 receiver_size,
                    const std::vector<std::vector<u8>> &trans_hash_inputs, std::vector<xsce_ose::block>* oprf_values)
{
    std::unordered_map<u64, std::vector<std::pair<block, u32>>> all_hashes;
    auto width_in_bytes = (width + 7) / 8;
    RandomOracle H(hash_length_in_bytes);
    u8 hash_output[sizeof(block)];
    std::vector<std::vector<u8>> hash_inputs(bucket2, std::vector<u8>(width_in_bytes));
    for (u64 low = 0; low < receiver_size; low += bucket2)
    {
        auto up = low + bucket2 < receiver_size ? low + bucket2 : receiver_size;
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
        for (auto j = low; j < up; ++j)
        {
            H.Reset();
            H.Update(hash_inputs[j - low].data(), width_in_bytes);
            SaveOprfValues((char*)hash_inputs[j - low].data(), width_in_bytes, oprf_values);
            H.Final(hash_output);
            all_hashes[*(u64 *)hash_output].push_back(std::make_pair(*(block *)hash_output, j));
        }
    }
    return all_hashes;
}

    /*
     * output: rlt_vec
     */
std::vector<std::pair<uint64_t, uint64_t>> ReceiveHashOutputsAndComputePSI(uint64_t width, uint64_t receiver_size,
                        const std::vector<std::vector<u8>> &trans_hash_inputs,
                        u64 bucket2, u64 hash_length_in_bytes, u64 sender_size,
                        const std::string& ip, uint32_t port,
                        Logger* logger, std::vector<xsce_ose::block>* oprf_values)
{
    auto all_hashes = ClientComputeHashOutputs(hash_length_in_bytes, bucket2, width, receiver_size, trans_hash_inputs, oprf_values);

    // std::vector<uint64_t> rlt_vec;
    std::vector<std::pair<uint64_t, uint64_t>> rlt_vec;

    // IOService ios;
    // Endpoint ep(ios, ip, port, EpMode::Client, "Run");
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
        ep = std::unique_ptr<Endpoint, std::function<void(Endpoint*)>>(new Endpoint(*ios, ip, port, EpMode::Client, "Run"), [](Endpoint* p) {
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

    std::vector<u8> recv_buff_vec(bucket2 * hash_length_in_bytes);
    u8 *recv_buff = recv_buff_vec.data();
    auto psi = 0;
    for (u64 low = 0; low < sender_size; low += bucket2)
    {
        auto up = low + bucket2 < sender_size ? low + bucket2 : sender_size;
        auto recv_size = (up - low) * hash_length_in_bytes;
        if (offline::USE_OFFLINE_CHANNEL) {
            if (offline::recv_data.size() < offline::cur_read_recv_data + recv_size) {
                std::cout << ">>>>>>>>>>Wrong recv data size:"
                                << offline::recv_data.size()
                                << ", current read:" << offline::cur_read_recv_data
                                << ", to read bytes:" << recv_size;
            }
            memcpy(recv_buff, offline::recv_data.data() + offline::cur_read_recv_data, recv_size);
            offline::cur_read_recv_data += recv_size;
        } else {
            ch->recv(recv_buff, recv_size);
        }
        for (unsigned long idx = 0; idx < up - low; ++idx)
        {
            u64 map_idx = *(u64 *)(recv_buff + idx * hash_length_in_bytes);
            auto found = all_hashes.find(map_idx);
            if (found == all_hashes.end())
                continue;
            for (size_t i = 0; i < found->second.size(); ++i)
            {
                if (memcmp(&(found->second[i].first), recv_buff + idx * hash_length_in_bytes, hash_length_in_bytes) == 0)
                {
                    ++psi;
                    // rlt_vec.push_back(idx + low);
                    // rlt_vec.push_back(found->second[i].second);
                    rlt_vec.emplace_back(idx + low, found->second[i].second);
                    //client hash output to generate 128bit aes key.
                    break;
                }
            }
        }
    }
    LOG_INFO(logger, "final psi match rlt=" << psi);

    // ch.close();
    // ep.stop();
    // ios.stop();
    return rlt_vec;
}

std::vector<std::vector<u8>> ClientExchangeMetrics(uint64_t commonSeed, uint64_t commonSeed1,
                    const std::string& ip_, uint32_t port_,
                    uint64_t height, uint64_t width,
                    uint64_t logHeight, uint64_t receiver_size,
                    uint32_t bucket1,
                    const std::vector<block>& recv_set,
                    std::vector<std::array<block, 2>>& ot_messages) {
    const block common_seed = oc::toBlock(commonSeed, commonSeed1);
    PRNG common_prng(common_seed);
    block common_key;
    AES common_aes;
    common_prng.get((u8 *)&common_key, sizeof(block));
    common_aes.setKey(common_key);

    std::unique_ptr<IOService, std::function<void(IOService*)>> ios;
    std::unique_ptr<Endpoint, std::function<void(Endpoint*)>> ep;
    std::unique_ptr<Channel, std::function<void(Channel*)>> ch;
    if (!offline::USE_OFFLINE_CHANNEL) {
        ios = std::unique_ptr<IOService, std::function<void(IOService*)>>(new IOService(), [](IOService* p) {
            p->stop();
            delete p;
        });
        ep = std::unique_ptr<Endpoint, std::function<void(Endpoint*)>>(new Endpoint(*ios, ip_, port_, EpMode::Client, "Run"), [](Endpoint* p) {
            p->stop();
            delete p;
        });
        std::string compute_name = std::to_string(port_) + "compute";
        auto tmp_ch = ep->addChannel(compute_name, compute_name);
        ch = std::unique_ptr<Channel, std::function<void(Channel*)>>(new Channel(std::move(tmp_ch)), [](Channel* p) {
            p->close();
            delete p;
        });
    }
    // IOService ios;
    // Endpoint ep(ios, ip_, port_, EpMode::Client, "Run");
    // std::string compute_name = std::to_string(port_) + "compute";
    // Channel ch = ep.addChannel(compute_name, compute_name);
    auto height_in_bytes = (height + 7) / 8;
    auto location_in_bytes = (logHeight + 7) / 8;
    auto receiver_size_in_bytes = (receiver_size + 7) / 8;
    auto shift = (1 << logHeight) - 1;
    auto width_bucket1 = sizeof(block) / location_in_bytes;
    std::vector<std::vector<u8>> trans_locations(width_bucket1, std::vector<u8>(receiver_size * location_in_bytes + sizeof(u32)));
    std::vector<std::vector<u8>> matrixA(width_bucket1, std::vector<u8>(height_in_bytes));
    std::vector<std::vector<u8>> matrix_delta(width_bucket1, std::vector<u8>(height_in_bytes));
    std::vector<std::vector<u8>> trans_hash_inputs(width, std::vector<u8>(receiver_size_in_bytes, 0));
    for (uint32_t w_left = 0; w_left < width; w_left += width_bucket1)
    {
        auto wRight = w_left + width_bucket1 < width ? w_left + width_bucket1 : width;
        auto w = wRight - w_left;
        //////////// Compute random locations (transposed) ////////////////
        //recv_set holds the the key of PRF fucntion F for calculating value v of each element of reciever
        //for each element y,calculate the value v[i] and store them in trans_locations
        ComputeRandomLocations(common_prng, common_aes, common_key, receiver_size, bucket1,
                               recv_set, location_in_bytes, w, trans_locations);
        //////////// Compute matrix Delta /////////////////////////////////
        ////for each element y,uses the value v to set D[v[i]] of column[i] to zero according to (c) of step 1
        ComputMatrixDelta(width_bucket1, height_in_bytes, w, receiver_size, shift,
                          location_in_bytes, trans_locations, matrix_delta);
        //////////////// Compute matrix A & sent matrix ///////////////////////
        //   .Modified by wumingzi. 2022:08:01,Monday,22:43:58.
        /* client sends matrix D to server. here is different with paper but gets the same result.
        1）client generates ot_messages[2]:two random block vectors as seeds, server uses choices to get one block for each column and save to ot_message.
            client and server use the same prgn to generate matrixA.
            matrixA/B/Delta are all transposed.
        
        2）for client: generate matrix A using ot_messages[0] as seeds, generate matrix sent_matrix_vec using ot_messages[1] as seeds,
        sent_matrix_vec(sent_matrix_vec_original) saves one column of matrix( width columns in total, suppose current column is i).
        then for each row j to set sent_matrix_vec as following:
            sent_matrix_vec[j] = sent_matrix_vec_original ^(matrixA[i][j] ^ matrix_delta[i][j]);
            
        3）for server: generate matrix C using ot_messages recieved by OT from server.
        for the ith column of matrix C.
            i) if choices[i] = 0, then server get seed from client's ot_messages[0],then matrixC[i] = matrixA[i](server party);
            ii) if choices[i] = 1, then server get seed from client's ot_messages[1],then matrixC[i] = sent_matrix_vec_original[i]();
                    matrixC is calculated as following:
                    matrixC[i][j] ^= recv_matrix[j];
                    recv_matrix[j] = sent_matrix_vec(from server patry);
                    finally, matrixC[i][j] = sent_matrix_vec_original[i]^(sent_matrix_vec_original ^(matrixA[i][j] ^ matrix_delta[i][j]))
                                           =  (matrixA[i][j] ^ matrix_delta[i][j])    
                    the result is the same as (a) of step 2 in paper.
        */
        ComputeMatrixAAndSentMatrix(ot_messages, w, w_left, height_in_bytes, matrix_delta, ch, matrixA);
        ///////////////// Compute hash inputs (transposed) /////////////////////
        // for each element y, calculate the value of A1[v[1]],A2[v[2]]....,Aw[v[w]] and store them in trans_hash_inputs
        ComputeHashInputs(w, receiver_size, trans_locations, location_in_bytes, shift, w_left, matrixA, trans_hash_inputs);
    }
    // {
    //     u64 sentData = ch.getTotalDataSent();
    //     u64 recvData = ch.getTotalDataRecv();
    //     LOG_INFO("Compute step Receiver sent communication: " << sentData);
    //     LOG_INFO("Compute step Receiver received communication: " << recvData);
    // }
    // ch.close();
    // ep.stop();
    // ios.stop();
    return trans_hash_inputs;
}

std::vector<util::Buf128> IdsToHash(const std::vector<std::string>& ids) {
    std::vector<util::Buf128> rlt(ids.size());
    for (size_t i = 0; i < ids.size(); i++)
    {
        const std::string& str = ids[i];
        getMd5((unsigned char *)str.c_str(), str.length(), (unsigned char *)(rlt[i].buf));
    }
    return rlt;
}

std::vector<block> HashIdToBlocks(const uint8_t* hash_ids, int dataByteLen, int id_num) {
    std::vector<block> block_ids;
    block_ids.reserve(id_num);
    for (uint64_t i = 0; i < id_num; ++i)
    {
        uint64_t *dataBase1;
        uint64_t *dataBase2;
        // senderSet[i] = prng.get<block>();
        if (8 == dataByteLen)
        {
            dataBase1 = (uint64_t *)(hash_ids + i * dataByteLen);
            block_ids.emplace_back(oc::toBlock(*dataBase1));
        }
        else if (16 == dataByteLen)
        {
            dataBase1 = (uint64_t *)(hash_ids + i * dataByteLen);
            dataBase2 = (uint64_t *)(hash_ids + i * dataByteLen + 8);
            block_ids.emplace_back(oc::toBlock(*dataBase1, *dataBase2));
        }
        else
        {
            block_ids.emplace_back(oc::toBlock((i + 1) * 5));
        }
    }
    return block_ids;
}
std::vector<block> HashIdToBlocks(const std::vector<util::Buf128>& hash_ids) {
    std::vector<block> block_ids;
    block_ids.reserve(hash_ids.size());
    if (hash_ids.size() == 0) {
        return block_ids;
    } 
    // return HashIdToBlocks((uint8_t *)hash_ids.data(), sizeof(util::Buf128), hash_ids.size());
    for (auto& hash_id : hash_ids) {
        block_ids.emplace_back(oc::toBlock(hash_id.buf[0], hash_id.buf[1]));
    }
    return block_ids;
}

std::vector<block> PreDealPSIIds(const std::vector<std::string>& ids) {
    auto hash_ids = IdsToHash(ids);
    return HashIdToBlocks(hash_ids);
}

int64_t OprfPsiClient::OprfPsiAlg(std::vector<util::Buf128>&& hashBuf, uint64_t rmtNeles, OptAlg* optAlg) {
    uint64_t sender_size = rmtNeles;
    std::vector<block> receiver_set;

    if (ids_.size() == 0) {
        if (hashLen < 128)
        {
            LOG_ERROR(optAlg->logger, "Hash len is invalid, hash len:" << hashLen);
            return -1;
        }
    
        // int dataByteLen = hashLen / (sizeof(uint8_t) * 8);
        receiver_set = HashIdToBlocks(hashBuf);
    } else {
        LOG_INFO(optAlg->logger, "oprf client predeal psi ids");
        receiver_set = PreDealPSIIds(ids_);
        sender_size = GetSenderSize(receiver_set.size(), ip_, port_);
    }
    if (offline::USE_OFFLINE_CHANNEL) {
        LOG_INFO(optAlg->logger, "oprf client offline mode");
    } else {
        LOG_INFO(optAlg->logger, "oprf client online mode");
    }

    hashBuf.clear();
    hashBuf.shrink_to_fit();
    return __OprfPsiAlg(std::move(receiver_set), sender_size, optAlg);
}
int64_t OprfPsiClient::OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles, OptAlg* optAlg)
{
    uint64_t receiver_size = neles;
    uint64_t sender_size = rmtNeles;
    std::vector<block> receiver_set;
    if (ids_.size() == 0) {
        if (hashLen < 128)
        {
            LOG_ERROR(optAlg->logger, "Hash len is invalid, hash len:" << hashLen);
            return -1;
        }
    
        int dataByteLen = hashLen / (sizeof(uint8_t) * 8);
        receiver_set = HashIdToBlocks(hashBuf, dataByteLen, receiver_size);
    } else {
        LOG_INFO(optAlg->logger, "oprf client predeal psi ids");
        receiver_set = PreDealPSIIds(ids_);
        receiver_size = receiver_set.size();
        sender_size = GetSenderSize(receiver_size, ip_, port_);
    }
    if (offline::USE_OFFLINE_CHANNEL) {
        LOG_INFO(optAlg->logger, "oprf client offline mode");
    } else {
        LOG_INFO(optAlg->logger, "oprf client online mode");
    }
    
    return __OprfPsiAlg(std::move(receiver_set), sender_size, optAlg);
}

int64_t OprfPsiClient::__OprfPsiAlg(std::vector<block>&& receiver_set, uint64_t sender_size, OptAlg* optAlg) {
    auto receiver_size = receiver_set.size();
    LOG_INFO(optAlg->logger, "oprf client ComputeRecvSet");
    auto recv_set = ComputeRecvSet(receiver_size, hash1LengthInBytes, commonSeed, commonSeed1, receiver_set);
    // 释放不再使用变量的内存;
    receiver_set.clear();
    receiver_set.shrink_to_fit();
    malloc_trim(0);

    LOG_INFO(optAlg->logger, "oprf client ReceiverBaseOTs");
    auto ot_messages = ReceiverBaseOTs(width, inertalSeed, inertalSeed1, ip_, port_);
    LOG_INFO(optAlg->logger, "oprf client ClientExchangeMetrics");
    auto trans_hash_inputs = ClientExchangeMetrics(commonSeed, commonSeed1, ip_, port_,
                            height, width, logHeight, receiver_size, bucket1,
                            recv_set, ot_messages);
    // 释放不再使用变量的内存;
    recv_set.clear();
    recv_set.shrink_to_fit();
    ot_messages.clear();
    ot_messages.shrink_to_fit();

    LOG_INFO(optAlg->logger, "oprf client ReceiveHashOutputsAndComputePSI");
    std::vector<xsce_ose::block>* oprf_values = nullptr;
    if (save_ov_) {
        oprf_values = &oprf_values_;
    }
    auto pair_result = ReceiveHashOutputsAndComputePSI(width, receiver_size, trans_hash_inputs, bucket2, hashLengthInBytes, sender_size, ip_, port_, optAlg->logger, oprf_values);
    result_.clear();
    for_each(pair_result.begin(), pair_result.end(), [this](auto iter){
        result_.push_back(iter.first);
        result_.push_back(iter.second);
    });
    //ch.close();
    //ep.stop();
    //ios.stop();
    return 0;
}

} // namespace oprf_psi_offline
