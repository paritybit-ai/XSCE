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
 * @file pir.h
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef XSCE_THIRD_PARTY_PIR_ALG_PIR_H
#define XSCE_THIRD_PARTY_PIR_ALG_PIR_H

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

namespace xscePirAlg
{
    using OptAlg = xsce_ose::OptAlg;
    using namespace osuCrypto;
    //here for  solo pir project which will be open sourced.
    
    // add err code for return   .Modified by wumingzi. 2022:09:07,Wednesday,22:42:35.
    #define OSE_ALG_GLOBAL_CONFIG_ERROR -1001
    #define OSE_ALG_CHECK_OPT_PARAM_ERROR -1002
    #define OSE_ALG_INPUT_CONFIG_ERROR -1004
    #define OSE_ALG_DATA_READ_ERROR -1005
    #define OSE_ALG_DATA_ALIGN_ERROR -1006
    #define OSE_ALG_INPUT_OPT_ERROR -1007
    #define OSE_ALG_INPUT_STATUS_ERROR -1008
    //   .Modification over by wumingzi. 2022:09:07,Wednesday,22:42:54.

    typedef struct _AlgStatus
    {
        uint64_t status = 0;
        int statusIndex = -1; //indicate statusVec index which need to be print
        std::vector<uint64_t> statusVec;
        std::vector<std::string> msgVec;
        std::vector<std::string> msgErrVec;
        uint64_t loopInterVal;
        std::string statusFile;
        std::string algName;
        std::string taskName;

    } AlgStatus;

    typedef struct _PirOtInfo
    {
        //for both side
        int role = 0; //0 means server, 1 means client.
        int pool_index = 0;
        int64_t element = 0; //hold server send data number.
        uint8_t *ot_send_buf = nullptr;
        uint8_t **ot_rlt_buf = nullptr;

        int32_t msg_byte_len = 16; //the byte length of each hash vaule.

    } PirOtInfo;

    typedef struct _PirAlgInfo
    {
        //for both side
        int role = 0;
        int pool_index = 0;                             //0 means server, 1 means client.
        uint8_t *id_hash_buf = nullptr;                 //id hash number buf.
        std::vector<uint64_t> *id_hash_index = nullptr; //holds the id_hash index after sorting
        int32_t hash_byte_len = 16;                     //the byte length of each hash vaule.
        int64_t id_num = 0;                             //the number of id string.
        int64_t max_str_len = 128;                      //the maximum string length of server data row.
        int secu_key = 100;

        //for server only
        int garble_hash_value = 1;
        std::vector<std::string> *data_row = nullptr; //hold the id string for srv
        std::vector<int64_t> matched_bucket;          //holds the index of buckets containing query id.

        //for cli only
        std::vector<std::string> *result = nullptr; //hold the id string for both cli
                                                    //cli bucket status
        uint64_t *bucket_range_buf = nullptr;
        int64_t total_bucket_num = 0;
        std::vector<int64_t> id_bucket_index;
        std::vector<uint64_t> id_range_index; //holds id range in bucket after updating data

    } PirAlgInfo;

    typedef struct _PirDataInfo
    {
        //for both side
        int role = 0;         //0 means server, 1 means client.
        int secu_key = 10000; //secuKey indicates the number of each bucket in pir alg.
        int bucket_pool_size = 1000 * 1000;
        int bucket_pool_num = 1;
        std::vector<std::vector<std::string> > id_str; //hold the id string for both srv&cli

        //for both srv&cli
        uint32_t *id_hash_buf = nullptr; //id hash number buf.
        int64_t id_num = 0;              //the number of id string.
        int64_t id_hash_byte_len = 16;   //the byte length of id hash value,usually 16 .
        int index_char_pos = 0;          //select the byte of hash value to split buckets.

        std::vector<int64_t> *original_index_id;   //the index id of input id_hash_buf
        std::vector<int64_t> bucket_pool_index_id; //the pool index of each id str

        std::vector<std::string> *original_id_str = nullptr;
        std::vector<std::string> *original_data_str = nullptr;

        std::vector<uint32_t *> bucket_pool_buf; //the bucket pool which contains multiple bucket;
        std::vector<int64_t> bucket_pool_vol;    //the volume of each bucket pool;

        //for pir srver only.
        std::vector<std::vector<std::string> > bucket_pool_data_row; //the bucket pool which contains srv data row;
        int64_t max_str_len = 128;                                   //the maximum string length of server data row.

        //for cli to save result
        std::vector<std::vector<std::string> > *pir_rlt;

        //for idle data.
        uint8_t idle_data_row = 0;
        uint8_t idle_id_str_cli = 0;
        uint8_t idle_id_str_srv = 0xff;

    } PirDataInfo;

