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
 * @file psi.cpp
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
 * @brief
 * @version
 * @date 2022-05-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "psi.h"

#include <string>

#include "oprf_psi/oprf_psi_client.h"
#include "oprf_psi/oprf_psi_server.h"
#include "toolkits/util/include/xlog.h"
#include "common/pub/include/util.h"

namespace xscePsiAlg
{
    using namespace util;
    using namespace xsce_ose;
    int64_t savePsiRltVec(std::vector<uint64_t> &rltVec, std::vector<uint64_t> &indexHash)
    {
        int rlt = 0;
        uint64_t psiNum = rltVec.size();
        LOG_INFO("save psi rlt. psi num=" << psiNum);
        indexHash.resize(0);

        for (uint64_t i = 0; i < psiNum; i++)
        {
            auto index = rltVec.at(i);

            if (index >= 0)
            {
                indexHash.push_back(index);
                rlt++;
            }
        }

        return rlt;
    }

    void SetOprfPsiParams(const OptAlg &opt, oprf_psi::OprfPsi *psi_object)
    {
        psi_object->SetWidth(opt.oprfWidth);
        psi_object->SetLogHeight(opt.oprfLogHeight);
        psi_object->SetHashLengthInBytes(opt.oprfHashLenInBytes);
        psi_object->SetBucket(opt.oprfBucket1);
        psi_object->SetHashLen(opt.hashLen);
        psi_object->SetCommonSeed(opt.commonSeed, opt.commonSeed1);
        psi_object->SetInertalSeed(opt.inertalSeed, opt.inertalSeed1);
    }

    int64_t oprfPsiAlgClient(OptAlg *optAlg, std::vector<uint64_t> &srvIndexVec, bool get_oprf_values, std::vector<block> &oprf_values)
    {
        int64_t rlt = 0;
        LOG_INFO(optAlg->logger, "oprf psi alg client mode begin...");

        uint8_t *hashBuf = optAlg->hashBuf;
        PTR_ERR_RTN(hashBuf);

        LOG_INFO(optAlg->logger, "local role=" << optAlg->role << ",ip=" << optAlg->addr << ":" << optAlg->port);

        auto senderSize = optAlg->neles;
        auto receiverSize = optAlg->rmtNeles;
        if (CLIENT_C == optAlg->role)
        {
            senderSize = optAlg->rmtNeles;
            receiverSize = optAlg->neles;
        }
        std::string ip = optAlg->addr;
        int port = optAlg->port;

        uint32_t rptTime = 1;

        LOG_INFO(optAlg->logger, "opt role=" << optAlg->role << ",rpt time=" << rptTime);
        for (uint32_t i = 0; i < rptTime; i++)
        {
            if (SERVER_C == optAlg->role)
            {
                LOG_INFO(optAlg->logger, "begin run oprf sender..");

                oprf_psi::OprfPsiServer psi_server(ip, port);
                // to init common seed & internal seed.
                SetOprfPsiParams(*optAlg, &psi_server);

                psi_server.SetOprfValuesFlag(get_oprf_values);
                // optAlg->task_status.SetProgressPerBucket(25, 0);
                rlt = psi_server.OprfPsiAlg((uint8_t *)hashBuf, senderSize, receiverSize, optAlg);
                if (rlt < 0)
                {
                    LOG_ERROR("execution error in psi_server.OprfPsiAlg function, check the detailed log")
                }
                // optAlg->task_status.SetProgressPerBucket(30, 0);
                oprf_values = psi_server.GetOprfValues();
            }

            if (CLIENT_C == optAlg->role)
            {
                LOG_INFO(optAlg->logger, "begin run oprf receiver..");
                oprf_psi::OprfPsiClient psi_client(ip, port);
                // to init common seed & internal seed.
                SetOprfPsiParams(*optAlg, &psi_client);

                psi_client.SetOprfValuesFlag(get_oprf_values);
                // optAlg->task_status.SetProgressPerBucket(25, 0);
                rlt = psi_client.OprfPsiAlg((uint8_t *)hashBuf, receiverSize, senderSize, optAlg);
                if (rlt < 0)
                {
                    LOG_ERROR("execution error in psi_client.OprfPsiAlg function, check the detailed log")
                }
                // optAlg->task_status.SetProgressPerBucket(30, 0);
                oprf_values = psi_client.GetOprfValues();
                auto rltIndexVec = psi_client.GetPsiResult();

                // here copy rlt vec.
                splitHalfVector(&srvIndexVec, &optAlg->rltVec, &rltIndexVec);

                LOG_INFO(optAlg->logger, "oprf reciver no need to send psi rlt. ");
            }
        }

        return rlt;
    }

