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
 * @file oprf_psi_client.h
 * @author Created by haiyu.  2022:07:21,Thursday,00:10:48.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include <string>
#include <vector>

#include "oprf_psi.h"
#include "common/pub/include/Defines.h"

namespace oprf_psi_offline
{
std::vector<block> PreDealPSIIds(const std::vector<std::string>& ids);
int64_t GetSenderSize(uint64_t receiver_size, const std::string& ip, uint32_t port);
std::vector<std::array<block, 2>> ReceiverBaseOTs(
    int width, uint64_t inertalSeed, uint64_t inertalSeed1,
    std::string ip, uint32_t port);
std::vector<block> ComputeRecvSet(u64 receiver_size, u64 h1_length_in_bytes, 
                        uint64_t commonSeed, uint64_t commonSeed1,
                        const std::vector<block> &receiver_set);
std::vector<std::vector<u8>> ClientExchangeMetrics(uint64_t commonSeed, uint64_t commonSeed1,
                    const std::string& ip_, uint32_t port_,
                    uint64_t height, uint64_t width,
                    uint64_t logHeight, uint64_t receiver_size,
                    uint32_t bucket1,
                    const std::vector<block>& recv_set,
                    std::vector<std::array<block, 2>>& ot_messages);
std::vector<std::pair<uint64_t, uint64_t>> ReceiveHashOutputsAndComputePSI(uint64_t width, uint64_t receiver_size,
                        const std::vector<std::vector<u8>> &trans_hash_inputs,
                        u64 bucket2, u64 hash_length_in_bytes, u64 sender_size,
                        const std::string& ip, uint32_t port);


    class OprfPsiClient : public OprfPsi
    {
    public:
        OprfPsiClient(const std::string &ip, uint32_t port) : OprfPsi(ip, port) {}
        std::vector<uint64_t> GetPsiResult()
        {
            return result_;
        }
        int64_t OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles, OptAlg* optAlg);

    private:
        void Run(xsce_ose::PRNG &prng, xsce_ose::block common_seed, const xsce_ose::u64 &sender_size,
                 const xsce_ose::u64 &receiver_size, std::vector<xsce_ose::block> &receiver_set, OptAlg* optAlg);

        // void ComputeHashOutputs(xsce_ose::u64 hash_length_in_bytes, xsce_ose::u64 width_in_bytes, xsce_ose::u64 receiver_size, const std::vector<std::vector<xsce_ose::u8>> &trans_hash_inputs, std::unordered_map<xsce_ose::u64, std::vector<std::pair<xsce_ose::block, xsce_ose::u32>>> &all_hashes);
    private:
        std::vector<uint64_t> result_;
    };
} // namespace oprf_psi_offline
