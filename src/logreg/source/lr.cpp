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
 * @file lr.cpp
 * @author Created by wumingzi. 2023:02:15,Wednesday,14:41:29.
 * @brief 
 * @version 
 * @date 2023-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "lr.h"
#include "lr_ose.h"
#include "toolkits/util/include/xlog.h"
#include "common/pub/include/util.h"
#include "PSI/include/psi.h"
#include "toolkits/util/include/task_status.h"

#include "arithmetic/include/base-compute.h"

namespace xsceLogRegAlg
{
    using namespace std;
    using namespace oc;
    using namespace util;
    using namespace xsce_ose;

    int64_t logReg2PartyAlg(OptAlg *opt)
    {
        int64_t rlt = -1;
        LRTrainAlgOpt cptOpt;
        // RltCheckOpt checkOpt;
        std::string rltFile = opt->rltFn;
        int comptMode = opt->flMode;

        int verticalMode = 0;
        int horizontalMode = 1;
        int rltCol = 0;
        std::string algName = "logistic_regression_2_party";

        setLogReg2pAlgOpt(opt, &cptOpt);

        LOG_INFO("   --- ---  show log reg 2p  opt   --- ---  ");
        LOG_INFO(showLogReg2pAlgOpt(&cptOpt));
        LOG_INFO("   --- ---  show log reg 2p over --- ---  ");


        if (verticalMode == comptMode)
        {
            lrTrainQuick(cptOpt);
        }

        //here to save model rlt to file
        std::string fn = opt->rltFn;
        LOG_INFO("save param_w="<<cptOpt.param_w.size()<<" to rlt file="<<fn);
        saveVec2File(cptOpt.param_w,fn);

        rlt = 1;
        return rlt;
    }

