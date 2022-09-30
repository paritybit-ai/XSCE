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
 * @file base-compute.h
 * @author Created by chonglou. 2022:05:28
 * @brief 
 * @version 
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BASE_COMPUTE_H
#define BASE_COMPUTE_H

#include <string.h>

#include <vector>
#include <string>
#include <iostream>

#include "common/pub/include/globalCfg.h"

using namespace std;

#define MAX_THREAD_NUM 40

#define EXPORT_SYM  __attribute__((visibility("default")))

struct EndPoint_SPDZ
{
    EndPoint_SPDZ()
    {
        ip = "0.0.0.0";
        port = 9001;
    }
    std::string ip;
    uint16_t port;
};

typedef struct _SPDZAlg
{
    void SetParam(const xsce_ose::OptAlg* alg)
    {
        size_t row_num = alg->sampleNum;
        size_t index = 0; // always use first colomn

        input.clear();
        task_id = alg->taskId;
        ip_file_name = task_id + std::string("-ip-file");
        for (auto i = 0; i < row_num; i++)
        {
            input.push_back(alg->sampleDataBuf[i][index]);
        }

        role = alg->role;
        // support multi-parties
        if (xsce_ose::NETWORKMODE_DEFAULT == alg->networkmode)
        {
            for (unsigned int i = 0; i < alg->ipVec.size(); ++i) {
               EndPoint_SPDZ endpoint;
               endpoint.ip = alg->ipVec.at(i);
               endpoint.port = alg->portVec.at(i);
               endpoint_spdz_vec.push_back(endpoint);
            }
        }

        middle_prefix = "tmp";
        thread_num = alg->thdNum;

        output_file = alg->rltFn;
        SetNetworkMode(alg);
    }
    void SetNetworkMode(const xsce_ose::OptAlg* alg)
    {
        if (xsce_ose::NETWORKMODE_GATEWAY == alg->networkmode)
	    {
            gateway_cert_path = alg->gateway_cert_path;
            endpointlist_spdz = alg->endpointlist_spdz;
            endpointlist_spdz_gw = alg->endpointlist_spdz_gw;
        }
        else if (xsce_ose::NETWORKMODE_PORTSHARE == alg->networkmode)
        {
            endpointlist_spdz = alg->endpointlist_spdz;
        }
    }
    std::string task_id;           // xsce task id
    std::string alg_index_str;     // algorithms index, e.g., add2
    uint32_t role;                 // party(role) id

    std::vector<EndPoint_SPDZ> endpoint_spdz_vec;    // for multi-parties
    std::string ip_file_name;          // for spdz ip-file

    std::vector<double> input;        // spdz input vector
    std::vector<double> result;       // spdz output vector, but we store results in the output file by default
    std::string middle_prefix;        // spdz
    std::string output_file;          // output file
    std::string pub_input_file;       // public input file
    // If 1, spdz will use main thread to calculate. If >1, spdz will create thread_num sub-thread to calculate, which main thread is not included.
    size_t thread_num = 1;            // for multi-threads
    int input_mode = 0;               // data input mode,0:file;1:memory
    std::string gateway_cert_path;    // cert path of gateway
    std::string endpointlist_spdz;    // spdz endpoint list
    std::string endpointlist_spdz_gw; // for xsce gateway mode
} SPDZAlg;

// two-party computation
EXPORT_SYM int runAdd2(SPDZAlg *spdzalg);
EXPORT_SYM int runMul2(SPDZAlg *spdzalg);
EXPORT_SYM int runCmp2(SPDZAlg *spdzalg);
EXPORT_SYM int runVar2(SPDZAlg *spdzalg);
EXPORT_SYM int runMid2(SPDZAlg *spdzalg);

// three-party computation
EXPORT_SYM int runAdd3(SPDZAlg *spdzalg);
EXPORT_SYM int runMul3(SPDZAlg *spdzalg);
EXPORT_SYM int runVar3(SPDZAlg *spdzalg);
EXPORT_SYM int runMid3(SPDZAlg *spdzalg);

#endif //   BASE_COMPUTE_H
