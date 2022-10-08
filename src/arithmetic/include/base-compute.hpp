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
 * @file base-compute.hpp
 * @author Created by chonglou. 2022:05:28
 * @brief 
 * @version 
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BASE_COMPUTE_HPP
#define BASE_COMPUTE_HPP

#include "base-compute.h"
#include "machinerunner.h"
#include "toolkits/util/include/xlog.h"

/**
 * @brief Get the End Point From Str object
 * 
 * @param strin 
 * @param strout 
 * @param strsep 
 * @return true 
 * @return false 
 */
bool GetEndPointFromStr(string& strin, string& strout, string strsep=";")
{
    if (0 == strin.size())
    {
        return false;
    }
    size_t pos = strin.find(strsep);
    if (pos == string::npos)
    {
        strout = strin;
        strin = "";
    }
    else
    {
        strout = strin.substr(0, pos);
        strin = strin.substr(pos+strsep.size());
    }

    return true;
}

/**
 * @brief split endpoint list to vector
 * 
 * @param endpointlist_str 
 * @param endpoint_vec 
 * @param strsep 
 * @param pDefault 
 * @return int 
 */
int SplitEndPointStr2Vec(const string& endpointlist_str, vector<EndPoint_SPDZ>& endpoint_vec, string strsep=";", const EndPoint_SPDZ* pDefault=NULL)
{
    string endpointlist_str_tmp = endpointlist_str;
    string endpoint_str;
    while (GetEndPointFromStr(endpointlist_str_tmp, endpoint_str, strsep))
    {
        LOG_INFO("get endpoint: (" << endpoint_str << ") from: " << endpointlist_str);
        if (endpoint_str.length() > 0 && endpoint_str.at(0) != '#') 
        {
            EndPoint_SPDZ endpoint;
            auto pos = endpoint_str.find(':');
            string port;
            if (pos == string::npos)
            {
                //  if only one value, assign to ip or port from the format
                auto tmp = endpoint_str.find(".");
                if (tmp != string::npos)
                {
                    endpoint.ip = endpoint_str;
                }
                else
                {
                    port = endpoint_str;
                }
            }
            else
            {
                endpoint.ip = endpoint_str.substr(0, pos);
                port = endpoint_str.substr(pos + 1);
            }
            if (0 == endpoint.ip.size())
            {
                if (NULL != pDefault)
                {
                    endpoint.ip = pDefault->ip;
                }
                else
                {
                    endpoint.ip = "0.0.0.0";
                }
            }
            if (0 == port.size())
            {
                if (NULL != pDefault)
                {
                    port = std::to_string(pDefault->port);
                }
                else
                {
                    port = 9000;    //  default 9000
                }
            }
            stringstream(port) >> endpoint.port;
            endpoint_vec.push_back(endpoint);
        }
    }

    return 0;
}

/**
 * @brief the mp-spdz virtual machine entry function
 * 
 * @tparam T 
 * @param spdzalg 
 * @param parties 
 * @param machine_type 
 * @param input 
 * @param size 
 * @return int 
 */
template<typename T>
int runSPDZMachine(SPDZAlg *spdzalg, int parties, int machine_type=EMACHINETYPE_NONE, T* input=NULL, int size=0)
{
    if (spdzalg->role >= parties)
    {
        LOG_ERROR("role is larger than parties: " << spdzalg->role << "/" << parties);
        return -1;
    }
    int protocol = 1;       // TODO: to be configurable
    //  Init Parameters
    int argc = 0;
    const char* argv[300] = {NULL};
    argv[argc++] = "spdz";
    std::cout << "entering spdz" << std::endl;
    // player name
    argv[argc++] = "-p";
    std::string role_str = std::to_string(spdzalg->role);
    argv[argc++] = role_str.c_str();
    argv[argc++] = "-N";
    std::string n_parties_str = std::to_string(parties);
    argv[argc++] = n_parties_str.c_str();
    if (3 == parties) {
        protocol = 1; // SHAMIR protocol
    } else {
        protocol = 0;  // SEMI protocol
    }

    if (0 == spdzalg->endpoint_spdz_vec.size())
    {
        SplitEndPointStr2Vec(spdzalg->endpointlist_spdz, spdzalg->endpoint_spdz_vec);
    }
    // port information
    if (0 == spdzalg->endpoint_spdz_vec.size())
    {
        LOG_ERROR("INCORRECT number of parties: " << spdzalg->endpoint_spdz_vec.size());
        return -1;
    }
    argv[argc++] = "-pn";
    string strport = std::to_string(spdzalg->endpoint_spdz_vec[0].port);
    argv[argc++] = strport.c_str();

    argv[argc++] = "-IF";
    std::string middlefilename = spdzalg->middle_prefix;
    argv[argc++] = middlefilename.c_str();

    if (spdzalg->output_file.size() > 0)
    {
        argv[argc++] = "-OF";
        argv[argc++] = spdzalg->output_file.c_str();
    }

    // circuit name
    argv[argc++] = spdzalg->alg_index_str.c_str();

    stringstream log_ss;
    log_ss << "command: ";
    for (int i=0; i<argc; i++)
    {
        log_ss << string(argv[i]) << " ";
    }
    LOG_INFO(log_ss.str());

    if (EMACHINETYPE_NONE == machine_type)
    {
        switch (protocol)
        {
        case 0: //  semi machine
            machine_type = EMACHINETYPE_SEMI;
            break;
        case 1: //  shamir machine
            machine_type = EMACHINETYPE_SHAMIRE;
            break;
        case 2: //  mascot machine
            machine_type = EMACHINETYPE_MASCOT;
            break;
        default:
            break;
        }
    }

    MachineRunner::Run(argc, argv, (MachineType)machine_type);

    return 0;
}

/**
 * @brief for input
 * 
 * @param spdzalg 
 * @param parties 
 * @param machine_type 
 * @param store_rlt_vec 
 * @return int 
 */
int runSPDZComputation_mem(SPDZAlg *spdzalg, int parties, int machine_type=EMACHINETYPE_NONE, bool store_rlt_vec= false);

/**
 * @brief 
 * 
 * @param spdzalg 
 * @param data_size 
 * @param sum 
 * @param square_sum 
 * @param parties 
 * @return int 
 */
int runSPDZComputation_mem(SPDZAlg *spdzalg, size_t data_size, double sum, double square_sum, int parties);

#endif  //  BASE_COMPUTE_HPP