    typedef struct _PirBasicOpt
    {
        //for both side
        int num = -1;                                  //indicates the order of thid pir basic called by applicaiton.
        int64_t totalElement = 0;                      //the total number of element.
        int64_t startLoc = -1;                         //the start location for hash data to psi input.
        int64_t element = 0;                           //the number of element for pir input.
        int64_t elementLen = 16;                       //byte lenght fo each psi element.
        int64_t maxStrLen = 128;                       //max data string length in
        std::vector<uint64_t> *idxVec = nullptr;       //for client input id
        std::vector<std::string> *idxStrVec = nullptr; //for client input id

        //for server side
        uint32_t *hashBuf = nullptr;                 //buf for hash data
        uint64_t *encBufBase = nullptr;              // save encrypted data string.
        uint64_t *keyBufBase = nullptr;              // save aes key. generated by encStrVec2Buf internally.
        uint64_t strEncLen = 16;                     //data string encrypted length in bytes
        uint64_t keyLen = 16;                        //aes key lenght in bytes
        std::vector<std::string> *dataVec = nullptr; //for server data input

        //for client side to save each basic pir result
        std::vector<std::int64_t> *pirRltIdx = nullptr;    //save pir result to this pointer.
        std::vector<std::string> *pirRlt = nullptr;        //save pir result to this pointer.
        std::vector<std::int64_t> *pirRltSrvIdx = nullptr; //save the pir result data's index in server side
        int64_t srvBucketBase = 0;                         //the base index of current bucket in server side
        int64_t srvBucketLen = 0;                          //the length index of current bucket in server side

    } PirBasicOpt;

#define GET_POOL_INDEX(val, sum, num) ((val) * (num) / (sum))

    //here move the index bytes to the beginning of dst buffer.
    inline void copyBufWithIndex(uint8_t *dst, uint8_t *src, int64_t len, int index_pos, int index_len)
    {

        for (int64_t i = 0; i < index_pos; i++)
        {
            dst[i + index_len] = src[i];
        }

        //move index byte to msb pos of new buf.
        //for example:  Byte0 Byte1 Byte2 Byte3 Byte4 Byte5 Byte6 Byte7
        //index_pos = 3, index_len = 2;
        //then in new buf,byte order is: Byte0 Byte1 Byte2  Byte5 Byte6 Byte7 Byte3 Byte4
        for (int64_t i = index_pos; i < index_pos + index_len; i++)
        {
            dst[len - index_len + i - index_pos] = src[i];
        }
        for (int64_t i = index_pos + index_len; i < len; i++)
        {
            dst[i - index_len] = src[i];
        }

        return;
    }

    int64_t pir2PartyAlgTerminal(OptAlg *optAlg);
    int64_t pir2PartyAlgTerminalSingle(OptAlg *optAlg);
    int64_t pir2PartyAlgTerminalBatch(OptAlg *optAlg);

    int64_t pir2PartyAlgTerminalPool(OptAlg *optAlg, PirDataInfo *data_info, int pool_num);
    int64_t pir2PartyAlgTerminalBasic(OptAlg *optAlg, PirAlgInfo *alg_info);

    int64_t pirPoolSplitBucket(OptAlg *optAlg, PirAlgInfo *alg_info);
    int64_t pirBucketRange(PirAlgInfo *alg_info, uint8_t **range_buf, int64_t *bucket_num);
    int64_t pirBucketFilter(OptAlg *optAlg, PirAlgInfo *alg_info);
    int64_t pirBucketDataUpdate(OptAlg *optAlg, PirAlgInfo *alg_info);

    int64_t pirAesDecode(uint8_t *cipher_buf, uint8_t *key_buf,
                         int msg_aes_len,
                         const std::vector<uint64_t> &cli_rlt,
                         const std::vector<uint64_t> &srv_rlt, std::vector<std::string> *str_rlt);
    int64_t savePirRlt2Vec(const std::vector<uint64_t> &psi_rlt, int64_t msg_aes_len, uint64_t *str_buf, std::vector<std::string> *rlt_str);

    bool checkLocalRmtNumNonZero(OptAlg *optAlg, int64_t num, int pool);

    int64_t splitIdBufBucket(PirDataInfo *data_info);
    int64_t splitIdBufBucketByPoolNum(PirDataInfo *data_info);
    int64_t splitIdBufBucketByPoolSize(PirDataInfo *data_info);

    bool checkPirDataInfo(PirDataInfo *data_info, std::string &msg);

    void showPirDataInfo(PirDataInfo *data_info, int show_cnt = 0);

    std::string showPirOpt(PirBasicOpt *opt);

    int64_t savePirStrBufRlt(uint64_t *strBuf, int64_t strNum, int64_t decStrEncLen, std::vector<std::string> *rltStr);
    int64_t copyPirOptBuf(uint32_t *dst, uint32_t *src, std::vector<int64_t> *startVec, std::vector<int64_t> *lenVec, int eleLen, int64_t total);
    int64_t copyPirOptEncBuf(uint64_t *dst, std::vector<uint64_t *> *srcVec, std::vector<int64_t> *lenVec, int eleLen, int64_t total);
    int64_t combinePirOpt(OptAlg *optAlg, std::vector<PirBasicOpt> *pirOptVec, PirBasicOpt *pirOpt, int base, int len);
    int64_t freePirOptBuf(PirBasicOpt *pirOpt);
}

#endif //XSCE_THIRD_PARTY_PIR_ALG_PIR_H
