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
 * @file oprf_psi_server.h
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
#include "common/pub/include/util.h"

namespace oprf_psi
{
struct SenderBaseOtResult {
    std::vector<block> ot_messages;
    BitVector choices;
};
uint64_t GetRecevierSize(uint64_t sender_size, const std::string& ip, uint32_t port);

SenderBaseOtResult SenderBaseOTs(
    uint64_t width, uint64_t inertalSeed, uint64_t inertalSeed1,
    std::string ip, uint32_t port);
std::vector<block> ComputeSendSet(u64 h1_length_in_bytes, u64 sender_size, 
                    uint64_t commonSeed, uint64_t commonSeed1,
                    const std::vector<block> &sender_set);
std::vector<std::vector<u8>> ServerExchangeMetrics(uint64_t commonSeed, uint64_t commonSeed1,
                    const std::string& ip, uint32_t port,
                    uint64_t height, uint64_t width,
                    uint64_t logHeight, uint64_t sender_size,
                    uint32_t bucket1, uint32_t hash1LengthInBytes,
                    const std::vector<block>& send_set,
                    const std::vector<block> &ot_messages,
                    const BitVector &choices);
void ServerComputeHashOutputs(u64 width, u64 sender_size, uint64_t bucket2, 
        const std::vector<std::vector<u8> > &trans_hash_inputs, u64 hash_length_in_bytes,
        const std::string& ip, uint32_t port, std::vector<xsce_ose::block>* oprf_values=nullptr);


class OprfPsiServer : public OprfPsi
{
public:
    OprfPsiServer(const std::string &ip, uint32_t port) : OprfPsi(ip, port) {}
    int64_t OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles, OptAlg* optAlg);
    int64_t OprfPsiAlg(std::vector<util::Buf128>&& hashBuf, uint64_t rmtNeles, OptAlg* optAlg) override;
private:
    int64_t __OprfPsiAlg(std::vector<xsce_ose::block>&& sender_set, uint64_t rmtNeles, OptAlg* optAlg);
    void Run(xsce_ose::PRNG &prng, xsce_ose::block commonSeed, const xsce_ose::u64 &sender_size,
             const xsce_ose::u64 &receiverSize, const std::vector<xsce_ose::block> &sender_set, OptAlg* optAlg);
    // void ComputeHashOutputs(xsce_ose::u64 width_in_bytes, xsce_ose::u64 sender_size, xsce_ose::Timer &timer, const std::vector<std::vector<xsce_ose::u8>> &trans_hash_inputs, xsce_ose::u64 hash_length_in_bytes, xsce_ose::Channel &ch);
};
} // namespace oprf_psi_offline
