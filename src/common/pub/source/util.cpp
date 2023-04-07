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
 * @file util.cpp
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "util.h"
#include "ot.h"

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>

#include "toolkits/util/include/xlog.h"
#include "toolkits/util/include/xutil.hpp"

namespace util
{
    using namespace std;
    using namespace oc;
    using namespace xsce_ose;

    int64_t copyAlgOpt(OptAlg *optDst, OptAlg *optSrc)
    {
        int64_t rlt = 0;

        if (nullptr == optDst || nullptr == optSrc)
        {
            LOG_ERROR("copy alg opt error");
            return -1;
        }
        // init ip info.
        int len = optSrc->ipVec.size();
        optDst->ipVec.resize(len);
        for (int64_t i = 0; i < len; i++)
        {
            optDst->ipVec[i] = optSrc->ipVec[i];
        }

        optDst->taskId = optSrc->taskId;
        optDst->dbg = optSrc->dbg;
        optDst->nodeToken = optSrc->nodeToken;
        optDst->oriRole = optSrc->oriRole;
        optDst->cfgFile = optSrc->cfgFile;
        optDst->secuK = optSrc->secuK;
        optDst->bitlen = optSrc->bitlen;
        optDst->simdLen = optSrc->simdLen;
        optDst->stdoutFlag = optSrc->stdoutFlag;

        optDst->algIndexStr = optSrc->algIndexStr;
        optDst->alg = optSrc->alg;
        optDst->regressionFlag = optSrc->regressionFlag;

        len = optSrc->algIndexVec.size();
        optDst->algIndexVec.resize(len);
        for (int64_t i = 0; i < len; i++)
        {
            optDst->algIndexVec.at(i) = optSrc->algIndexVec.at(i);
        }

        optDst->portNum = optSrc->portNum;
        optDst->thdNum = optSrc->thdNum;
        optDst->aby2ThdNum = optSrc->aby2ThdNum;
        optDst->thdOver = optSrc->thdOver;

        optDst->startLoc = optSrc->startLoc;
        optDst->dataLen = optSrc->dataLen;

        // use bucketNum to replace dataLen  .Modified by wumingzi. 2023:02:21,Tuesday,10:52:48.
        optDst->bucketNum = optSrc->bucketNum;
        //   .Modification over by wumingzi. 2023:02:21,Tuesday,10:52:58.

        optDst->circuitDir = optSrc->circuitDir;

        optDst->dataFn = optSrc->dataFn;
        optDst->row = optSrc->row;
        optDst->col = optSrc->col;
        optDst->headLine = optSrc->headLine;
        optDst->colLine = optSrc->colLine;

        optDst->sampleFile = optSrc->sampleFile;
        optDst->lableFile = optSrc->lableFile;
        optDst->testSampleFile = optSrc->testSampleFile;
        optDst->testLableFile = optSrc->testLableFile;

        optDst->rltFn = optSrc->rltFn;
        optDst->port3 = optSrc->port3;
        optDst->port4 = optSrc->port4;
        optDst->testN = optSrc->testN;
        optDst->intMul = optSrc->intMul;
        optDst->intAdd = optSrc->intAdd;
        optDst->errThd = optSrc->errThd;

        optDst->algOver = optSrc->algOver;
        optDst->logfn = optSrc->logfn;
        optDst->status = optSrc->status;
        optDst->statusPtr = optSrc->statusPtr;

        // for oprf psi alg use.
        optDst->oprfWidth = optSrc->oprfWidth;
        optDst->oprfLogHeight = optSrc->oprfLogHeight;
        optDst->oprfHashLenInBytes = optSrc->oprfHashLenInBytes;
        optDst->oprfBucket1 = optSrc->oprfBucket1;
        optDst->oprfBucket2 = optSrc->oprfBucket2;

        optDst->commonSeed = optSrc->commonSeed;
        optDst->inertalSeed = optSrc->inertalSeed;

        // computation alg
        optDst->circuitDir = optSrc->circuitDir;
        optDst->comptMode = optSrc->comptMode;
        optDst->statMode = optSrc->statMode;

        optDst->flMode = optSrc->flMode;

        optDst->featureIdIndex = optSrc->featureIdIndex;
        optDst->sampleIdIndex = optSrc->sampleIdIndex;
        optDst->lableIndex = optSrc->lableIndex;
        optDst->lableParty = optSrc->lableParty;
        optDst->iteration = optSrc->iteration;
        optDst->batchSize = optSrc->batchSize;
        optDst->learningRate = optSrc->learningRate;
        optDst->lableRequired = optSrc->lableRequired;

        optDst->psiFlag = optSrc->psiFlag;
        optDst->groupNum = optSrc->groupNum;
        optDst->ivMax = optSrc->ivMax;
        optDst->ivMin = optSrc->ivMin;

        optDst->sampleNum = optSrc->sampleNum;
        optDst->featureNum = optSrc->featureNum;
        optDst->sampleDataBuf = optSrc->sampleDataBuf;
        optDst->lableDataBuf = optSrc->lableDataBuf;

        optDst->stopCmd = optSrc->stopCmd;

        optDst->taskJobId = optSrc->taskJobId;
        optDst->globalCfg = optSrc->globalCfg;
        optDst->taskId = optSrc->taskId;

        optDst->dataRowLen = optSrc->dataRowLen;

        optDst->type = optSrc->type;
        optDst->networkmode = optSrc->networkmode;
        optDst->gateway_cert_path = optSrc->gateway_cert_path;
        optDst->endpointlist_spdz = optSrc->endpointlist_spdz;
        optDst->endpointlist_spdz_gw = optSrc->endpointlist_spdz_gw;

        return rlt;
    }

    int64_t copyThdAlgOpt(OptAlg *optDst, OptAlg *optSrc, int thdIdx)
    {
        int64_t rlt = 0;

        LOG_INFO("begin to copy thd alg opt at thd idx=" << thdIdx);
        copyAlgOpt(optDst, optSrc);
        if (thdIdx >= optSrc->thdNum)
        {
            LOG_ERROR("input thread idx = " << thdIdx << " is invalid.");
            return -1;
        }

        // copy ip&port info here.
        LOG_INFO("begin to copy alg specific opt at thd idx=" << thdIdx);
        optDst->thdIdex = thdIdx;
        int portNum = optSrc->portNum;
        int offset = thdIdx * portNum;
        int len = optSrc->portVec.size();
        optDst->portVec.resize(len);
        for (int64_t i = 0; i < len; i++)
        {
            optDst->portVec[i] = optSrc->portVec[i] + offset;
        }

        optDst->role = optSrc->role;
        optDst->addr = optSrc->addr;
        optDst->port = optSrc->port + offset;

        optDst->localParty.addr = optSrc->localParty.addr;
        optDst->localParty.port = optSrc->localParty.port + offset;
        optDst->rmtParty.addr = optSrc->rmtParty.addr;
        optDst->rmtParty.port = optSrc->rmtParty.port + offset;

        optDst->type = optSrc->type;

        optDst->cpuNum = optSrc->cpuNum;
        optDst->simdLen = optSrc->simdLen;
        optDst->aby2ThdNum = optSrc->aby2ThdNum;

        optDst->commonSeed = optSrc->commonSeed;
        optDst->commonSeed1 = optSrc->commonSeed1;
        optDst->inertalSeed = optSrc->inertalSeed;
        optDst->inertalSeed1 = optSrc->inertalSeed1;

        optDst->groupNum = optSrc->groupNum;
        optDst->comptMode = optSrc->comptMode;

        optDst->task_status = optSrc->task_status;

        return rlt;
    }