    void setLogReg2pAlgOpt(OptAlg *algOpt, LRTrainAlgOpt *cptOpt)
    {
        // set ip port

        bool srvFlag = false;
        if (0 == algOpt->role)
        {
            // server use local ip&port
            cptOpt->ip = algOpt->ipVec[0];
            cptOpt->port = algOpt->portVec[0];

            cptOpt->rmtIp = algOpt->ipVec[1];
            cptOpt->rmtPort = algOpt->portVec[1];
            srvFlag = true;
        }
        else
        {
            // client use server side ipaddr&port.
            cptOpt->ip = algOpt->ipVec[0];
            cptOpt->port = algOpt->portVec[0];

            cptOpt->rmtIp = algOpt->ipVec[1];
            cptOpt->rmtPort = algOpt->portVec[1];
        }

        cptOpt->role = algOpt->role;
        cptOpt->inner_parallels = algOpt->aby2ThdNum;
        cptOpt->port_num = algOpt->portNum;
        cptOpt->cirDir = algOpt->circuitDir;
        cptOpt->networkmode = algOpt->networkmode;
        cptOpt->gateway_cert_path = algOpt->gateway_cert_path;
        cptOpt->endpointlist_spdz = algOpt->endpointlist_spdz;
        cptOpt->endpointlist_spdz_gw = algOpt->endpointlist_spdz_gw;

        // for now,only support vertical mode.
        //  cptOpt->compt_mode = 0;

        int64_t dataRow = algOpt->sampleNum;
        int64_t dataCol = algOpt->featureNum;
        cptOpt->train_data_part.resize(0);
        cptOpt->labels.resize(0);
        cptOpt->test_data_part.resize(0);
        cptOpt->test_labels.resize(0);

        // here get remote feature num  .Modified by wumingzi. 2021:11:10,Wednesday,20:36:29.
        uint32_t rmtDataCol = 0;
        rmtDataCol = getUint32FromRmt(algOpt, dataCol, "log2p");
        LOG_INFO("2p log reg alg. get rmt feature num=" << rmtDataCol);
        cptOpt->rmtFeatureNum = rmtDataCol;
        //   .Modification over by panguangming/wumingzi. 2021:11:10,Wednesday,20:36:46.
        LOG_INFO("setLogReg2pAlgOpt,dataRow=" << dataRow << ",data col=" << dataCol);

        if (dataCol < 1)
        {
            LOG_INFO("log reg 2p alg. data feature number is invalid = " << dataCol);
            return;
        }
        if (dataRow < 1)
        {
            LOG_INFO("log reg 2p  alg. data sample number is invalid = " << dataRow);
            return;
        }

        if (nullptr == algOpt->sampleDataBuf)
        {
            LOG_INFO("log reg 2p  alg. data sample buf is invalid null.");

            return;
        }

        if (nullptr == algOpt->lableDataBuf && srvFlag)
        {
            LOG_INFO("log reg 2p  alg. server data label buf is invalid null.");
            return;
        }

        // init taskInfo & simdlen  .Modified by panguangming/wumingzi. 2022:03:15,Tuesday,16:30:40.
        if (nullptr != algOpt->globalCfg)
        {
            cptOpt->cfg = algOpt->globalCfg;
            LOG_INFO("save global cfg to cptOpt");
        }

        cptOpt->taskId = algOpt->taskId;
        cptOpt->simdLen = algOpt->simdLen;

        // for multi thread purpose  .Modified by panguangming/wumingzi. 2022:04:10,Sunday,11:16:20.
        cptOpt->threadNum = algOpt->thdNum;
        cptOpt->cpuNum = algOpt->cpuNum;
        //   .Modification over by panguangming/wumingzi. 2022:04:10,Sunday,11:16:38.

        //   .Modification over by panguangming/wumingzi. 2022:03:15,Tuesday,16:30:51.

        // support spdz innder product  .Modified by panguangming/wumingzi. 2022:04:20,Wednesday,21:50:49.
        cptOpt->type = algOpt->type;
        LOG_INFO("alg type=" << algOpt->type);
        //   .Modification over by panguangming/wumingzi. 2022:04:20,Wednesday,21:51:03.

        // init databuf;
        int64_t dataColIndex = 0;
        cptOpt->train_data_part.resize(dataRow);
        cptOpt->labels.resize(dataRow);
        cptOpt->param_w.resize(dataCol);
        double inputData = 0;
        int64_t offset = 0;

        for (int64_t i = 0; i < dataRow; i++)
        {

            cptOpt->train_data_part[i].resize(dataCol + offset);

            if (srvFlag)
            {
                cptOpt->labels[i] = algOpt->lableDataBuf[i];
            }
            else
            {
                cptOpt->labels[i] = 0;
            }

            for (int64_t j = 0; j < dataCol; j++)
            {
                inputData = algOpt->sampleDataBuf[i][j];
                cptOpt->train_data_part[i][j + offset] = inputData;
            }
        }

        // init test data.
        if (algOpt->testSampleNum > 0) // here test daa is valid.
        {
            dataRow = algOpt->testSampleNum;

            if (nullptr == algOpt->testSampleDataBuf)
            {
                LOG_INFO("[error] test sample data buf  is null");
                return;
            }
            if (srvFlag && nullptr == algOpt->testLableDataBuf)
            {
                LOG_INFO("[error] test label data buf  is null");
                return;
            }

            cptOpt->test_data_part.resize(dataRow);
            cptOpt->test_labels.resize(dataRow);
            for (int64_t i = 0; i < dataRow; i++)
            {

                cptOpt->test_data_part[i].resize(dataCol);

                if (srvFlag)
                {
                    cptOpt->test_labels[i] = algOpt->testLableDataBuf[i];
                }
                else
                {
                    cptOpt->test_labels[i] = 0;
                }

                for (int64_t j = 0; j < dataCol; j++)
                {
                    inputData = algOpt->testSampleDataBuf[i][j];
                    cptOpt->test_data_part[i][j] = inputData;
                }
            }
        }
        else // here for debug. use train data to mock test_data
        {

            LOG_INFO("setLogReg2pAlgOpt. use train data to mock test_data");
            cptOpt->test_data_part.resize(dataRow);
            cptOpt->test_labels.resize(dataRow);
            for (int64_t i = 0; i < dataRow; i++)
            {

                cptOpt->test_data_part[i].resize(dataCol);

                if (srvFlag)
                {
                    cptOpt->test_labels[i] = algOpt->lableDataBuf[i];
                }
                else
                {
                    cptOpt->test_labels[i] = 0;
                }

                for (int64_t j = 0; j < dataCol; j++)
                {
                    inputData = algOpt->sampleDataBuf[i][j];
                    cptOpt->test_data_part[i][j] = inputData;
                }
            }
        }

        cptOpt->auc = 0;
        cptOpt->ks = 0;

        // here to init learning parameters
        cptOpt->params.mIterations = algOpt->iteration;
        int64_t learnRate = algOpt->learningRate;
        double lrRate = 1.0 / (double)(1 << learnRate);
        cptOpt->params.mLearningRate = lrRate;

        cptOpt->params.mBatchSize = algOpt->batchSize;
        if (cptOpt->params.mBatchSize > cptOpt->train_data_part.size())
            cptOpt->params.mBatchSize = cptOpt->train_data_part.size();

        return;
    }

