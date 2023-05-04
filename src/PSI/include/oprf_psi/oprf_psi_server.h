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

namespace oprf_psi
{
    class OprfPsiServer : public OprfPsi
    {
    public:
        OprfPsiServer(const std::string &ip, uint32_t port) : OprfPsi(ip, port) {}
        int64_t OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles, OptAlg* optAlg);

    private:
        void Run(xsce_ose::PRNG &prng, xsce_ose::Channel &ch, xsce_ose::block commonSeed, const xsce_ose::u64 &sender_size,
                 const xsce_ose::u64 &receiverSize, const std::vector<xsce_ose::block> &sender_set, OptAlg* optAlg);

        void ComputeHashOutputs(xsce_ose::u64 width_in_bytes, xsce_ose::u64 sender_size, xsce_ose::Timer &timer, const std::vector<std::vector<xsce_ose::u8>> &trans_hash_inputs, xsce_ose::u64 hash_length_in_bytes, xsce_ose::Channel &ch);
    };
} // namespace oprf_psi
