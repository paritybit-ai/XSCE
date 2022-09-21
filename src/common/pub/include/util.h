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
 * @file util.h
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef XSCE_THIRD_PARTY_PIR_ALG_UTIL_H
#define XSCE_THIRD_PARTY_PIR_ALG_UTIL_H

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>

#include <string>
#include <iomanip>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

#include "toolkits/util/include/xlog.h"
#include "common/pub/include/globalCfg.h"

#define CLIENT_C 1
#define SERVER_C 0

#define PTR_ERR_RTN(x)                     \
    {                                      \
        if (nullptr == (x))                \
        {                                  \
            LOG_ERROR("(x)"                \
                      << " is invalid. "); \
            return -1;                     \
        }                                  \
    }

#define VMIN_ERR_RTN(x, val)                   \
    {                                          \
        if ((x) < (val))                       \
        {                                      \
            LOG_ERROR((x) << " is invalid. "); \
            return -1;                         \
        }                                      \
    }

namespace util
{
    using OptAlg = xsce_ose::OptAlg;
    //here for  solo pir project which will be open sourced.
    class TimeUtils
    {
    public:
        struct timeval startTime, stopTime;
        double timeElapsed;

        TimeUtils();
        void start(std::string msg, int num);
        void start(std::string msg);
        void stopMs(std::string msg);
        void stopS(std::string msg);
        void stopS(std::string msg, int num);

        std::string strStart(std::string msg);
        std::string strStopMs(std::string msg);
        std::string strStopS(std::string msg);

        float getTime();
        void clear();
    };

    typedef struct _PartyInfo
    {
        std::string addr = "127.0.0.1";
        int port;
        int role;
    } PartyInfo;

    int64_t copyThdAlgOpt(OptAlg *optDst, OptAlg *optSrc, int thdIdx);
    int64_t copyAlgOpt(OptAlg *optDst, OptAlg *optSrc);

    int64_t savePirRltThd(std::vector<std::vector<std::string> > &pirRltVec,
                          std::vector<std::string> *pirRlt);

    int64_t savePirRlt2File(std::vector<std::vector<std::string> > *pirRltVec, std::string fn);
    int64_t savePirRlt2StrVec(std::vector<std::vector<std::string> > *pirRltVec, std::vector<std::string> &vec);
    int64_t savePsiRlt2IntVec(std::vector<std::vector<std::int64_t> > *pirRltVec, std::vector<std::int64_t> &vec);


    void showHexValue(uint32_t *buf, uint64_t dataLen, uint64_t len);
    // tcp tool

    int64_t cliRcvBuf(OptAlg *optAlg, uint64_t **buf, uint64_t *len);
    int64_t srvSendBuf(OptAlg *optAlg, uint64_t *buf, uint64_t len);

    int64_t cliSendBuf(OptAlg *optAlg, uint64_t *buf, uint64_t len);
    int64_t srvRcvBuf(OptAlg *optAlg, uint64_t **buf, uint64_t *len);

    int64_t chSendBufWithRcv(std::string ip, uint64_t *sendBuf, uint64_t sendLen, uint64_t **rcvBuf, uint64_t rcvLenMax, std::string chName = "");
    int64_t chRecvBufWithSend(std::string ip, uint64_t **rcvBuf, uint64_t rcvLen, uint64_t *sendBuf, uint64_t sendLen, std::string chName = "");

    int64_t rcvChTcpVecSync(std::string ip, int port, std::vector<uint64_t> &rcvVec, std::vector<uint64_t> &sendVec, std::string chName = "");
    int64_t sendChTcpVecSync(std::string ip, int port, std::vector<uint64_t> &rcvVec, std::vector<uint64_t> &sendVec, std::string chName = "");

    uint32_t getUint32FromRmt(OptAlg *optAlg, uint32_t num, std::string chName);
    int64_t exchangeSecretKey(OptAlg *optAlg);

    //aes tool
    uint64_t getRand(uint64_t max);
    int64_t encStrVec2BufIndex(std::vector<std::string> &strVec, int64_t strLen, std::vector<uint64_t> seed, uint64_t **dataBuf, uint64_t **keyBuf, std::vector<std::int64_t> &indexVec);
    int64_t convertStrVec2Md5Index(std::vector<std::string> &strVec, uint32_t *&buf, std::vector<std::int64_t> &indexVec);
    int64_t convStr2Buf(std::string &str, uint64_t *buf, int64_t len);

    int64_t encStr2BufIndex(std::vector<std::string> *strVec, int64_t strLen, std::vector<uint64_t> seed, uint64_t **dataBuf, uint64_t *keyBuf, std::vector<std::int64_t> *indexVec);
    int64_t encStr2BufIndex(std::vector<std::string> *strVec, int64_t strLen, std::vector<uint64_t> seed, uint64_t **dataBuf, uint64_t **keyBuf, std::vector<std::int64_t> *indexVec);

    int getMd5(unsigned char *input, uint32_t len, unsigned char *output);
    int64_t aesEncBUf(uint64_t *src, uint64_t *dst, int64_t len, uint64_t *key);
    int64_t aesDecBUf(uint64_t *src, uint64_t *dst, int64_t len, uint64_t *key);

    template <typename datatype>
    int64_t splitHalfVector(std::vector<datatype> *dst1, std::vector<datatype> *dst2, std::vector<datatype> *src)
    {
        int64_t rlt = -1;

        if (nullptr == src)
        {
            LOG_INFO("splitHalfVector. input invalid src pointer=" << src);
            return rlt;
        }

        if (nullptr == dst1)
        {
            LOG_INFO("splitHalfVector. input invalid dst1 pointer=" << dst1);
            return rlt;
        }

        if (nullptr == dst2)
        {
            LOG_INFO("splitHalfVector. input invalid dst2 pointer=" << dst2);
            return rlt;
        }

        uint64_t len = src->size();
        LOG_INFO("splitHalfVector. input len=" << len);

        if (len < 1)
        {
            LOG_INFO("splitHalfVector. input invalid len=" << len);
            return rlt;
        }

        dst1->resize(len / 2);
        dst2->resize(len / 2);

        for (uint64_t i = 0; i < len / 2; i++)
        {
            dst1->at(i) = src->at(i * 2);
            dst2->at(i) = src->at(i * 2 + 1);
        }

        return len;
    }

    template <typename datatype>
    void initSortIndex(std::vector<datatype> &vec, uint64_t len)
    {
        if (len < 1)
            return;

        vec.resize(len);
        for (uint64_t i = 0; i < len; i++)
        {
            vec.at(i) = i;
        }
    }

    template <typename datatype>
    void initSortIndex(std::vector<datatype> *vec, uint64_t len)
    {
        if (len < 1)
            return;

        vec->resize(len);
        for (uint64_t i = 0; i < len; i++)
        {
            vec->at(i) = i;
        }
    }

}
#endif //XSCE_THIRD_PARTY_PIR_ALG_UTIL_H
