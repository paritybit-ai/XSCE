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

#include "oprf_psi/oprf_psi.h"
#include "common/pub/include/Defines.h"

namespace oprf_psi
{
    class OprfPsiClient : public OprfPsi
    {
    public:
        OprfPsiClient(const std::string &ip, uint32_t port) : OprfPsi(ip, port) {}
        std::vector<uint64_t> GetPsiResult()
        {
            return result_;
        }
        int64_t OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles);

    private:
        void Run(util::PRNG &prng, util::Channel &ch, util::block common_seed, const util::u64 &sender_size,
                 const util::u64 &receiver_size, std::vector<util::block> &receiver_set);

        void ComputeHashOutputs(util::u64 hash_length_in_bytes, util::u64 width_in_bytes, util::u64 receiver_size, const std::vector<std::vector<util::u8>> &trans_hash_inputs, std::unordered_map<util::u64, std::vector<std::pair<util::block, util::u32>>> &all_hashes);
    private:
        std::vector<uint64_t> result_;
    };
} // namespace oprf_psi
