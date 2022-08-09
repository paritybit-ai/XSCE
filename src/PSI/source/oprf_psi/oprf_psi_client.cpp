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

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>

#include "toolkits/util/include/xlog.h"

namespace oprf_psi
{
    using namespace xsce_ose;

    /*
     * output: ot_messages
     */
    void ReceiverBaseOTs(PRNG &prng, Channel &ch,
                         std::vector<std::array<block, 2>> &ot_messages)
    {
        IknpOtExtSender ot_ext_sender;
        ot_ext_sender.genBaseOts(prng, ch);

        ot_ext_sender.send(ot_messages, prng, ch);
        

        LOG_INFO("Receiver base OT finished");
    }

    /*
     * output recv_set
     */
    //   .Modified by wumingzi. 2022:08:01,Monday,22:20:16.
    /*
    ComputeRecvSet calculates the PRF function F: input is psi elements,output is  a w-dimension vector v.
    for each output of F v(v[1],v[2],...v[w]) ,v[i]'s value is from 0 to h-1
    */
    void ComputeRecvSet(u64 receiver_size, u64 h1_length_in_bytes, const AES &common_aes,
                        const std::vector<block> &receiver_set,
                        std::vector<block> &recv_set)
    {
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
                                     u64 height_in_bytes, std::vector<std::vector<u8>> &matrix_delta, Channel &ch,
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

            ch.asyncSend(std::move(sent_matrix_vec));
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
    void OprfPsiClient::ComputeHashOutputs(u64 hash_length_in_bytes, u64 width_in_bytes, u64 receiver_size, const std::vector<std::vector<u8>> &trans_hash_inputs, std::unordered_map<u64, std::vector<std::pair<block, u32>>> &all_hashes)
    {
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
                SaveOprfValues((char*)hash_inputs[j - low].data(), width_in_bytes);

                H.Final(hash_output);

                all_hashes[*(u64 *)hash_output].push_back(std::make_pair(*(block *)hash_output, j));
            }
        }
    }

    /*
     * output: rlt_vec
     */
    void ReceiveHashOutputsAndComputePSI(u64 bucket2, u64 hash_length_in_bytes, u64 sender_size, Channel &ch,
                                         const std::unordered_map<u64, std::vector<std::pair<block, u32>>> &all_hashes,
                                         std::vector<uint64_t> &rlt_vec)
    {
        std::vector<u8> recv_buff_vec(bucket2 * hash_length_in_bytes);
        u8 *recv_buff = recv_buff_vec.data();
        auto psi = 0;
        for (u64 low = 0; low < sender_size; low += bucket2)
        {
            auto up = low + bucket2 < sender_size ? low + bucket2 : sender_size;

            ch.recv(recv_buff, (up - low) * hash_length_in_bytes);

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
                        rlt_vec.push_back(idx + low);
                        rlt_vec.push_back(found->second[i].second);
                        //client hash output to generate 128bit aes key.
                        break;
                    }
                }
            }
        }
        LOG_INFO("final psi match rlt=" << psi);
    }

    //oprf psi alg note according to figure 3 in oprf paper << Private Set Intersection in the Internet Setting From Lightweight Oblivious PRF>>   .Modified by wumingzi. 2022:08:01,Monday,21:47:07.
    /*
    the entire protocol is shown in docs/img/oprf_psi_alg.png
    1）D/A/B are all matrices of the same row/column: w means width and h means height.
    2）P1 is server/sender, P2 is client/reciever. P1 holds elements x,P2 holds elements y,
    3）two hash functions: H1 and H2: H1 converts inputs with any length to output with l1 bits,
        H2 converts inputs with w bits to output  with l2 bits,
    4) PRF function F: converts the output of H1 with l1 bits to  a w-dimension vector v(v[1],v[2],...v[w]) ,v[i]'s value is from 0 to h-1
    */
    //   .Modification over by wumingzi. 2022:08:01,Monday,21:47:11.
    void OprfPsiClient::Run(PRNG &prng, Channel &ch, block common_seed, const u64 &sender_size,
                            const u64 &receiver_size,
                            std::vector<block> &receiver_set)
    {
        Timer timer;

        timer.setTimePoint("Receiver start 20210805 002");

        TimeUnit start, end;

        //logHeight means the number of bits to hold the value of h
        //location_in_bytes means the location of row in matrix(A/D/B)
        //width_bucket1 means the number of rows in a block(128 bits)

        auto height_in_bytes = (height + 7) / 8;
        auto width_in_bytes = (width + 7) / 8;
        auto location_in_bytes = (logHeight + 7) / 8;
        auto receiver_size_in_bytes = (receiver_size + 7) / 8;
        auto shift = (1 << logHeight) - 1;
        auto width_bucket1 = sizeof(block) / location_in_bytes;

        LOG_INFO("in rcv run: height_in_bytes=" << height_in_bytes << ",width_in_bytes=" << width_in_bytes << ",location_in_bytes=" << location_in_bytes);
        LOG_INFO("receiver_size_in_bytes=" << receiver_size_in_bytes << ",width_bucket1=" << width_bucket1);

        //  .Modified by wumingzi. 2022:08:01,Monday,22:14:14.
        /*
         server uses ot to get seed to generate matrix C,this is different with paper which transfer an entire column in each OT
         here only transfer a block in each OT and then use the block as seed to generate a column with h bits.
         w=width is the column size of matrxA/B/D. so there are width OT transfers in total. server is chooser,client is sender.
        */
        ///////////////////// Base OTs ///////////////////////////
        std::vector<std::array<block, 2>> ot_messages(width);
        ReceiverBaseOTs(prng, ch, ot_messages);
        timer.setTimePoint("Receiver base OT finished");

        //////////// Initialization ///////////////////

        PRNG common_prng(common_seed);
        block common_key;
        AES common_aes;

        LOG_INFO("Receiver initialized");
        timer.setTimePoint("Receiver initialized");

        /////////// Transform input /////////////////////

        common_prng.get((u8 *)&common_key, sizeof(block));
        common_aes.setKey(common_key);

        // calculate the key of PRF function F for each element  .Modified by wumingzi. 2022:08:01,Monday,22:22:16.
        ///////////////// Comput recv_set ///////////////////
        std::vector<block> recv_set(receiver_size);
        ComputeRecvSet(receiver_size, hash1LengthInBytes, common_aes, receiver_set, recv_set);

        LOG_INFO("Receiver set transformed");
        timer.setTimePoint("Receiver set transformed");

        // matrixA/B/D are all tranposed to store in memory  .Modified by wumingzi. 2022:08:01,Monday,22:24:09.
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

        LOG_INFO("Receiver matrix sent and transposed hash input computed");
        timer.setTimePoint("Receiver matrix sent and transposed hash input computed");

        /////////////////// Compute hash outputs ///////////////////////////
        //for each element y,  use H2 to calculate oprf value ,H2(A1[v[1]],A2[v[2]]....,Aw[v[w]])
        //recieve oprf value of each element x in server and compare them with oprf value of y to find intersection elements
        std::unordered_map<u64, std::vector<std::pair<block, u32>>> all_hashes;
        ComputeHashOutputs(hashLengthInBytes, width_in_bytes, receiver_size,
                           trans_hash_inputs, all_hashes);

        timer.setTimePoint("Receiver hash outputs computed");

        ///////////////// Receive hash outputs from sender and compute PSI ///////////////////
        LOG_INFO("begin to compare oprf value.");
        ReceiveHashOutputsAndComputePSI(bucket2, hashLengthInBytes, sender_size, ch, all_hashes, result_);

        timer.setTimePoint("Receiver intersection computed");

        LOG_INFO("\n" << timer);

        //////////////// Output communication /////////////////

        u64 sentData = ch.getTotalDataSent();
        u64 recvData = ch.getTotalDataRecv();
        u64 totalData = sentData + recvData;

        LOG_INFO("Final step Receiver sent communication: " << sentData / std::pow(2.0, 20) << " MB");
        LOG_INFO("Final step Receiver received communication: " << recvData / std::pow(2.0, 20) << " MB");
        LOG_INFO("Final step Receiver total communication: " << totalData / std::pow(2.0, 20) << " MB");
    }

    int64_t OprfPsiClient::OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles)
    {
        uint64_t senderSize = rmtNeles;
        uint64_t receiverSize = neles;

        std::string chName = "oprfPsi_000";
        if (hashLen < 128)
        {
            LOG_ERROR("Hash len is invalid, hash len:" << hashLen);
            return -1;
        }
        int dataByteLen = hashLen / (sizeof(uint8_t) * 8);

        uint64_t *dataBase1;
        uint64_t *dataBase2;
        uint8_t *dataBuf = hashBuf;
        LOG_INFO("Client ip:" << ip_ << ", port:" << port_);
        IOService ios;
        Endpoint ep(ios, ip_, port_, EpMode::Client, chName);
        Channel ch = ep.addChannel();
        LOG_INFO("chName=" << chName);
        LOG_INFO("dataByteLen=" << dataByteLen);
        LOG_INFO("common seed=" << std::hex << commonSeed << ":" << commonSeed1);
        LOG_INFO("client internal seed=" << std::hex << inertalSeed << ":" << inertalSeed1);

        LOG_INFO("senderSize=" << std::dec << senderSize << ",receiverSize=" << receiverSize);

        std::vector<block> receiverSet(receiverSize);
        //prng should use 128 bit seed.
        PRNG prng(oc::toBlock(inertalSeed,inertalSeed1));

        for (uint64_t i = 0; i < receiverSize; ++i)
        {
            if (8 == dataByteLen)
            {
                dataBase1 = (uint64_t *)(dataBuf + i * dataByteLen);
                receiverSet[i] = oc::toBlock(*dataBase1);
            }
            else if (16 == dataByteLen)
            {
                dataBase1 = (uint64_t *)(dataBuf + i * dataByteLen);
                dataBase2 = (uint64_t *)(dataBuf + i * dataByteLen + 8);
                receiverSet[i] = oc::toBlock(*dataBase1, *dataBase2);
            }
            else
            {
                receiverSet[i] = oc::toBlock((i + 1) * 5);
            }
        }

        const block block_common_seed = oc::toBlock(commonSeed, commonSeed1);
        Run(prng, ch, block_common_seed, senderSize, receiverSize, receiverSet);
        ch.close();
        ep.stop();
        ios.stop();
        return 0;
    }

} // namespace oprf_psi
