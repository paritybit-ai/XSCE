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

namespace oprf_psi_offline
{
namespace offline {
extern bool USE_OFFLINE_CHANNEL;
extern std::vector<uint8_t> recv_data;
extern uint64_t cur_read_recv_data;
extern std::vector<uint8_t> send_data;
extern uint64_t cur_write_send_data;

inline void SetOfflineChannelFlag(bool offline) {
    USE_OFFLINE_CHANNEL = offline;
}
inline std::vector<uint8_t> GetSendData() {
    auto tmp = std::move(send_data); // 返回并清空数据
    send_data.clear();
    cur_write_send_data = 0;
    return tmp;
}
inline void SetRecvData(std::vector<uint8_t>&& data) {
    recv_data.clear();
    recv_data = std::move(data);
    cur_read_recv_data = 0;
}
inline void SetRecvData(const std::vector<uint8_t>& data) {
    auto tmp = data;
    SetRecvData(std::move(tmp));
}
} // namespace offline

    using namespace xsce_ose;

    using OptAlg = xsce_ose::OptAlg;
    class OprfPsi
    {
    public:
        OprfPsi(const std::string &ip, uint32_t port) : ip_(ip), port_(port)
        {
            height = 1 << (logHeight);
        }
        virtual ~OprfPsi() {}
        virtual int64_t OprfPsiAlg(uint8_t *hashBuf, uint64_t neles, uint64_t rmtNeles, OptAlg* optAlg) = 0;

        void SetOprfValuesFlag(bool flag) {save_ov_ = flag;}
        std::vector<xsce_ose::block> GetOprfValues() {return oprf_values_;}
        void SaveOprfValues(char* hash_inputs, size_t size) {
            if (save_ov_) {
                uint64_t hash_out[2];
                util::getMd5((unsigned char*)hash_inputs, size, (unsigned char*)hash_out);
                xsce_ose::block tmp(hash_out[0], hash_out[1]);
                // LOG_DEBUG("oprf_value:" << tmp);
                oprf_values_.emplace_back(std::move(tmp));
            }
        }
        void SetHashLen(uint64_t hash_len) {
            if (hash_len > 0) {
                hashLen = hash_len;
            }
        }
        void SetWidth(uint32_t w) {
            if (w > 0) {
                width = w;
            }
        }
        void SetLogHeight(uint32_t log_height) {
            if (log_height > 0) {
                logHeight = log_height;
            }
        }
        void SetHeight(uint32_t h) {
            if (h > 0) {
                height = h;
            }
        }
        void SetHashLengthInBytes(uint32_t hash_length_in_bytes) {
            if (hash_length_in_bytes > 0) {
                hashLengthInBytes = hash_length_in_bytes;
            }
        }
        void SetHash1LengthInBytes(uint64_t hash1_length_in_bytes) {
            if (hash1_length_in_bytes > 0) {
                hash1LengthInBytes = hash1_length_in_bytes;
            }
        }
        void SetBucket(uint32_t bucket) {
            if (bucket > 0) {
                bucket1 = 1 << bucket;
                bucket2 = 1 << bucket;
                LOG_INFO("oprf psi alg client mode. set bucket1 & bucket2 to " << bucket1);
            }
        }
        void SetCommonSeed(uint64_t seed, uint64_t seed1) {
            if (seed > 0 && seed1 > 0) {
                commonSeed = seed;
                commonSeed1 = seed1;
            }
        }
        void SetInertalSeed(uint64_t seed, uint64_t seed1) {
            if (seed > 0 && seed1 > 0) {
                inertalSeed = seed;
                inertalSeed1 = seed1;
            }
        }

        void SetIds(const std::vector<std::string> &ids) {
            auto tmp = ids;
            SetIds(std::move(tmp));
        }
        void SetIds(std::vector<std::string> &&ids) {
            ids_ = std::move(ids);
        }
        void PrintArgs() {
            std::cout << "hashLen: " << hashLen << std::endl;
            std::cout << "width: " << width << std::endl;
            std::cout << "logHeight: " << logHeight << std::endl;
            std::cout << "height: " << height << std::endl;
            std::cout << "hashLengthInBytes: " << hashLengthInBytes << std::endl;
            std::cout << "hash1LengthInBytes: " << hash1LengthInBytes << std::endl;
            std::cout << "bucket1: " << bucket1 << std::endl;
            std::cout << "bucket2: " << bucket2 << std::endl;
            std::cout << "commonSeed: " << commonSeed << std::endl;
            std::cout << "commonSeed1: " << commonSeed1 << std::endl;
            std::cout << "inertalSeed: " << inertalSeed << std::endl;
            std::cout << "inertalSeed1: " << inertalSeed1 << std::endl;
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
        std::vector<std::string> ids_;

        std::string ip_;
        uint32_t port_;

        bool save_ov_{false};
        std::vector<xsce_ose::block> oprf_values_;
    };
} // namespace oprf_psi_offline
