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
 * @file oprf_psi.h
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

#include "common/pub/include/util.h"
#include "common/pub/include/Defines.h"
#include "common/pub/include/globalCfg.h"
#include "toolkits/util/include/xlog.h"

namespace oprf_psi
{
    using OptAlg = xsce_ose::OptAlg;
    class OprfPsi
    {
    public:
        OprfPsi(const std::string &ip, uint32_t port) : ip_(ip), port_(port)
        {
            height = 1 << (logHeight);
        }
        virtual ~OprfPsi() {}
        virtual int64_t OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles) = 0;
        bool ParseOpt(const OptAlg &opt)
        {
            if (opt.oprfWidth > 0)
            {
                width = opt.oprfWidth;
                LOG_INFO("oprf psi alg. set width to " << width);
            }

            if (opt.oprfLogHeight > 0)
            {
                logHeight = opt.oprfLogHeight;
                height = 1 << (opt.oprfLogHeight);
                LOG_INFO("oprf psi alg client mode. set logHeight to " << logHeight);
            }

            if (opt.oprfHashLenInBytes > 0)
            {
                hashLengthInBytes = opt.oprfHashLenInBytes;
                LOG_INFO("oprf psi alg client mode. set hashLengthInBytes to " << hashLengthInBytes);
            }

            if (opt.oprfBucket1 > 0)
            {
                bucket1 = 1 << (opt.oprfBucket1);
                bucket2 = 1 << (opt.oprfBucket1);
                LOG_INFO("oprf psi alg client mode. set bucket1 & bucket2 to " << bucket1);
            }
            if (opt.hashLen > 0)
            {
                hashLen = opt.hashLen;
            }
            if (opt.commonSeed > 0 && commonSeed1 > 0)
            {
                commonSeed = opt.commonSeed;
                commonSeed1 = opt.commonSeed1;
            }
            if (opt.inertalSeed > 0 && opt.inertalSeed1 > 0)
            {
                inertalSeed = opt.inertalSeed;
                inertalSeed1 = opt.inertalSeed1;
            }
            
            return true;
        }

        void SetOprfValuesFlag(bool flag) {save_ov_ = flag;}
        std::vector<util::block> GetOprfValues() {return oprf_values_;}
        void SaveOprfValues(char* hash_inputs, size_t size) {
            if (save_ov_) {
                uint64_t hash_out[2];
                util::getMd5((unsigned char*)hash_inputs, size, (unsigned char*)hash_out);
                util::block tmp(hash_out[0], hash_out[1]);
                LOG_DEBUG("oprf_value:" << tmp);
                oprf_values_.emplace_back(std::move(tmp));
            }
        }

    protected:
        uint64_t hashLen{128};
        uint32_t width{632};
        uint32_t logHeight{20};
        uint32_t height;
        uint32_t hashLengthInBytes{10};
        uint64_t hash1LengthInBytes{32};
        uint32_t bucket1{256};
        uint32_t bucket2{256};
        uint64_t commonSeed{0xAA55AA55AA55AA55};
        uint64_t commonSeed1{0xAA55AA55AA55AA55};
        uint64_t inertalSeed{0x1122334455667788};
        uint64_t inertalSeed1{0x1122334455667788};

        std::string ip_;
        uint32_t port_;

        bool save_ov_{false};
        std::vector<util::block> oprf_values_;
    };
} // namespace oprf_psi