    int64_t savePirRlt2File(std::vector<std::vector<std::string> > *pirRltVec, std::string fn)
    {
        int64_t rlt = -1;

        std::stringstream log;

        if (nullptr == pirRltVec)
        {
            LOG_ERROR("savePirRltFile input pirRltVec is null " << fn);
            return rlt;
        }
        int64_t len = pirRltVec->size();

        std::ofstream outf;
        outf.open(fn.data());

        if (outf.fail())
        {
            LOG_ERROR("open file error at " << fn);
            return rlt;
        }

        rlt = 0;
        for (int64_t i = 0; i < len; i++)
        {
            std::vector<std::string> *rltStr = &pirRltVec->at(i);
            int64_t alen = rltStr->size();
            for (int64_t j = 0; j < alen; j++)
            {
                outf << rltStr->at(j);
                rlt++;
            }
        }

        outf.close();
        return rlt;
    }

    int64_t savePirRlt2StrVec(std::vector<std::vector<std::string> > *pirRltVec, std::vector<std::string> &vec)
    {
        int64_t rlt = -1;

        if (nullptr == pirRltVec)
        {
            LOG_ERROR("savePirRlt2StrVec input pirRltVec is null ");
            return rlt;
        }
        int64_t len = pirRltVec->size();
        int64_t total_len = 0;
        for (int64_t i = 0; i < len; i++)
        {
            total_len += pirRltVec->at(i).size();
        }

        vec.resize(total_len);
        rlt = 0;
        for (int64_t i = 0; i < len; i++)
        {
            std::vector<std::string> *rltStr = &pirRltVec->at(i);
            int64_t alen = rltStr->size();
            for (int64_t j = 0; j < alen; j++)
            {
                if (rlt < total_len)
                    vec.at(rlt) = rltStr->at(j);
                rlt++;
            }
        }

        return rlt;
    }

    int64_t savePsiRlt2IntVec(std::vector<std::vector<std::int64_t> > *pirRltVec, std::vector<std::int64_t> &vec)
    {
        int64_t rlt = -1;

        if (nullptr == pirRltVec)
        {
            LOG_ERROR("savePirRlt2StrVec input pirRltVec is null ");
            return rlt;
        }
        int64_t len = pirRltVec->size();
        int64_t total_len = 0;
        for (int64_t i = 0; i < len; i++)
        {
            total_len += pirRltVec->at(i).size();
        }

        vec.resize(total_len);
        rlt = 0;
        for (int64_t i = 0; i < len; i++)
        {
            std::vector<std::int64_t> *rltStr = &pirRltVec->at(i);
            int64_t alen = rltStr->size();
            for (int64_t j = 0; j < alen; j++)
            {
                if (rlt < total_len)
                    vec.at(rlt) = rltStr->at(j);
                rlt++;
            }
        }

        return rlt;
    }

    int64_t savePirRltThd(std::vector<std::vector<std::string> > &pirRltVec,
                          std::vector<std::string> *pirRlt)
    {
        int64_t rlt = -1;
        int64_t len = pirRltVec.size();

        if (len < 1)
        {
            LOG_INFO("savePirRltThd input error. pirRltVec.size() =" << len);
        }

        if (nullptr == pirRlt)
        {
            LOG_ERROR("savePirRltThd input pirRltVec is error= " << pirRlt);
            return rlt;
        }

        rlt = 0;
        for (int64_t i = 0; i < len; i++)
        {
            std::vector<std::string> *rltStr = &pirRltVec.at(i);

            int64_t alen = rltStr->size();

            for (int64_t j = 0; j < alen; j++)
            {
                pirRlt->push_back(rltStr->at(j));
                rlt++;
            }
        }

        return rlt;
    }

    uint32_t getUint32FromRmt(OptAlg *optAlg, uint32_t num, std::string chName)
    {
        int port = optAlg->port;
        std::string srvAddr = optAlg->addr;

        std::vector<uint64_t> sendVec, rcvVec, rltVec;
        rcvVec.push_back(num);
        sendVec.push_back(num);

        if (0 == optAlg->role)
        {
            LOG_INFO("this is role server. addr=" << srvAddr << ",port=" << port << ",cn=" << chName);
            rcvChTcpVecSync(srvAddr, port, rltVec, rcvVec, chName);
        }
        if (1 == optAlg->role)
        {
            LOG_INFO("this is role client. addr=" << srvAddr << ",port=" << port << ",cn=" << chName);
            sendChTcpVecSync(srvAddr, port, rltVec, sendVec, chName);
        }

        if (rltVec.size() < 1)
            return 0;
        else
            return rltVec.at(0);
    }

    int64_t exchangeSecretKey(OptAlg *optAlg)
    {

        int64_t rlt = 0;
        uint64_t s[2];
        uint64_t maxU64 = std::numeric_limits<uint64_t>::max();

        // check whether the common seed is set in previous code .Modified by wumingzi. 2023:04:06,Thursday,23:44:04.
        //if yes,no need to call exchangeSeed here and return directly
        if(optAlg->commonSeed != 0xAA55AA55AA55AA55)
        {
            LOG_INFO("no need to call  exchangeSeed here   ");
        }else
        {
            LOG_INFO("need to call exchangeSeed here because commonseed is default value="<<optAlg->commonSeed);
        }
        
        //   .Modification over by wumingzi. 2023:04:06,Thursday,23:44:39.
        
        s[0] = getRand(maxU64);
        s[1] = getRand(maxU64);

        int role = optAlg->role;
        std::string ip = optAlg->addr;
        int port = optAlg->port;

        string strFlag = optAlg->taskId + ":" + std::to_string(optAlg->algInChainIndex) + ":" + std::to_string(optAlg->thdIdex);
        exchangeSeed(role, ip, port, (uint8_t *)s, 16, strFlag.c_str()); //something wrong with this function, modified by light, 2022.0322

        optAlg->commonSeed = s[0];
        optAlg->commonSeed1 = s[1];

        // for data security,disable printing input seed  .Modified by wumingzi. 2023:04:06,Thursday,21:39:57.
        // LOG_INFO("final exchangeSecretKey seed " << std::hex << optAlg->commonSeed << ":" << optAlg->commonSeed1);
        return rlt;
    }