    std::string showLogReg2pAlgOpt(LRTrainAlgOpt *opt)
    {
        std::string rlt;
        std::stringstream log;

        log << "show log reg 2p  alg opt" << std::endl;
        log << "role=" << opt->role << ",addr=" << opt->ip << ":" << opt->port << ",cir dir=" << opt->cirDir << std::endl;
        log << "port num=" << opt->port_num << ",innerThd=" << opt->inner_parallels << std::endl;
        log << "rmt ip=" << opt->rmtIp << ",rmt port=" << opt->rmtPort << ",no testFlag" << std::endl;

        log << "train data sample size=" << opt->train_data_part.size();
        if (opt->train_data_part.size() > 0)
            log << ",data feature size=" << opt->train_data_part.at(0).size();

        log << ",test data sample size=" << opt->test_data_part.size();
        if (opt->test_data_part.size() > 0)
            log << ",test data feature size=" << opt->test_data_part.at(0).size();
        log << std::endl;

        log << "lr rate=" << opt->params.mLearningRate << ",iteration=" << opt->params.mIterations << ",batch=" << opt->params.mBatchSize << std::endl;
        log << "lr rmtFeatureNum=" << opt->rmtFeatureNum << ",type=" << opt->type << std::endl;
        log << "lr taskId=" << opt->taskId << ",cfg ptr=" << opt->cfg << ",simdlen=" << opt->simdLen << std::endl;
        log << "lr threadNum=" << opt->threadNum << ",cpu num=" << opt->cpuNum << ",simdlen=" << opt->simdLen << std::endl;
        log << std::endl;

        return log.str();
    }
    float cal_ks(std::vector<float> &scores, std::vector<float> &labels)
    {
        int pos_num = 0;
        int neg_num = 0;
        int sample_num = scores.size();
        std::multimap<float, float> score_label_map;
        for (int i = 0; i < sample_num; i++)
        {
            if (labels[i] == 1)
                pos_num++;
            else
                neg_num++;
            score_label_map.insert(std::make_pair(scores[i], labels[i]));
        }

        float ks = 0;
        float goodnum = 0;
        float badnum = 0;
        for (auto sl : score_label_map)
        {
            if (sl.second == 1)
                goodnum++;
            else
                badnum++;

            float rate = std::abs(goodnum / pos_num - badnum / neg_num);
            if (rate > ks)
                ks = rate;
        }

        return ks;
    }

    float cal_auc(std::vector<float> &scores, std::vector<float> &labels)
    {
        int sample_num = scores.size();
        long long pos_num = 0;
        long long neg_num = 0;
        std::multimap<float, float> score_label_map;
        for (int i = 0; i < sample_num; i++)
        {
            if (labels[i] == 1)
                pos_num++;
            else
                neg_num++;
            score_label_map.insert(std::make_pair(scores[i], labels[i]));
        }

        long long rank_sum = 0;
        int rank = 1;
        for (auto sl : score_label_map)
        {
            if (sl.second == 1)
            {
                rank_sum += rank;
            }
            rank++;
        }

        float auc = (rank_sum - (pos_num * (pos_num + 1) / 2)) * 1.0 / (pos_num * neg_num);

        return auc;
    }

    int dotMulSimdMultiThd(Aby2Opt opt)
    {
        int rlt = -1;
        int thdNumInput = 1;
        int thdMax = 32;
        int role = opt.role;

        std::vector<Aby2Opt> optVec(thdNumInput);
        int64_t totalLen = opt.totalLen;
        int32_t dataSlice = thdNumInput;
        uint64_t dataSliceLen = std::floor((float)totalLen / dataSlice) + 1; //
        uint64_t offset = 0;
        uint64_t curLen = 0;

        int32_t thdNum = std::ceil((float)totalLen / dataSliceLen);

        LOG_INFO("dotMulSimdMultiThd totalLen=" << totalLen << ",dataSlice=" << dataSlice << ",dataSliceLen=" << dataSliceLen << ",thdNum=" << thdNum);

        std::vector<float> *rltVec = opt.rltVec;
        std::string ip = opt.ip0;
        int port = opt.port0;

        std::vector<float> *data = opt.data;
        int batchnum = opt.nvals;
        int data_len = data->size();
        int total_num = batchnum * totalLen;
        LOG_INFO("dotMulSimdMultiThd data_len=" << data_len << ",total_num=" << total_num << ",batchnum=" << batchnum << ",");
        std::vector<float> srv_vec(data_len);
        std::vector<float> cli_vec(data_len);
        std::vector<float> send_vec(data_len);
        std::vector<float> rcv_vec(data_len);

        //batchnum sample * totalLen * feature num
        if (0 == role)
        {
            for (int64_t i = 0; i < data_len; i++)
            {
                srv_vec.at(i) = data->at(i);
            }
            sendChTcpVecFloat(ip, port, rcv_vec, srv_vec);
        }
        else
        {
            for (int64_t i = 0; i < data_len; i++)
            {
                cli_vec.at(i) = data->at(i);
            }
            rcvChTcpVecFloat(ip, port, rcv_vec, send_vec);
            LOG_INFO("show rcv_vec \n");
            show_vec(rcv_vec);

            LOG_INFO("show dor_rlt_vec \n");
            dotMulFloatVec(&cli_vec, &rcv_vec, rltVec, batchnum);
            show_vec(*rltVec);
        }

        rlt = 0;
        return rlt;
    }

