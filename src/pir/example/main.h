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
 * @file main.h
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef XSCE_THIRD_PARTY_PIR_ALG_MAIN_H
#define XSCE_THIRD_PARTY_PIR_ALG_MAIN_H

#include "pir.h"
#include "common/pub/include/util.h"
#include "common/pub/include/ot.h"
#include "PSI/include/psi.h"

#include "toolkits/util/include/xutil.hpp"
#include "common/pub/include/globalCfg.h"

using namespace std;
using namespace xscePirAlg;
using namespace xsce_ose;

typedef struct _OseOpt
    {
        void *ose_opt_ptr = nullptr;
        int64_t role = 0; //default server mode,generate sk and send pk/rlk/galkey to client
        std::string addr = "127.0.0.1";
        int port = 5169;
        std::string name = "";
        std::vector<std::string> srv_data_vec;
        std::vector<std::string> cli_data_vec;
        std::vector<std::string> srv_id_vec;
        std::vector<std::string> cli_id_vec;
        int64_t batch_num = 0;
        int64_t simd_len = 0;
        double rlt_double = 0;
        bool test_rlt = true;
        bool thdOver = false;
    } OseOpt;

typedef std::function<int64_t(int64_t *)> Functional;
typedef std::function<int64_t(OptAlg*,OseOpt*)> UtFunction;
int64_t launchThread(UtFunction &func, std::vector<OptAlg> *opt_vec,std::vector<OseOpt> *ose_vec);
int64_t test_all(int role, int64_t srvSize, int64_t cliSize, int step, int alg, std::string ip, int port);

// int64_t pirUtest(std::vector<OptAlg> *opt_vec, std::vector<OseOpt> *ose_vec);
int64_t pirUtest(OptAlg *opt, OseOpt *pir_opt);
int64_t psipirUtest(OptAlg *opt, OseOpt *pir_opt);

//for ut use
#include <thread>
#include <functional>


#endif  //XSCE_THIRD_PARTY_PIR_ALG_MAIN_H
