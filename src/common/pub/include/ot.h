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
 * @file ot.h
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef XSCE_THIRD_PARTY_PIR_ALG_OT_H
#define XSCE_THIRD_PARTY_PIR_ALG_OT_H

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

// for ot api  .Modified by /wumingzi. 2021:07:12,Monday,14:54:28.
#include <libOTe/Tools/Tools.h>
#include <libOTe/Tools/LinearCode.h>
#include <libOTe/NChooseOne/NcoOtExt.h>
#include <libOTe/NChooseOne/Oos/OosNcoOtReceiver.h>
#include <libOTe/NChooseOne/Oos/OosNcoOtSender.h>
#include <libOTe/TwoChooseOne/OTExtInterface.h>

#define SUCCESS_RLT 0
#define ERROR_RLT -1

using namespace osuCrypto;

namespace xsce_ose
{
    typedef struct _NcoOtOpt
    {
        uint32_t dbg = 0;
        uint8_t *dataBuf = nullptr; //send data for sender. save result data for recv.
        uint64_t dataNum = 0;
        uint32_t dataByteLen = 0;

        uint32_t role; //0 indicats sender. 1 indicates recv.
        std::string ip;
        std::string addr;
        uint32_t port;
        std::string rmtIp;
        uint32_t rmtPort;

        //for ot protocol
        uint64_t numOTs;                   //the number of 1-out-of-N OT extensions executed in parallel.
        uint64_t seedOprf;                 //seed for otExtension oprf
        uint64_t seedBaseOt;               //seed for random string in OT. should be the same for both sender&recv.
        uint64_t msgCnt;                   //the size of N in each 1-out-of-N.
        std::vector<uint64_t> chooseIndex; //choose value for each 1-out-of-N OT protoco for recv
        uint32_t stasSecuKey = 40;         //statistical security parameter
        uint64_t maxNumOt = 1000;
        uint64_t maxMsgCnt = 50000000;

        //for future use
        uint64_t simdLen = 1024;
        uint64_t rptTime = 1;
        uint64_t threadNum = 1;
        uint32_t bitSize = 32;
        uint32_t uint = 1;
        uint32_t delay = 0;
        std::string rltFn;
        std::vector<uint64_t> rltIndexVec;

    } NcoOtOpt;

    int64_t NcoOtSend(NcoOtOpt &opt);
    int64_t NcoOtRecv(NcoOtOpt &opt);

    void OT_NChooseOne_Util_Receive_impl(std::string ip_port, int numChosenMsgs, std::vector<u64> &choices, std::vector<block> &recvMsgs);
    void OT_NChooseOne_Util_Send_impl(std::string &ip_port, bool useInputRand, block &seed, Matrix<block> &sendMessages);
    void OT_NChooseOne_Util_Receive(std::string &ip_port, uint64_t numChosenMsgs, uint64_t choice, std::vector<uint64_t> &recvMsg);
    int64_t OT_NChooseOne_Util_Send(std::string &ip_port, std::vector<uint64_t> &seed, uint64_t *sendMessages, int64_t msgCnt, int64_t msgSize);

    void setSenderBaseOtsSeed(NcoOtExtSender &sender,
                              Channel &sendChl, u64 baseCnt, u64 seed);
    void setRecvBaseOtsSeed(NcoOtExtReceiver &recv,
                            Channel &recvChl, u64 baseCnt, u64 seed);

    int NcoOtRecvCh(NcoOtOpt &opt);
    int NcoOtSendCh(NcoOtOpt &opt);

    int64_t clientRcvBuf(NcoOtOpt &opt, uint64_t **buf, uint64_t *len);
    int64_t srvSendBuf(NcoOtOpt &opt, uint64_t *buf, uint64_t len);

    int getLog2(unsigned int x);
    int getNextLog2(unsigned int x);

    int getMd5Char(unsigned char *input, uint32_t len, unsigned char *output);
    int64_t exchangeSeed(int role, std::string ip, int port, uint8_t *seed, uint64_t len, const char* pFLag=NULL);

    static inline int
    isPowOf2(uint32_t x)
    {
        return !(x & (x - 1));
    }

    static inline uint32_t
    nextPowOf2(uint32_t x)
    {
        if (isPowOf2(x))
            return x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return x + 1;
    }

    static inline int getOrder(unsigned long size)
    {
        int order;
        size = (size - 1) >> (0);
        order = -1;
        do
        {
            size >>= 1;
            order++;
        } while (size);
        return order;
    }

}

#endif //XSCE_THIRD_PARTY_PIR_ALG_OT_H
