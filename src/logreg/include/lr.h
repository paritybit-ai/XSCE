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
 * @file lr.h
 * @author Created by wumingzi. 2023:02:15,Wednesday,14:41:29.
 * @brief 
 * @version 
 * @date 2023-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef XSCE_THIRD_PARTY_LR_ALG_LR_H
#define XSCE_THIRD_PARTY_LR_ALG_LR_H

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iomanip>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <ctime>

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Common/TestCollection.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/Matrix.h>
#include <cryptoTools/Crypto/PRNG.h>

// for ot api  .Modified by wumingzi/wumingzi. 2021:07:12,Monday,14:54:28.
#include <libOTe/Tools/Tools.h>
#include <libOTe/Tools/LinearCode.h>
#include <libOTe/NChooseOne/Oos/OosNcoOtReceiver.h>
#include <libOTe/NChooseOne/Oos/OosNcoOtSender.h>
#include <libOTe/NChooseOne/NcoOtExt.h>
#include <libOTe/TwoChooseOne/OTExtInterface.h>

#include "toolkits/util/include/xutil.hpp"
#include "common/pub/include/globalCfg.h"
#include "common/pub/include/ot.h"
#include "common/pub/include/util.h"

namespace xsceLogRegAlg
{
    using OptAlg = xsce_ose::OptAlg;
    using namespace osuCrypto;

#define SERVER 0
#define CLIENT 1

    struct RegressionParam
    {
        int mIterations;
        int mBatchSize;
        float mLearningRate;
    };

    typedef struct _Aby2Opt
    {
        int role;
        std::string address;
        uint16_t port;
        // seclvl seclvl;
        uint32_t secparam;
        uint32_t nthreadpool;
        uint32_t len;
        uint32_t nthreads;
        int mt_alg;
        int sharing;
        std::vector<float> *data = nullptr;
        std::string circuit_dir;
        std::vector<float> *rltVec = nullptr;

        //for lr alg,in each iteration,nvals equals to the bacth sample number.
        int nvals = 1;

        //thread related opt
        int threadNum = 1;
        int thdIdx;
        int cpuNum = 0;
        int64_t offset;
        int64_t curLen;
        int64_t totalLen;
        bool thdOver = false;

        // for spdz alg  .Modified by panguangming/wumingzi. 2022:04:20,Wednesday,22:32:17.
        std::string ip0 = "127.0.0.1";
        std::string ip1 = "127.0.0.1";
        int port0;
        int port1;

        std::string taskId;
        int networkmode = 0;
        std::string gateway_cert_path;
        std::string endpointlist_spdz;
        std::string endpointlist_spdz_gw;
        //   .Modification over by panguangming/wumingzi. 2022:04:20,Wednesday,22:32:26.

    } Aby2Opt;

    // used for lr train
    typedef struct _LRTrainAlgOpt
    {
        int role = 0; // 0 means server,1 means client.
        std::string ip = "127.0.0.1";
        int port = 7766;
        std::string cirDir = "../";

        std::string rmtIp = "127.0.0.1";
        int rmtPort = 7756;

        // taskInfo & simdlen  .Modified by panguangming/wumingzi. 2022:03:15,Tuesday,16:31:14.
        // call saveTaskMapProcess(std::string taskId, int data, GlobalCfg *taskGlobal);
        std::string taskId = "";
        xsce_ose::GlobalCfg *cfg = nullptr;
        uint32_t simdLen = 1024;
        int threadNum = 1;
        int cpuNum = 0;

        //   .Modification over by panguangming/wumingzi. 2022:03:15,Tuesday,16:31:31.

        // support spdz innder product  .Modified by panguangming/wumingzi. 2022:04:20,Wednesday,21:50:49.
        int type = 0;
        //   .Modification over by panguangming/wumingzi. 2022:04:20,Wednesday,21:51:03.

        // fix bug in case that server has diferent feature number with client  .Modified by panguangming/wumingzi. 2021:11:10,Wednesday,20:26:47.
        int64_t rmtFeatureNum = 0;
        //   .Modification over by panguangming/wumingzi. 2021:11:10,Wednesday,20:27:15.

        int port_num = 1;         // each outer thread needs a seperate port
        int inner_parallels = 32; // num of inner threads
        int nvals = 1000;         // num of simd operation elements
        int secparam = 128;       // symmetric security bits, default: 128
        RegressionParam params;

        std::vector<std::vector<float> > train_data_part; // one train data part
        std::vector<float> labels;                        // labels only for server, such as 0/1
        std::vector<float> param_w;
        std::vector<std::vector<float> > test_data_part;
        std::vector<float> test_labels;
        float auc;
        float ks;
        // add by light, for recording the predicting time info
        std::string predictTime;
        // add by light, for recording the predicting time info
        int networkmode = 0;
        std::string gateway_cert_path;
        std::string endpointlist_spdz;
        std::string endpointlist_spdz_gw;
    } LRTrainAlgOpt;

    int64_t logReg2PartyAlg(OptAlg *opt);
    void setLogReg2pAlgOpt(OptAlg *algOpt, LRTrainAlgOpt *opt);
    std::string showLogReg2pAlgOpt(LRTrainAlgOpt *opt);
    int lrTrainQuick(LRTrainAlgOpt &opt);
    int dotMulSimdMultiThd(Aby2Opt opt);
    void dotMulFloatVec(std::vector<float> *vec1, std::vector<float> *vec2, std::vector<float> *rltVec, int batch);
    int spdzDotMulSimdMultiThd(Aby2Opt &opt, int round_seq = 0 );
    
    float cal_ks(std::vector<float> &scores, std::vector<float> &labels);
    float cal_auc(std::vector<float> &scores, std::vector<float> &labels);
}

#endif //XSCE_THIRD_PARTY_LR_ALG_LR_H