    int lrTrainQuick(LRTrainAlgOpt &opt)
    {
        int rlt = -1;
        int mt_alg = 0;
        int role = opt.role;
        uint32_t secparam = opt.secparam;
        // seclvl seclvl = get_sec_lvl(secparam);
        uint32_t nthreads = opt.inner_parallels;
        uint32_t nthreadpool = opt.port_num;
        uint32_t port = opt.port;
        std::string address = opt.ip;
        int32_t spdzPortNum = 0;     // for spdz port idle use
        int32_t spdzPortUsedCnt = 1; // for spdz port idle use
        int32_t aby2PortUsecCnt = 1;

        uint32_t rmtPort = opt.rmtPort;     // fix by light, 2022.05.07
        std::string rmtAddress = opt.rmtIp; // fix by light, 2022.05.07

        LOG_INFO( "ip address in lrTrainAby2Quick is: " << address ); //add by light, 2022.05.07
        LOG_INFO( "port in lrTrainAby2Quick is: " << port );
        LOG_INFO( "rmt ip address in lrTrainAby2Quick is: " << rmtAddress );
        LOG_INFO( "rmt port in lrTrainAby2Quick is: " << rmtPort );

        spdzPortNum = opt.port_num;
        if (spdzPortNum < 1)
        {
            spdzPortNum = 1;
        }

        RegressionParam &params = opt.params;
        auto cirDir = opt.cirDir;
        auto &param_w = opt.param_w;
        int showCnt = 3;

        GlobalCfg *cfg = opt.cfg;
        std::string taskId = opt.taskId;

        // support spdz inner product  .Modified by panguangming/wumingzi. 2022:04:20,Wednesday,21:55:25.
        int type = opt.type;
        //   .Modification over by panguangming/wumingzi. 2022:04:20,Wednesday,21:55:37.

        if (nullptr == cfg)
        {
            LOG_INFO("LRTrainOpt opt is invalid, GlobalCfg pointer is null");
            return rlt;
        }

        if (taskId.length() < 1)
        {
            LOG_INFO("LRTrainOpt opt is invalid, taskId is null");
            return rlt;
        }

        int64_t step_start_ms, step_end_ms;
        // step_start_ms = TimeUtil::CurrentTimeInMilliseconds();
        auto &train_data_part = opt.train_data_part;
        auto &labels = opt.labels;
        int samplenum = train_data_part.size();

        // the learning rate in log2 form. We will truncate this many bits.
        // uint64_t lr = std::log2(1 / (params.mLearningRate / params.mBatchSize));
        float lrb = params.mLearningRate / params.mBatchSize;

        auto fnum = train_data_part[0].size();
        auto batchnum = params.mBatchSize;
        int64_t simdLen = opt.simdLen;
        int64_t simdMin = 10000;

        if (simdLen < simdMin)
            simdLen = simdMin;

        int32_t batchFeature = simdLen / batchnum;

        if (batchFeature < 1)
            batchFeature = 1;

        int32_t clientFeatureNum = 0;

        if (CLIENT == role)
        {
            clientFeatureNum = fnum;
        }
        else
        {
            clientFeatureNum = opt.rmtFeatureNum;
        }

        // batch operation to speed up  .Modified by panguangming/wumingzi. 2022:04:08,Friday,22:28:17.
        int loopNum = std::ceil((float)clientFeatureNum / batchFeature);

        LOG_INFO("lrTrainQuick. sampleNum=" << samplenum << ",fnum=" << fnum << ",clientFeatureNum=" << clientFeatureNum);
        LOG_INFO("batchnum=" << batchnum << ",simdLen=" << simdLen << ",batchFeature=" << batchFeature << ",loopNum=" << loopNum);

        // here use rndSampleIdx to save random id index,
        // so in each iterration training, select random sample index here.
        std::vector<int32_t> rndSampleIdx;
        setVecPermutation(rndSampleIdx, samplenum);

        // here to sycn random sample id   .Modified by panguangming/wumingzi. 2022:04:27,Wednesday,10:14:35.
        showCnt = showCnt > samplenum ? samplenum : showCnt;

        uint64_t *rndIdx = nullptr;
        if (role == SERVER)
        {
            rndIdx = allocateUInt64Vec(samplenum);
            if (nullptr == rndIdx)
            {
                LOG_INFO("lrTrainAby2Simd  rndIdx buffer error");
                return rlt;
            }

            for (int64_t i = 0; i < samplenum; i++)
            {
                rndIdx[i] = rndSampleIdx.at(i);
                if (i < showCnt || i >= samplenum - showCnt)
                {
                    LOG_INFO(i << ": rndSampleIdx=" << rndSampleIdx.at(i));
                }
            }
        }

        int dataLenIdx = sizeof(uint64_t) / sizeof(uint8_t);
        int sendLen = samplenum * dataLenIdx;

        LOG_INFO("sycn rnd sample id datalen=" << sendLen);

        // here need to server send rnd sampleIdx to client  .Modified by panguangming/wumingzi. 2022:04:24,Sunday,22:58:44.

        if (role == SERVER)
        {

            // xsceUtil::sendChBufSyncTcp(address, port, (uint8_t *)rndIdx, sendLen);

            LOG_INFO("send rndIdx rmtAddress="<<rmtAddress<<",rmtPort="<<rmtPort);
            sendChBufSyncTcp(rmtAddress, rmtPort, (uint8_t *)rndIdx, sendLen); // fix by light, 2022.05.07
            LOG_INFO("send rndIdx over.");
        }
        else
        {
            // auto rcvLen = xsceUtil::rcvChBufSyncTcp(address, port, (uint8_t **)&rndIdx, sendLen);
            LOG_INFO("rcv rndIdx rmtAddress="<<rmtAddress<<",rmtPort="<<rmtPort);
            auto rcvLen = rcvChBufSyncTcp(rmtAddress, rmtPort, (uint8_t **)&rndIdx, sendLen); // fix by light, 2022.05.07
            if (rcvLen != sendLen)
            {
                LOG_INFO("lrTrainAby2Simd recv client rnd idx error at rcvLen=" << rcvLen);
                return rlt;
            }

            for (int64_t i = 0; i < samplenum; i++)
            {
                rndSampleIdx.at(i) = rndIdx[i];
                if (i < showCnt || i >= samplenum - showCnt)
                {
                    LOG_INFO(i << ": rndSampleIdx=" << rndSampleIdx.at(i));
                }
            }
        }

        LOG_INFO("samplenum=" << samplenum << ",showCnt=" << showCnt);

        // end

        int fininshedPercentage = 0;
        uint64_t totalSampleTrain = params.mIterations * batchnum;
        uint64_t totalSampleTrainFeature = totalSampleTrain * fnum;
        LOG_INFO("total train sample number=" << totalSampleTrain);
        LOG_INFO("total train sample feature number=" << totalSampleTrainFeature);

        // for performance promotion debug  .Modified by panguangming/wumingzi. 2022:04:08,Friday,16:04:34.
        TimeUtils t1, t2, t3, t4;
        int itNum = params.mIterations;
        t3.start("training begin with iteration ", itNum);

        // init aby2 opt  .Modified by panguangming/wumingzi. 2022:04:09,Saturday,22:53:21.
        Aby2Opt abyOpt;

        //   .Modification over by panguangming/wumingzi. 2022:04:09,Saturday,22:53:28.

        for (uint64_t it2 = 0; it2 < totalSampleTrain; it2 += batchnum)
        {
            LOG_INFO("quick lr. mpc batch index: " << it2);
            auto itBase = it2 % samplenum;
            if (itBase + batchnum >= samplenum)
                itBase = samplenum - batchnum;

            std::vector<float> error(batchnum, 0);
            std::vector<float> mul_sum(batchnum, 0);
            std::vector<float> new_sum(batchnum, 0);
            std::vector<float> plain_sum(batchnum, 0);
            float error_sum = 0;

            t1.start("iteration,", it2 / batchnum);
            uint64_t curTotal = it2 * fnum;

            for (int b = 0; b < batchnum; b++)
            {
                for (int i = 0; i < fnum; i++)
                {
                    mul_sum[b] += param_w[i] * train_data_part[rndSampleIdx.at(itBase + b)][i];
                }
            }

            // LOG_INFO( "001" );

            // LOG_INFO( "002" );
            // here no need to use aby2 to compute mul_sum,   .Modified by panguangming/wumingzi. 2022:05:08,Sunday,23:13:19.
            if (role == SERVER)
            {
                LOG_INFO("server to exchange mul sum with client");
                auto mulNum = rcvChTcpVecFloat(address, port, new_sum, mul_sum);
                auto sumNum = plain_sum.size();
                LOG_INFO("server to exchange mul sum with client. sumNum=" << sumNum);
                if (sumNum != batchnum)
                {
                    LOG_INFO("server to exchange mul sum with client error. sumNum=" << sumNum << " is not matched with batch=" << batchnum);
                }
                // for (int64_t k = 0; k < sumNum; k++)
                // {
                //     if (plain_sum.at(k) != new_sum.at(k))
                //     {
                //         LOG_INFO( "server to exchange mul sum with client. error at i=" << k << plain_sum.at(k) << "--" << new_sum.at(k) );
                //     }
                // }
            }
            else
            {
                LOG_INFO("client to exchange mul sum with server");
                auto mulNum = sendChTcpVecFloat(address, port, new_sum, mul_sum);
                auto sumNum = plain_sum.size();
                LOG_INFO("client to exchange mul sum with server. sumNum=" << sumNum);
                if (sumNum != batchnum)
                {
                    LOG_INFO("client to exchange mul sum with server error. sumNum=" << sumNum << " is not matched with batch=" << batchnum);
                }
                // for (int64_t k = 0; k < sumNum; k++)
                // {
                //     if (plain_sum.at(k) != new_sum.at(k))
                //     {
                //         LOG_INFO( "server to exchange mul sum with client. error at i=" << k << plain_sum.at(k) << "--" << new_sum.at(k) );
                //     }
                // }
            }

            for (int64_t k = 0; k < batchnum; k++)
            {
                new_sum.at(k) += mul_sum.at(k);
            }
            //   .Modification over by panguangming/wumingzi. 2022:05:08,Sunday,23:13:58.

            if (role == SERVER)
            {
                for (int b = 0; b < batchnum; b++)
                {
                    float fxw = 1.0 / (1 + std::exp(-new_sum[b]));
                    error[b] = fxw - labels[rndSampleIdx.at(itBase + b)];
                    error_sum += error[b] * error[b];
                }

                for (int i = 0; i < fnum; i++)
                {
                    float update = 0;
                    for (int b = 0; b < batchnum; b++)
                    {
                        update += train_data_part[rndSampleIdx.at(itBase + b)][i] * error[b];
                    }
                    update *= lrb;
                    param_w[i] -= update;
                }
            }
            // LOG_INFO( "003" );

            int featureBase = 0;
            int loopLen = batchFeature; // loopLen indicates how many client features updated in this batch.
            std::vector<float> data;
            for (int64_t j = 0; j < loopNum; j++)
            {
                t2.start("feature batch update loop", j);
                featureBase = j * batchFeature;
                if (featureBase + loopLen >= clientFeatureNum)
                {
                    loopLen = clientFeatureNum - featureBase;
                    LOG_INFO("set feature len=" << loopLen);
                }

                data.resize(loopLen * batchnum);

                int64_t dataBase = 0;
                if (SERVER == role) // for server side, fill in data vec to store error value
                {
                    for (int64_t i = 0; i < loopLen; i++)
                    {
                        dataBase = i * batchnum;
                        for (int64_t k = 0; k < batchnum; k++)
                        {
                            data[k + dataBase] = error[k];
                        }
                    }
                }
                else // for client side, fill in data vec to store client feature value
                {
                    for (int64_t i = 0; i < loopLen; i++)
                    {
                        dataBase = i * batchnum;
                        for (int64_t b = 0; b < batchnum; b++)
                        {
                            data[b + dataBase] = train_data_part[rndSampleIdx.at(itBase + b)][i + featureBase];
                        }
                    }
                }

                // here loopLen client feature value is ready for batch simd aby2 add operation.
                LOG_INFO(".. .. "
                         << "loop[" << j << "], feature base=" << featureBase << ",feature len=" << loopLen);

                std::vector<float> updateRlt;
                abyOpt.role = role;
                abyOpt.address = address;
                abyOpt.port = port;
                abyOpt.secparam = opt.secparam;
                // abyOpt.sharing = S_BOOL;
                abyOpt.nthreads = nthreads;
                abyOpt.mt_alg = mt_alg;
                abyOpt.circuit_dir = cirDir;

                abyOpt.nvals = batchnum;
                abyOpt.data = &data;
                abyOpt.rltVec = &updateRlt;
                abyOpt.threadNum = opt.threadNum;

                abyOpt.totalLen = loopLen;
                abyOpt.curLen = loopLen;
                abyOpt.threadNum = opt.threadNum;
                abyOpt.cpuNum = opt.cpuNum;
                abyOpt.offset = 0;
                updateRlt.resize(loopLen);

                // for spdz alg  .Modified by panguangming/wumingzi. 2022:04:20,Wednesday,22:32:55.
                abyOpt.ip0 = opt.ip;
                abyOpt.ip1 = opt.rmtIp;
                abyOpt.port0 = opt.port;
                abyOpt.port1 = opt.rmtPort;
                abyOpt.networkmode = opt.networkmode;
                abyOpt.gateway_cert_path = opt.gateway_cert_path;
                abyOpt.endpointlist_spdz = opt.endpointlist_spdz;
                abyOpt.endpointlist_spdz_gw = opt.endpointlist_spdz_gw;
                abyOpt.taskId = taskId;

                //   .Modified by panguangming/wumingzi. 2022:04:20,Wednesday,21:59:00.
                int simdRlt = 0;

                static int spdzCnt = 0;
                t4.start("dot mul begin ", spdzCnt++);
                if (0 == type)
                {

                    //   .Modification over by panguangming/wumingzi. 2022:05:10,Tuesday,09:54:21.
                    LOG_INFO("use plaintext dot mul by default. type=" << std::dec << type << ",port0=" << abyOpt.port0 << ",port1=" << abyOpt.port1);
                    simdRlt = dotMulSimdMultiThd(abyOpt);
                    // simdRlt = 0;
                }
                else //send dot mul plaintext
                {
                    // here to used different port within portnum range  to fix spdz port busy bug.Modified by panguangming/wumingzi. 2022:05:10,Tuesday,09:54:06.
                    abyOpt.port0 += spdzPortUsedCnt % spdzPortNum;
                    abyOpt.port1 += spdzPortUsedCnt % spdzPortNum;
                    spdzPortUsedCnt++;
                    //   .Modification over by panguangming/wumingzi. 2022:05:10,Tuesday,09:54:21.
                    LOG_INFO("use spdz dot mul. type=" << std::dec << type << ",port0=" << abyOpt.port0 << ",port1=" << abyOpt.port1 << ",loopLen=" << loopLen);
                    simdRlt = spdzDotMulSimdMultiThd(abyOpt, j);
                    // simdRlt = dotMulSimdMultiThd(abyOpt);
                }
                t4.stopS("dot mul over ", spdzCnt);

                //   .Modification over by panguangming/wumingzi. 2022:04:20,Wednesday,21:59:04.

                // here to update paraw
                int updateLen = updateRlt.size();

                if (updateLen != loopLen)
                {
                    LOG_INFO("lrTrainAby2 quick. batch rlt =" << updateLen << " is error,loopLen=" << loopLen);
                    return -1;
                }

                if (CLIENT == role)
                { // for client side, update para
                    float update = 0;
                    for (int64_t i = 0; i < updateLen; i++)
                    {
                        update = updateRlt.at(i) * lrb;
                        param_w[featureBase + i] -= update;
                        LOG_INFO(featureBase + i << ":update=" << update << ",param_w[featureBase + i]=" << param_w[featureBase + i]);
                    }
                }

                curTotal += batchnum * loopLen;
                fininshedPercentage = curTotal * 100 / totalSampleTrainFeature;
                // saveTaskMapProcess(taskId, fininshedPercentage, cfg);

                LOG_INFO("quick aby2 lr. cur progress = " << fininshedPercentage);
                t2.stopS("feature batch update over ", j);
            }

            if (it2 > 0)
            {
                LOG_INFO("Iteration: " << it2 / batchnum);
                LOG_INFO("L2 Error: " << error_sum);
            }

            t1.stopS("iteration,", it2 / batchnum);
        }

        LOG_INFO("show training model paras: \n");
        show_vec(param_w);
        t3.stopS("training over.");

        // test data no need to sample random.
        auto &test_data_part = opt.test_data_part;
        auto &test_labels = opt.test_labels;
        std::vector<float> test_scores;
        int testnum = test_data_part.size();
        LOG_INFO("begin to predict : testNum=" << testnum << ",local para size=" << param_w.size());

        t3.start("predicting begin with iteration ", testnum);

        // here to allocate buf  .Modified by panguangming/wumingzi. 2022:03:25,Friday,16:39:05.
        std::vector<float> local_scores;
        std::vector<float> total_scores;
        int memNum = (testnum + 1) / 2 * 2;
        LOG_INFO("testnum=" << testnum << ",memNum=" << memNum);

        float *localScore = allocateFloatVec(memNum);
        float *rmtScore = nullptr;
        uint8_t *rcvScore = nullptr;

        if (nullptr == localScore)
        {
            LOG_INFO("lrTrainAby2Simd allocate memory buffer error at testNum=" << testnum);
            return rlt;
        }

        for (int b = 0; b < testnum; b++)
        {
            float score = 0;
            for (int i = 0; i < fnum; i++)
            {
                score += param_w[i] * test_data_part[b][i];
            }

            localScore[b] = score;
        }
        // here client to send local scores to server side. no need to use mpc to calculate sum.
        if (role == SERVER)
        {
            auto rcvLen = rcvChBufSyncTcp(address, port, &rcvScore, testnum * 4);
            if (rcvLen != 4 * testnum)
            {
                LOG_INFO("lrTrainAby2Simd recv client buffer error at testNum=" << testnum);
                return rlt;
            }
        }
        else
        {
            sendChBufSyncTcp(address, port, (uint8_t *)localScore, testnum * 4);
        }

        float new_score = 0;
        rmtScore = (float *)rcvScore;
        for (int b = 0; b < testnum; b++)
        {

            if (role == SERVER)
            {
                new_score = localScore[b] + rmtScore[b];
                float fxw = 1.0 / (1 + std::exp(-new_score));
                test_scores.push_back(fxw);
                if (b < 10)
                    LOG_INFO("server predict test sample at " << b << ", result = " << fxw << ",new_score=" << new_score << ",rmtScore=" << rmtScore[b]);
            }
            else
            {
                // LOG_INFO( "client predict test sample result at " << b << ", localScore=" << localScore[b] );
            }
        }

        if (role == SERVER)
        {
            LOG_INFO("mpc labels-scores: ");
            for (int j = 0; j < 10; j++)
            {
                LOG_INFO("(" << test_labels[j] << ", " << test_scores[j] << "), ");
            }
            LOG_INFO("\n");

            float auc = cal_auc(test_scores, test_labels);
            opt.auc = auc;
            LOG_INFO("mpc auc: " << auc);
            float ks = cal_ks(test_scores, test_labels);
            opt.ks = ks;
            LOG_INFO("mpc ks: " << ks);
        }

        // t3.stopS("predicting over.");
        // modified by light, 2022.05.12
        opt.predictTime = t3.strStopS("predicting over.");

        return 0;
    }