    int64_t cliSendBuf(OptAlg *optAlg, uint64_t *buf, uint64_t len)
    {
        int64_t rlt = -1;

        if (nullptr == optAlg)
        {
            LOG_ERROR("srvSendBuf input optAlg is null");
            return rlt;
        }
        if (nullptr == buf)
        {
            LOG_ERROR("srvSendBuf input buf is null");
            return rlt;
        }
        if (len < 1)
        {
            LOG_ERROR("srvSendBuf input len is 0");
            return rlt;
        }

        bool is_server = (0 == optAlg->role) ? true : false;

        NcoOtOpt otOpt;
        NcoOtOpt otOptRcv;
        NcoOtOpt otOptSnd;
        if (is_server)
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for srv rcv only
            otOptRcv.ip = optAlg->ipVec[1];
            otOptRcv.port = optAlg->portVec[1];
        }
        else
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for client send only
            otOptSnd.ip = optAlg->ipVec[1];
            otOptSnd.port = optAlg->portVec[1];
        }

        srvSendBuf(otOptSnd, buf, len);

        return rlt;
    }

    int64_t srvRcvBuf(OptAlg *optAlg, uint64_t **buf, uint64_t *len)
    {
        int64_t rlt = -1;
        if (nullptr == optAlg)
        {
            LOG_ERROR("cliRcvBuf input optAlg is null");
            return rlt;
        }
        if (nullptr == buf)
        {
            LOG_ERROR("cliRcvBuf input buf is null");
            return rlt;
        }
        if (nullptr == len)
        {
            LOG_ERROR("cliRcvBuf input len is null");
            return rlt;
        }

        bool is_server = (0 == optAlg->role) ? true : false;

        NcoOtOpt otOpt;
        NcoOtOpt otOptRcv;
        NcoOtOpt otOptSnd;
        if (is_server)
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for srv rcv only
            otOptRcv.ip = optAlg->ipVec[1];
            otOptRcv.port = optAlg->portVec[1];
        }
        else
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for client send only
            otOptSnd.ip = optAlg->ipVec[1];
            otOptSnd.port = optAlg->portVec[1];
        }

        clientRcvBuf(otOptRcv, buf, len);

        return rlt;
    }

    int64_t cliRcvBuf(OptAlg *optAlg, uint64_t **buf, uint64_t *len)
    {
        int64_t rlt = -1;
        if (nullptr == optAlg)
        {
            LOG_ERROR("cliRcvBuf input optAlg is null");
            return rlt;
        }
        if (nullptr == buf)
        {
            LOG_ERROR("cliRcvBuf input buf is null");
            return rlt;
        }
        if (nullptr == len)
        {
            LOG_ERROR("cliRcvBuf input len is null");
            return rlt;
        }

        bool is_server = (0 == optAlg->role) ? true : false;

        NcoOtOpt otOpt;
        NcoOtOpt otOptRcv;
        NcoOtOpt otOptSnd;
        if (is_server)
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for srv rcv only
            otOptRcv.ip = optAlg->ipVec[1];
            otOptRcv.port = optAlg->portVec[1];
        }
        else
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for client send only
            otOptSnd.ip = optAlg->ipVec[1];
            otOptSnd.port = optAlg->portVec[1];
        }

        clientRcvBuf(otOpt, buf, len);

        return rlt;
    }

    int64_t srvSendBuf(OptAlg *optAlg, uint64_t *buf, uint64_t len)
    {
        int64_t rlt = -1;

        if (nullptr == optAlg)
        {
            LOG_ERROR("srvSendBuf input optAlg is null");
            return rlt;
        }
        if (nullptr == buf)
        {
            LOG_ERROR("srvSendBuf input buf is null");
            return rlt;
        }
        if (len < 1)
        {
            LOG_ERROR("srvSendBuf input len is 0");
            return rlt;
        }

        bool is_server = (0 == optAlg->role) ? true : false;

        NcoOtOpt otOpt;
        NcoOtOpt otOptRcv;
        NcoOtOpt otOptSnd;
        if (is_server)
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for srv rcv only
            otOptRcv.ip = optAlg->ipVec[1];
            otOptRcv.port = optAlg->portVec[1];
        }
        else
        {
            otOpt.ip = optAlg->ipVec[0];
            otOpt.port = optAlg->portVec[0];
            // otOpt.rmtIp = optAlg->ipVec[1];
            // otOpt.rmtPort = optAlg->portVec[1];

            //for client send only
            otOptSnd.ip = optAlg->ipVec[1];
            otOptSnd.port = optAlg->portVec[1];
        }

        srvSendBuf(otOpt, buf, len);

        return rlt;
    }

    int64_t rcvChTcpVecSync(std::string ip, int port, std::vector<uint64_t> &rcvVec, std::vector<uint64_t> &sendVec, std::string chName)
    {
        int64_t rlt = -1;
        std::string addr = ip + ":" + std::to_string(port);

        uint64_t sendLen = sendVec.size();
        uint64_t *sendBuf = nullptr;

        if (sendLen > 0)
            sendBuf = (uint64_t *)&sendVec[0];

        uint64_t *rcvBuf = nullptr;

        LOG_INFO("rcvChTcpVecSync addr=" << addr << ",chname=" << chName);

        auto rcvRLt = chRecvBufWithSend(addr, &rcvBuf, 0, sendBuf, sendLen, chName);
        if (rcvRLt > 0 && rcvBuf != nullptr)
        {
            rcvVec.resize(rcvRLt);
            for (int64_t i = 0; i < rcvRLt; i++)
            {
                rcvVec[i] = rcvBuf[i];
            }
            LOG_INFO("rcvChTcpVecSync rcv len=" << rcvRLt);
            free(rcvBuf);

            rlt = rcvRLt;
        }

        return rlt;
    }

    int64_t sendChTcpVecSync(std::string ip, int port, std::vector<uint64_t> &rcvVec, std::vector<uint64_t> &sendVec, std::string chName)
    {
        int64_t rlt = -1;
        std::string addr = ip + ":" + std::to_string(port);

        uint64_t sendLen = sendVec.size();
        uint64_t *sendBuf = nullptr;

        if (sendLen > 0)
            sendBuf = (uint64_t *)&sendVec[0];

        uint64_t *rcvBuf = nullptr;

        LOG_INFO("sendChTcpVecSync sddr=" << addr << ",chname=" << chName);

        auto rcvRLt = chSendBufWithRcv(addr, sendBuf, sendLen, &rcvBuf, 0, chName);
        if (rcvRLt > 0 && rcvBuf != nullptr)
        {
            rcvVec.resize(rcvRLt);
            for (int64_t i = 0; i < rcvRLt; i++)
            {
                rcvVec[i] = rcvBuf[i];
            }
            LOG_INFO("rcvChTcpVecSync rcv len=" << rcvRLt);
            rlt = rcvRLt;
            free(rcvBuf);
        }

        return rlt;
    }

    void setVecPermutation(std::vector<int32_t> &vec, int num)
    {
        if (num < 1)
        {
            std::cout << "setVecPermutation input num error=" << num << std::endl;
        }

        vec.resize(num, -1);

        for (int64_t i = 0; i < num; i++)
        {
            vec.at(i) = i;
        }

        int loop = 3;
        for (int64_t i = 0; i < loop; i++)
        {
            std::random_shuffle(vec.begin(), vec.end());
        }

        return;
    }

    // for float buf send  .Modified by wumingzi. 2023:02:16,Thursday,13:32:48.
    int64_t rcvChTcpVecFloat(std::string ip, int port, std::vector<float> &rcvVec, std::vector<float> &sendVec, std::string chName)
    {
        int64_t rlt = -1;
        std::string addr = ip + ":" + std::to_string(port);

        uint64_t sendLen = sendVec.size();
        float *sendBuf = nullptr;
        int32_t dataTypeLen = sizeof(float) / sizeof(uint8_t);

        if (sendLen > 0)
            sendBuf = (float *)&sendVec[0];

        uint8_t *rcvBuf = nullptr;
        float *rltBuf = nullptr;

        std::cout << "rcvChTcpVecFloat addr=" << addr << ",chname=" << chName << ",dataTypeLen=" << dataTypeLen << std::endl;

        auto rcvRLt = chRecvCharBufWithSend(addr, &rcvBuf, 0, (uint8_t *)sendBuf, sendLen * dataTypeLen, chName);
        int64_t dataNum = rcvRLt / dataTypeLen;
        if (rcvRLt > 0 && rcvBuf != nullptr)
        {
            rcvVec.resize(rcvRLt / dataTypeLen);
            rltBuf = (float *)rcvBuf;
            for (int64_t i = 0; i < dataNum; i++)
            {
                rcvVec[i] = rltBuf[i];
            }
            std::cout << "rcvChTcpVecFloat rcv len=" << dataNum << std::endl;
            free(rcvBuf);

            rlt = dataNum;
        }

        return rlt;
    }

    int64_t sendChTcpVecFloat(std::string ip, int port, std::vector<float> &rcvVec, std::vector<float> &sendVec, std::string chName)
    {
        int64_t rlt = -1;
        std::string addr = ip + ":" + std::to_string(port);

        uint64_t sendLen = sendVec.size();
        float *sendBuf = nullptr;

        if (sendLen > 0)
            sendBuf = (float *)&sendVec[0];

        uint8_t *rcvBuf = nullptr;
        float *rltBuf = nullptr;
        int32_t dataTypeLen = sizeof(float) / sizeof(uint8_t);
        std::cout << "sendChTcpVeFloat sddr=" << addr << ",chname=" << chName << ",dataTypeLen=" << dataTypeLen << std::endl;

        auto rcvRLt = chSendCharBufWithRcv(addr, (uint8_t *)sendBuf, sendLen * dataTypeLen, &rcvBuf, 0, chName);
        int64_t dataNum = rcvRLt / dataTypeLen;
        if (rcvRLt > 0 && rcvBuf != nullptr)
        {
            rcvVec.resize(dataNum);
            rltBuf = (float *)rcvBuf;
            for (int64_t i = 0; i < dataNum; i++)
            {
                rcvVec[i] = rltBuf[i];
            }
            std::cout << "rcvChTcpVecSync rcv len=" << dataNum << std::endl;
            rlt = dataNum;
            free(rcvBuf);
        }

        return rlt;
    }

    //for ch uint8_t buf rcv&send
    int64_t chRecvCharBufWithSend(std::string ip, uint8_t **rcvBuf, uint64_t rcvLen, uint8_t *sendBuf, uint64_t sendLen, std::string chName)
    {

        int64_t rlt = -1;

        if (ip.length() < 10)
            return rlt;

        IOService ios;
        Endpoint ep(ios, ip, EpMode::Server, "tcpSendCharBufRcv" + chName);
        // Endpoint ep(ios, ip, EpMode::Server, "tcpSendRcv");
        Channel ch = ep.addChannel();

        uint64_t rcvLenChar = 0;
        //first send buf len to rcv
        ch.recv(rcvLenChar);

        // std::cout << "chRecvBufWithSend rcvLen=" << rcvLenChar << std::endl;

        if (rcvLenChar > 0)
        {

            uint8_t *databuf = (uint8_t *)calloc(rcvLenChar / sizeof(uint8_t), sizeof(uint8_t));
            if (databuf)
            {
                // sendLen *= sizeof(uint64_t);
                // std::cout<<"begin recv len="<<rcvLenChar<<std::endl;
                ch.recv((uint8_t *)databuf, rcvLenChar);
                rlt = rcvLenChar;
                *(uint8_t **)rcvBuf = (uint8_t *)databuf;
            }

            rlt = rcvLenChar / (sizeof(uint8_t));
            // for (int64_t i = 0; i < rlt; i++) {
            //    std::cout<<i<<":"<<std::dec<<*(*(uint64_t**)rcvBuf+i)<<std::endl;
            // }
        }

        //begin to send data
        uint64_t sendLenChar = sendLen * sizeof(uint8_t);
        ch.send(sendLenChar);

        if (sendLenChar > 0 && sendBuf != nullptr)
        {
            ch.send((uint8_t *)sendBuf, sendLenChar);
        }

        ch.close();
        ep.stop();
        ios.stop();

        return rlt;
    }

    int64_t chSendCharBufWithRcv(std::string ip, uint8_t *sendBuf, uint64_t sendLen, uint8_t **rcvBuf, uint64_t rcvLenMax, std::string chName)
    {

        int64_t rlt = -1;

        if (sendLen < 1)
            return rlt;

        if (nullptr == sendBuf)
            return rlt;

        if (ip.length() < 10)
            return rlt;

        IOService ios;
        // Endpoint ep(ios, ip, EpMode::Client, "tcpSendRcv");
        Endpoint ep(ios, ip, EpMode::Client, "tcpSendCharBufRcv" + chName);
        Channel ch = ep.addChannel();

        uint64_t sendBufLen = sendLen * sizeof(uint8_t);
        // std::cout<<"chSendCharBufWithRcv sendBufLen="<<sendBufLen<<std::endl;
        //first send buf len to rcv
        ch.send(sendBufLen);
        ch.send((uint8_t *)sendBuf, sendBufLen);

        uint64_t rcvLen = 0;

        //begin to rcv data from other.
        ch.recv(rcvLen);
        rlt = rcvLen;
        // std::cout<<"chSendCharBufWithRcv rcvLen="<<rcvLen<<std::endl;

        if (rcvLen > 0)
        {
            uint8_t *databuf = (uint8_t *)calloc(rcvLen / sizeof(uint8_t), sizeof(uint8_t));
            if (databuf)
            {
                ch.recv((uint8_t *)databuf, rcvLen);
                rlt = rcvLen;
                *(uint8_t **)rcvBuf = (uint8_t *)databuf;
            }

            rlt = rcvLen / (sizeof(uint8_t));
            // for (int64_t i = 0; i < rlt; i++) {
            //    std::cout<<i<<":"<<std::dec<<*(*(uint64_t**)rcvBuf+i)<<std::endl;
            // }
        }

        //release socket.
        ch.close();
        ep.stop();
        ios.stop();

        return rlt;
    }

    int64_t sendChBufSyncTcp(std::string ip, int port, uint8_t *dataBuf, uint64_t sendLen)
    {

        int rcvLen = 0;
        uint8_t *rcvBuf = nullptr;

        std::string addr = ip + ":" + std::to_string(port);
        std::cout << "sendChBufSyncTcp addr=" << addr << std::endl;

        auto rlt = chSendCharBufWithRcv(addr, dataBuf, sendLen, &rcvBuf, rcvLen);
        return rlt;
    }

    int64_t rcvChBufSyncTcp(std::string ip, int port, uint8_t **dataBuf, uint64_t rcvLen)
    {

        uint8_t sendBuf[1];
        sendBuf[0] = 0;
        uint64_t sendLen = 1;

        std::string addr = ip + ":" + std::to_string(port);

        std::cout << "rcvChBufSyncTcp addr=" << addr << std::endl;
        auto rlt = chRecvCharBufWithSend(addr, dataBuf, rcvLen, sendBuf, 0);

        return rlt;
    }

    /////////// ch tcp function
    int64_t chRecvBufWithSend(std::string ip, uint64_t **rcvBuf, uint64_t rcvLen, uint64_t *sendBuf, uint64_t sendLen, std::string chName)
    {

        int64_t rlt = -1;

        if (ip.length() < 10)
            return rlt;

        IOService ios;
        Endpoint ep(ios, ip, EpMode::Server, "chRcv" + chName);
        LOG_INFO("chRecvBufWithSend, server mode.ip=" << ip << ",chname="
                                                      << "chRcv" + chName);

        Channel ch = ep.addChannel();
        LOG_INFO("add channel ok");
        uint64_t rcvLenChar = 0;
        //first send buf len to rcv
        ch.recv(rcvLenChar);
        LOG_INFO("chRecvBufWithSend rcvLen=" << rcvLenChar);

        if (rcvLenChar > 0)
        {
            uint64_t *databuf = (uint64_t *)calloc(rcvLenChar / sizeof(uint64_t), sizeof(uint64_t));
            if (databuf)
            {
                ch.recv((uint8_t *)databuf, rcvLenChar);
                rlt = rcvLenChar;
                *(uint64_t **)rcvBuf = (uint64_t *)databuf;
            }

            rlt = rcvLenChar / (sizeof(uint64_t));
        }

        //begin to send data
        uint64_t sendLenChar = sendLen * sizeof(uint64_t);
        ch.send(sendLenChar);

        if (sendLenChar > 0 && sendBuf != nullptr)
        {
            ch.send((uint8_t *)sendBuf, sendLenChar);
        }

        ch.close();
        ep.stop();
        ios.stop();

        return rlt;
    }

    int64_t chSendBufWithRcv(std::string ip, uint64_t *sendBuf, uint64_t sendLen, uint64_t **rcvBuf, uint64_t rcvLenMax, std::string chName)
    {
        int64_t rlt = -1;

        if (sendLen < 1)
            return rlt;

        if (nullptr == sendBuf)
            return rlt;

        if (ip.length() < 10)
            return rlt;

        IOService ios;
        Endpoint ep(ios, ip, EpMode::Client, "chRcv" + chName);
        LOG_INFO("chSendBufWithRcv, client mode.ip=" << ip << ",chname="
                                                     << "chRcv" + chName);

        Channel ch = ep.addChannel();
        LOG_INFO("add channel ok");

        uint64_t sendBufLen = sendLen * sizeof(uint64_t);
        //first send buf len to rcv
        ch.send(sendBufLen);
        LOG_INFO("sendBufLen ok,sendBufLen=" << sendBufLen);
        ch.send((uint8_t *)sendBuf, sendBufLen);
        LOG_INFO("send buf ok");
        uint64_t rcvLen = 0;

        //begin to rcv data from other.
        ch.recv(rcvLen);
        LOG_INFO("chSendBufWithRcv rcvLen=" << rcvLen);

        if (rcvLen > 0)
        {
            uint64_t *databuf = (uint64_t *)calloc(rcvLen / sizeof(uint64_t), sizeof(uint64_t));
            if (databuf)
            {
                ch.recv((uint8_t *)databuf, rcvLen);
                rlt = rcvLen;
                *(uint64_t **)rcvBuf = (uint64_t *)databuf;
            }

            rlt = rcvLen / (sizeof(uint64_t));
        }

        //release socket.
        ch.close();
        ep.stop();
        ios.stop();

        return rlt;
    }

    //aes/md5 tool
    int getMd5(unsigned char *input, uint32_t len, unsigned char *output)
    {
        MD5_CTX x;
        MD5_Init(&x);
        MD5_Update(&x, (char *)input, len);
        MD5_Final(output, &x);
        return 0;
    }
    // end of hash functions

    void showHexValue(uint32_t *buf, uint64_t dataLen, uint64_t len)
    {
        for (uint64_t i = 0; i < len; i++)
        {
            std::stringstream ss;
            ss << std::dec << "[" << i << "]:";
            uint32_t *base = buf + i * dataLen;
            for (uint64_t j = 0; j < dataLen; j++)
            {
                ss << std::hex << *(uint32_t *)(base + dataLen - 1 - j) << "-";
            }
            LOG_DEBUG(ss.str());
        }
    }

    //strVec: input:  string vector
    //seedVec: input. seed for random key
    //strLen:  input. maxmum string length. force each string the same length so as to keep all ciphter have the same size.
    //dataBuf: output. store the ciphertext string pointer.
    //keyBuf: input. store the encode key.
    //indexVec: input, encrypte the elements in strVec at the order of index given by indexVec
    int64_t encStr2BufIndex(std::vector<std::string> *strVec, int64_t strLen, std::vector<uint64_t> seed, uint64_t **dataBuf, uint64_t *keyBuf, std::vector<std::int64_t> *indexVec)
    {
        int64_t rlt = -1;
        int64_t minLen = 1;
        uint64_t maxShowCnt = 3;

        if (nullptr == strVec)
        {
            LOG_ERROR("encStrVec2Buf strVec is null. exit");
            return rlt;
        }

        if (nullptr == indexVec)
        {
            LOG_ERROR("encStrVec2Buf indexVec is null. exit");
            return rlt;
        }

        if (nullptr == dataBuf)
        {
            LOG_ERROR("encStrVec2Buf dataBuf is null. exit");
            return rlt;
        }

        if (nullptr == keyBuf)
        {
            LOG_ERROR("encStrVec2Buf keyBuf is null. exit");
            return rlt;
        }

        if (strLen < minLen)
        {
            LOG_ERROR("encStrVec2Buf strLen is too small=" << strLen << ", exit");
            return rlt;
        }

        uint64_t strNum = strVec->size();
        uint64_t aesLen = (strLen + 15) / 16;
        uint64_t strEncLen = 2 * aesLen; //single string enc length in uint64 size.

        uint64_t strEncLenTotal = strEncLen * strNum;

        LOG_INFO("encStrVec2Buf. strNum=" << strNum << ",aesLen=" << aesLen << ",strEncLen=" << strEncLen << ",strEncLenTotal=" << strEncLenTotal);

        uint64_t indexLen = indexVec->size();
        uint64_t index = 0;
        if (strNum != indexLen)
        {
            LOG_ERROR("encStrVec2Buf indexVec size is error =" << indexLen);
            return rlt;
        }

        uint64_t *encBuf = (uint64_t *)calloc(strEncLenTotal, sizeof(uint64_t));

        if (nullptr == encBuf)
        {
            LOG_ERROR("encStrVec2Buf encBuf is null. exit");
            return rlt;
        }

        uint64_t *aesKeyBuf = keyBuf;
        int64_t base = 0;

        //for debug
        maxShowCnt = 3;
        // for data security,disable printing input keys  .Modified by wumingzi. 2023:04:06,Thursday,21:39:57.
        // for (uint64_t i = 0; i < strNum && i < maxShowCnt; i++)
        // {
        //     base = i * 2;
        //     LOG_DEBUG("key[" << i << "]=" << std::hex << aesKeyBuf[base] << "-" << aesKeyBuf[base + 1]);
        // }

        //here call aes to encrypt str
        uint64_t *curAesKey = nullptr;
        uint64_t *curEncBuf = nullptr;
        uint64_t strBuf[strEncLen];
        unsigned char *charBuf = nullptr;

        int64_t keyBase = 0;
        int64_t encBufBase = 0;
        for (uint64_t i = 0; i < strNum; i++)
        {
            keyBase = i * 2;
            encBufBase = i * strEncLen;
            curAesKey = &aesKeyBuf[keyBase];
            curEncBuf = &encBuf[encBufBase];

            //here begin to aes encryption.
            index = indexVec->at(i);
            if (index >= strNum)
            {
                LOG_INFO("encStrVec2BufIndex, enc loop index is error=" << index);
                continue;
            }
            convStr2Buf(strVec->at(index), strBuf, strEncLen * 8);

            //show str buf.
            charBuf = (unsigned char *)strBuf;
        // for data security,disable printing input data  .Modified by wumingzi. 2023:04:06,Thursday,21:39:57.
            // if (i < maxShowCnt)
            // {
            //     std::stringstream ss;
            //     for (uint64_t k = 0; k < strEncLen * 8; k++)
            //     {
            //         ss << (unsigned char)charBuf[k];
            //     }
            //     LOG_DEBUG(ss.str());
            // }

            auto encRlt = aesEncBUf(strBuf, curEncBuf, strEncLen, curAesKey);

            if (encRlt < 0)
            {
                LOG_INFO("aes enc error at str=" << i);
            }
        }

        //set data output.
        *dataBuf = (uint64_t *)encBuf;
        // *keyBuf = (uint64_t *)aesKeyBuf;

        rlt = 0;
        return rlt;
    }

    //strVec: input:  string vector
    //seedVec: input. seed for random key
    //strLen:  input. maxmum string length. force each string the same length so as to keep all ciphter have the same size.
    //dataBuf: output. store the ciphertext string pointer.
    //keyBuf: ouput. store the random key.
    //indexVec: input, encrypte the elements in strVec at the order of index given by indexVec
    int64_t encStr2BufIndex(std::vector<std::string> *strVec, int64_t strLen, std::vector<uint64_t> seed, uint64_t **dataBuf, uint64_t **keyBuf, std::vector<std::int64_t> *indexVec)
    {
        int64_t rlt = -1;
        int64_t minLen = 1;
        uint64_t maxShowCnt = 3;

        if (nullptr == strVec)
        {
            LOG_ERROR("encStrVec2Buf strVec is null. exit");
            return rlt;
        }

        if (nullptr == indexVec)
        {
            LOG_ERROR("encStrVec2Buf indexVec is null. exit");
            return rlt;
        }

        if (nullptr == dataBuf)
        {
            LOG_ERROR("encStrVec2Buf dataBuf is null. exit");
            return rlt;
        }

        if (nullptr == keyBuf)
        {
            LOG_ERROR("encStrVec2Buf keyBuf is null. exit");
            return rlt;
        }

        if (strLen < minLen)
        {
            LOG_ERROR("encStrVec2Buf strLen is too small=" << strLen << ", exit");
            return rlt;
        }

        uint64_t strNum = strVec->size();
        uint64_t aesLen = (strLen + 15) / 16;
        uint64_t strEncLen = 2 * aesLen; //single string enc length in uint64 size.

        uint64_t strEncLenTotal = strEncLen * strNum;

        LOG_INFO("encStrVec2Buf. strNum=" << strNum << ",aesLen=" << aesLen << ",strEncLen=" << strEncLen << ",strEncLenTotal=" << strEncLenTotal);

        uint64_t indexLen = indexVec->size();
        uint64_t index = 0;
        if (strNum != indexLen)
        {
            LOG_ERROR("encStrVec2Buf indexVec size is error =" << indexLen);
            return rlt;
        }

        uint64_t *encBuf = (uint64_t *)calloc(strEncLenTotal, sizeof(uint64_t));

        if (nullptr == encBuf)
        {
            LOG_ERROR("encStrVec2Buf encBuf is null. exit");
            return rlt;
        }

        uint64_t aesKeyBufLen = strNum * 2;
        uint64_t *aesKeyBuf = (uint64_t *)calloc(aesKeyBufLen, sizeof(uint64_t));

        if (nullptr == aesKeyBuf)
        {
            LOG_ERROR("encStrVec2Buf aesKeyBuf is null. exit");
            return rlt;
        }

        int64_t base = 0;

        //first generate random aeskey.
        uint64_t seed1 = 0x1133557799AACCEE;
        uint64_t seed2 = 0x2244668800BBDDFF;

        oc::block seedBlk = oc::sysRandomSeed();
        if (seed.size() >= 2)
        {
            seed1 = seed.at(0);
            seed2 = seed.at(1);
        }
        else if (1 == seed.size())
        {
            seed1 = seed.at(0);
            seed2 = seedBlk.as<std::uint64_t>()[0];
        }
        if (0 == seed.size())
        {
            seed1 = seedBlk.as<std::uint64_t>()[0];
            seed2 = seedBlk.as<std::uint64_t>()[1];
        }

        PRNG prng0(oc::toBlock(seed1, seed2));
        prng0.get(aesKeyBuf, aesKeyBufLen);

        //for debug
        maxShowCnt = strNum;
        for (uint64_t i = 0; i < strNum && i < maxShowCnt; i++)
        {
            base = i * 2;
            LOG_DEBUG("key[" << i << "]=" << std::hex << aesKeyBuf[base] << "-" << aesKeyBuf[base + 1]);
        }

        //here call aes to encrypt str
        uint64_t *curAesKey = nullptr;
        uint64_t *curEncBuf = nullptr;
        uint64_t strBuf[strEncLen];
        unsigned char *charBuf = nullptr;

        int64_t keyBase = 0;
        int64_t encBufBase = 0;
        for (uint64_t i = 0; i < strNum; i++)
        {
            keyBase = i * 2;
            encBufBase = i * strEncLen;
            curAesKey = &aesKeyBuf[keyBase];
            curEncBuf = &encBuf[encBufBase];

            //here begin to aes encryption.
            index = indexVec->at(i);
            if (index >= strNum)
            {
                LOG_INFO("encStrVec2BufIndex, enc loop index is error=" << index);
                continue;
            }
            convStr2Buf(strVec->at(index), strBuf, strEncLen * 8);

            //show str buf.
            charBuf = (unsigned char *)strBuf;

            if (i < maxShowCnt)
            {
                std::stringstream ss;
                for (uint64_t k = 0; k < strEncLen * 8; k++)
                {
                    ss << (unsigned char)charBuf[k];
                }
                LOG_DEBUG(ss.str());
            }

            auto encRlt = aesEncBUf(strBuf, curEncBuf, strEncLen, curAesKey);

            if (encRlt < 0)
            {
                LOG_INFO("aes enc error at str=" << i);
            }
        }

        //set data output.
        *dataBuf = (uint64_t *)encBuf;
        *keyBuf = (uint64_t *)aesKeyBuf;

        rlt = 0;
        return rlt;
    }

    //strVec: input:  string vector
    //seedVec: input. seed for random key
    //strLen:  input. maxmum string length. force each string the same length so as to keep all ciphter have the same size.
    //dataBuf: output. store the ciphertext string pointer.
    //keyBuf: ouput. store the random key.
    //indexVec: input, encrypte the elements in strVec at the order of index given by indexVec
    int64_t encStrVec2BufIndex(std::vector<std::string> &strVec, int64_t strLen, std::vector<uint64_t> seed, uint64_t **dataBuf, uint64_t **keyBuf, std::vector<std::int64_t> &indexVec)
    {
        int64_t rlt = -1;
        int64_t minLen = 1;
        uint64_t maxShowCnt = 10;

        if (nullptr == dataBuf)
        {
            LOG_ERROR("encStrVec2Buf dataBuf is null. exit");
            return rlt;
        }

        if (nullptr == keyBuf)
        {
            LOG_ERROR("encStrVec2Buf keyBuf is null. exit");
            return rlt;
        }

        if (strLen < minLen)
        {
            LOG_ERROR("encStrVec2Buf strLen is too small=" << strLen << ", exit");
            return rlt;
        }

        uint64_t strNum = strVec.size();
        uint64_t aesLen = (strLen + 15) / 16;
        uint64_t strEncLen = 2 * aesLen; //single string enc length in uint64 size.

        uint64_t strEncLenTotal = strEncLen * strNum;

        LOG_INFO("encStrVec2Buf. strNum=" << strNum << ",aesLen=" << aesLen << ",strEncLen=" << strEncLen << ",strEncLenTotal=" << strEncLenTotal);

        uint64_t indexLen = indexVec.size();
        uint64_t index = 0;
        if (strNum != indexLen)
        {
            LOG_ERROR("encStrVec2Buf indexVec size is error =" << indexLen);
            return rlt;
        }

        uint64_t *encBuf = (uint64_t *)calloc(strEncLenTotal, sizeof(uint64_t));

        if (nullptr == encBuf)
        {
            LOG_ERROR("encStrVec2Buf encBuf is null. exit");
            return rlt;
        }

        uint64_t aesKeyBufLen = strNum * 2;
        uint64_t *aesKeyBuf = (uint64_t *)calloc(aesKeyBufLen, sizeof(uint64_t));

        if (nullptr == aesKeyBuf)
        {
            LOG_ERROR("encStrVec2Buf aesKeyBuf is null. exit");
            return rlt;
        }

        int64_t base = 0;

        //first generate random aeskey.
        uint64_t seed1 = 0x1133557799AACCEE;
        uint64_t seed2 = 0x2244668800BBDDFF;

        oc::block seedBlk = oc::sysRandomSeed();
        if (seed.size() >= 2)
        {
            seed1 = seed.at(0);
            seed2 = seed.at(1);
        }
        else if (1 == seed.size())
        {
            seed1 = seed.at(0);
            seed2 = seedBlk.as<std::uint64_t>()[0];
        }
        if (0 == seed.size())
        {
            seed1 = seedBlk.as<std::uint64_t>()[0];
            seed2 = seedBlk.as<std::uint64_t>()[1];
        }

        PRNG prng0(oc::toBlock(seed1, seed2));
        prng0.get(aesKeyBuf, aesKeyBufLen);

        for (uint64_t i = 0; i < strNum && i < maxShowCnt; i++)
        {
            base = i * 2;
            LOG_DEBUG("key[" << i << "]=" << std::hex << aesKeyBuf[base] << "-" << aesKeyBuf[base + 1]);
        }

        //here call aes to encrypt str
        uint64_t *curAesKey = nullptr;
        uint64_t *curEncBuf = nullptr;
        uint64_t strBuf[strEncLen];
        unsigned char *charBuf = nullptr;

        int64_t keyBase = 0;
        int64_t encBufBase = 0;
        for (uint64_t i = 0; i < strNum; i++)
        {
            keyBase = i * 2;
            encBufBase = i * strEncLen;
            curAesKey = &aesKeyBuf[keyBase];
            curEncBuf = &encBuf[encBufBase];

            //here begin to aes encryption.
            index = indexVec.at(i);
            if (index >= strNum)
            {
                LOG_INFO("encStrVec2BufIndex, enc loop index is error=" << index);
                continue;
            }
            convStr2Buf(strVec.at(index), strBuf, strEncLen * 8);

            //show str buf.
            charBuf = (unsigned char *)strBuf;

            if (i < maxShowCnt)
            {
                std::stringstream ss;
                for (uint64_t k = 0; k < strEncLen * 8; k++)
                {
                    ss << (unsigned char)charBuf[k];
                }
                LOG_DEBUG(ss.str());
            }

            auto encRlt = aesEncBUf(strBuf, curEncBuf, strEncLen, curAesKey);

            if (encRlt < 0)
            {
                LOG_INFO("aes enc error at str=" << i);
            }
        }

        //set data output.
        *dataBuf = (uint64_t *)encBuf;
        *keyBuf = (uint64_t *)aesKeyBuf;

        rlt = 0;
        return rlt;
    }

    //src: input data buffer pointer(plaintext data)
    //dst: output data buffer pointer(cipher data)
    //len: input data len.
    //keyBuf: input aes key.
    int64_t aesEncBUf(uint64_t *src, uint64_t *dst, int64_t len, uint64_t *key)
    {
        int64_t rlt = -1;

        //first generate block to save data.
        int64_t blkNum = len / 2;

        if (nullptr == src || nullptr == dst || nullptr == key)
        {
            LOG_ERROR("input buf pointer is null.");
            return rlt;
        }

        block commonKey;
        AES commonAes;
        block *aesInput = new block[blkNum];
        block *aesOutput = new block[blkNum];
        commonKey = toBlock(key[0], key[1]);
        commonAes.setKey(commonKey);
        int64_t base = 0;
        for (auto i = 0; i < blkNum; ++i)
        {

            base = i * 2;
            aesInput[i] = toBlock(src[base + 1], src[base]);
        }

        commonAes.ecbEncBlocks(aesInput, blkNum, aesOutput);

        for (auto i = 0; i < blkNum; ++i)
        {
            base = i * 2;
            dst[base] = aesOutput[i].as<std::uint64_t>()[0];
            dst[base + 1] = aesOutput[i].as<std::uint64_t>()[1];
        }

        delete[] aesInput;
        delete[] aesOutput;

        rlt = 0;
        return rlt;
    }

    uint64_t getRand(uint64_t max)
    {
        uint64_t modNum;
        modNum = max;
        if (modNum < 1)
            modNum = 1;
        std::srand((unsigned)time(NULL));
        return std::rand() % modNum;
    }

    //src: input data buffer pointer. (cipher data)
    //dst: output data buffer pointer. (plaintext data)
    //len: input data len.
    //keyBuf: input aes key.
    int64_t aesDecBUf(uint64_t *src, uint64_t *dst, int64_t len, uint64_t *key)
    {
        int64_t rlt = -1;

        //first generate block to save data.
        int64_t blkNum = len / 2;

        if (nullptr == src || nullptr == dst || nullptr == key)
        {
            LOG_ERROR("input buf pointer is null.");
            return rlt;
        }

        block commonKey;
        AESDec commonAes;

        block *aesInput = (block *)src;
        block *aesOutput = (block *)dst;
        commonKey = toBlock(key[0], key[1]);
        commonAes.setKey(commonKey);

        for (auto i = 0; i < blkNum; ++i)
        {
            commonAes.ecbDecBlock(aesInput[i], aesOutput[i]);
        }

        rlt = 0;
        return rlt;
    }

    int64_t convStr2Buf(std::string &str, uint64_t *buf, int64_t len)
    {
        int64_t rlt = -1;
        char blk = ' ';
        int64_t strLen = 0;
        uint8_t *charBuf;
        if (nullptr == buf)
        {
            LOG_ERROR("input buf is null.exit");
            return rlt;
        }

        charBuf = (uint8_t *)buf;

        strLen = str.length();

        for (int64_t i = 0; i < len; i++)
        {
            if (i < strLen)
            {
                charBuf[i] = str[i];
            }
            else
            {
                charBuf[i] = blk;
            }
        }
        return 0;
    }

    /**
     *
     * strVec:  input string vector
     * buf:     output hash result
     * headline:input, indicates headline number which are not used to compute hash value.
     *
     */
    int64_t convertStrVec2Md5Index(std::vector<std::string> &strVec, uint32_t *&buf, std::vector<std::int64_t> &indexVec)
    {
        int64_t rlt = 0;
        freeUInt32Vec(buf);
        size_t num = strVec.size();
        if (num != indexVec.size())
        {
            LOG_ERROR("convertStrVec2Md5 input dat num is invalid at=" << num << ",index vec size=" << indexVec.size());
            return ERROR_RLT;
        }

        uint32_t dwLen = 16 / sizeof(uint32_t);
        unsigned char *outBuf = nullptr;
        uint32_t *base = nullptr;
        size_t index = 0;

        buf = allocateUInt32Vec(num * 4);
        PTR_ERR_RTN(buf);

        for (size_t i = 0; i < num; i++)
        {
            base = buf + (i)*dwLen;
            index = indexVec.at(i);
            if (index >= num)
            {
                LOG_INFO("index is error=" << index << ",vec size=" << num);
                continue;
            }
            outBuf = (unsigned char *)base;
            std::string str = strVec.at(index);
            getMd5((unsigned char *)strVec.at(index).c_str(), strVec.at(index).length(), outBuf);
            rlt++;
        }

        return rlt;
    }

    // time tool  .Modified by wumingzi/wumingzi. 2022:05:17,Tuesday,15:12:54.
    TimeUtils::TimeUtils()
    {
        timeElapsed = 0;
    }

    void TimeUtils::clear()
    {
        timeElapsed = 0;
        gettimeofday(&startTime, 0);
    }

    float TimeUtils::getTime()
    {
        gettimeofday(&stopTime, 0);
        timeElapsed = (stopTime.tv_sec - startTime.tv_sec) * 1000.0;
        timeElapsed += (stopTime.tv_usec - startTime.tv_usec) / 1000.0;
        return (float)timeElapsed / 1000.0;
    }

    void TimeUtils::start(std::string msg)
    {
        LOG_INFO("............................................................................");
        LOG_INFO("Start " + msg);
        gettimeofday(&startTime, 0);
    }

    void TimeUtils::start(std::string msg, int num)
    {
        LOG_INFO("\n............................................................................");
        LOG_INFO("Start " + msg << ":" << num);
        gettimeofday(&startTime, 0);
    }

    std::string TimeUtils::strStart(std::string msg)
    {
        std::stringstream log;
        log << "\n............................................................................" << std::endl;
        log << "Start " + msg << ":" << std::endl;
        gettimeofday(&startTime, 0);
        return log.str();
    }

    void TimeUtils::stopMs(std::string msg)
    {
        gettimeofday(&stopTime, 0);
        timeElapsed = (stopTime.tv_sec - startTime.tv_sec) * 1000.0;
        timeElapsed += (stopTime.tv_usec - startTime.tv_usec) / 1000.0;
        LOG_INFO(msg + " time = " << timeElapsed << " ms");
        LOG_INFO("............................................................................");
    }
    std::string TimeUtils::strStopMs(std::string msg)
    {
        std::stringstream log;
        gettimeofday(&stopTime, 0);
        timeElapsed = (stopTime.tv_sec - startTime.tv_sec) * 1000.0;
        timeElapsed += (stopTime.tv_usec - startTime.tv_usec) / 1000.0;
        log << msg + " time = " << timeElapsed << " ms" << std::endl;
        log << "............................................................................" << std::endl;

        return log.str();
    }

    void TimeUtils::stopS(std::string msg)
    {
        gettimeofday(&stopTime, 0);
        timeElapsed = (stopTime.tv_sec - startTime.tv_sec) * 1000.0;
        timeElapsed += (stopTime.tv_usec - startTime.tv_usec) / 1000.0;
        LOG_INFO(msg + " time = " << timeElapsed / 1000.0 << " s");
        LOG_INFO("............................................................................");
    }

    std::string TimeUtils::strStopS(std::string msg)
    {
        std::stringstream log;
        gettimeofday(&stopTime, 0);
        timeElapsed = (stopTime.tv_sec - startTime.tv_sec) * 1000.0;
        timeElapsed += (stopTime.tv_usec - startTime.tv_usec) / 1000.0;
        log << msg + " time = " << timeElapsed / 1000.0 << " s" << std::endl;
        log << "............................................................................" << std::endl;

        return log.str();
    }

    void TimeUtils::stopS(std::string msg, int num)
    {
        gettimeofday(&stopTime, 0);
        timeElapsed = (stopTime.tv_sec - startTime.tv_sec) * 1000.0;
        timeElapsed += (stopTime.tv_usec - startTime.tv_usec) / 1000.0;
        LOG_INFO(msg << ":" << num << ", time = " << timeElapsed / 1000.0 << " s");
        LOG_INFO("............................................................................");
    }
}