    int64_t hashbufPsiAlgClient(uint64_t *hashBufInput, int64_t psiLen, std::vector<uint64_t> &rltVec, OptAlg *optAlg, std::vector<uint64_t> &srvIndexVec, bool get_oprf_values, std::vector<block> &oprf_values, int roleSwitch)
    {
        int64_t rlt = -1;
        LOG_INFO(optAlg->logger, "hashbufPsiAlgClient top level ...");

        int64_t client_neles = 0;
        int64_t neles = 0;
        int64_t rmtNeles = 0;
        bool localClientBin = false;
        bool noChangeServerRole = false; // do not change the

        if (roleSwitch > 0)
        {
            LOG_INFO(optAlg->logger, "hashbuf Psi Alg set no role switch flag to true");
            noChangeServerRole = true;
        }

        uint32_t *hashBuf = (uint32_t *)hashBufInput;
        if (nullptr == hashBuf)
        {
            LOG_ERROR(optAlg->logger, "hashbuf Psi Alg ,input buf is error");
            return rlt;
        }

        uint64_t realElementNum = psiLen;
        VMIN_ERR_RTN(realElementNum, 1);
        LOG_INFO(optAlg->logger, "input hash element=" << realElementNum);

        std::vector<uint64_t> indexHash(realElementNum);
        initSortIndex(indexHash, realElementNum);

        // now exchange element number to decide use oprf psi or hash psi.
        neles = realElementNum;
        rmtNeles = getUint32FromRmt(optAlg, neles, optAlg->chName);
        LOG_INFO(optAlg->logger, "local neles=" << neles << ",rmt neles=" << rmtNeles);

        // here to set  the party with more elements to be server.
        if (!noChangeServerRole)
        {
            if (SERVER_C == optAlg->role)
            {
                if (neles >= rmtNeles)
                {
                    localClientBin = false;
                    client_neles = rmtNeles;
                }
                else
                {
                    localClientBin = true;
                    client_neles = neles;
                }
            }
            else
            {
                if (neles > rmtNeles)
                {
                    localClientBin = false;
                    client_neles = rmtNeles;
                }
                else
                {
                    localClientBin = true;
                    client_neles = neles;
                }
            }

            if (SERVER_C == optAlg->role && localClientBin)
            {
                LOG_INFO(optAlg->logger, "set server party to be client party.");
                optAlg->role = CLIENT_C;
                optAlg->addr = optAlg->rmtParty.addr;
                optAlg->port = optAlg->rmtParty.port;
            }

            if (CLIENT_C == optAlg->role && !localClientBin)
            {
                LOG_INFO(optAlg->logger, "set client party to be server party.");
                optAlg->role = SERVER_C;
                optAlg->addr = optAlg->localParty.addr;
                optAlg->port = optAlg->localParty.port;
            }
        }

        if (SERVER_C == optAlg->role)
        {
            optAlg->addr = "0.0.0.0";
            LOG_INFO(optAlg->logger, "set server listening addr to 0.0.0.0");
        }

        LOG_INFO(optAlg->logger, "client neles=" << client_neles << ",role=" << optAlg->role);
        LOG_INFO(optAlg->logger, "final local addr=" << optAlg->addr << ":" << optAlg->port);

        // here to begin hashPsi or oprfPsi
        optAlg->neles = neles;
        optAlg->rmtNeles = rmtNeles;
        optAlg->hashBuf = (uint8_t *)hashBuf;
        optAlg->hashLen = 128;
        optAlg->hashIndex = &indexHash;

        //   .Modified by wumingzi/wumingzi. 2022:04:21,Thursday,21:28:06.
        // here to exchange seed.
        // here to disable exchange secret key,which is done in upper main thread function.
        exchangeSecretKey(optAlg);
        //   .Modification over by wumingzi/wumingzi. 2022:04:21,Thursday,21:28:09.

        srvIndexVec.resize(0);
        oprfPsiAlgClient(optAlg, srvIndexVec, get_oprf_values, oprf_values);

        if (CLIENT_C == optAlg->role)
        {
            savePsiRltVec(optAlg->rltVec, rltVec);
            LOG_INFO(optAlg->logger, "hashbufPsiAlgClient alg over. Client get result=" << rltVec.size() << ",srv index size=" << srvIndexVec.size());
        }
        else
        {
            rltVec.resize(0);
            LOG_INFO(optAlg->logger, "hashbufPsiAlgClient alg over. Server get no result=" << rltVec.size());
        }

        return rlt;
    }
    int64_t hashbufPsiAlgClient(uint64_t *hashBufInput, int64_t psiLen, std::vector<uint64_t> &rltVec, OptAlg *optAlg, std::vector<uint64_t> &srvIndexVec, int roleSwitch)
    {
        std::vector<block> tmp;
        return hashbufPsiAlgClient(hashBufInput, psiLen, rltVec, optAlg, srvIndexVec, false, tmp, roleSwitch);
    }

    int64_t hashbufPsiAlgClient(uint64_t *hashBufInput, int64_t psiLen, std::vector<uint64_t> &rltVec, OptAlg *optAlg, std::vector<uint64_t> &srvIndexVec, std::vector<block> &oprf_values, int roleSwitch)
    {
        return hashbufPsiAlgClient(hashBufInput, psiLen, rltVec, optAlg, srvIndexVec, true, oprf_values, roleSwitch);
    }

} // namespace xscePsiAlg