    int spdzDotMulSimdMultiThd(Aby2Opt &opt, int round_seq)
    {
        int rlt = -1;
        int thdNumInput = opt.threadNum;
        int thdMax = 32;
        int batchNum = opt.nvals; //sample number for each feature

        if (thdNumInput < 1)
        {
            LOG_INFO("spdzDotMulSimdMultiThd, thdNum is invalid=" << thdNumInput);
            return rlt;
        }

        if (thdNumInput > thdMax)
        {
            LOG_INFO("spdzDotMulSimdMultiThd, thdNum is too large " << thdNumInput);
            return rlt;
        }

        if (batchNum < 1)
        {
            LOG_INFO("spdzDotMulSimdMultiThd, batchNum is invalid=" << batchNum);
            return rlt;
        }

        //init data
        int chunk = batchNum;
        auto len = opt.data->size();
        if (len < 1 || chunk < 1)
        {
            LOG_INFO("spdz dot mul alg input error . chunk=" << chunk << ",data size=" << len);
            return rlt;
        }

        if (0 != len % chunk)
        {
            LOG_INFO("spdz dot mul alg input len error . chunk=" << chunk << ",data size=" << len);
            return rlt;
        }

        if (nullptr == opt.data)
        {
            LOG_INFO("spdz dot mul alg input data pointer is null");
            return rlt;
        }
        if (nullptr == opt.rltVec)
        {
            LOG_INFO("spdz dot mul alg input result pointer is null");
            return rlt;
        }

        LOG_INFO(std::dec << "spdz dot mul alg. chunk=" << chunk << ",data size=" << len);
        SPDZAlg alg;
        alg.SetParam(&opt, round_seq);
        LOG_INFO(std::dec << "opt ip0=" << opt.ip0 << ",ip1=" << opt.ip1 << ",port0=" << opt.port0 << ",port1=" << opt.port1);
        LOG_INFO(std::dec << "opt.networkmode=" << opt.networkmode
                          << " opt.gateway_cert_path=" << opt.gateway_cert_path
                          << " opt.endpointlist_spdz=" << opt.endpointlist_spdz
                          << " opt.endpointlist_spdz_gw=" << opt.endpointlist_spdz_gw);
        LOG_INFO(std::dec << "endpoint_spdz_vec.size()=" << alg.endpoint_spdz_vec.size());
        LOG_INFO(std::dec << "endpointlist_spdz=" << alg.endpointlist_spdz);

        alg.input.resize(len);
        for (int64_t i = 0; i < len; i++)
        {
            alg.input.at(i) = opt.data->at(i);
        }

        LOG_INFO("begin to call spdz alg");

        auto spdzRlt = runProDot2(&alg, chunk);

        LOG_INFO("spdz alg running over. begin to save reuslt=" << spdzRlt);
        auto rltLen = alg.result.size();
        auto dotMulCnt = len / chunk;

        for (int64_t i = 0; i < rltLen; i++)
        {
            LOG_INFO("spdz dot mul rlt[" << i << "]=" << alg.result.at(i));
        }
        if (rltLen != opt.rltVec->size())
        {
            LOG_INFO("spdz dot mul alg  result size is error=" << rltLen << ",dotMulCnt=" << dotMulCnt);
            return rlt;
        }

        opt.rltVec->resize(rltLen);
        for (int64_t i = 0; i < rltLen; i++)
        {
            opt.rltVec->at(i) = alg.result.at(i);
        }
        rlt = rltLen;
        return rlt;
    }

    void dotMulFloatVec(std::vector<float> *vec1, std::vector<float> *vec2, std::vector<float> *rltVec, int batch)
    {
        if (nullptr == vec1 || nullptr == vec2 || nullptr == rltVec)
        {
            LOG_INFO( "dotMulFloatVec input null \n");
            return;
        }

        auto len1 = vec1->size();
        auto len2 = vec2->size();
        if (len1 != len2 || batch < 1)
        {
            LOG_INFO( "dotMulFloatVec input len error \n");
            return;
        }

        int fea_num = len1 / batch;
        LOG_INFO("len1=" << len1 << ",len2=" << len2 << ",batch=" << batch << ",fea_num=" << fea_num);

        rltVec->resize(fea_num);

        int base = 0;
        for (int64_t i = base; i < fea_num; i++)
        {
            base = i * batch;
            float cur_rlt = 0;
            for (int64_t j = 0; j < batch; j++)
            {
                cur_rlt += vec1->at(j + base) * vec2->at(j + base);
            }
            rltVec->at(i) = cur_rlt;
        }
    }

}
