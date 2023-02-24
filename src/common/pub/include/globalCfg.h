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
 * @file globalCfg.h
 * @author Created by wumingzi on 2021/4/26.
 * @brief 
 * @version 
 * @date 2021-04-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <sys/timeb.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <map>
#include <iterator>
#include <fstream>
#include <cassert>
#include <cstring>
#include <sstream>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <thread>
#include "toolkits/util/include/task_status.h"

namespace xsce_ose {
// Define general function error codde....  Modified by wumingzi on 2021/4/26 5:17 pm.
#define SUCCESS_RLT 0
#define ERROR_RLT -1
// detailed error code added below here.

#define SYNC_SEP ","
#define SYNC_OK 1
#define SYNC_ERR -2
#define MAJOR_ERR -3
#define MINOR_ERR -4

//define alg status string   .Modified by wumingzi. 2022:03:17,Thursday,12:26:38.
#define ALG_DATA_READING "alg is reading data"
#define ALG_DATA_PREPARING "alg is preparing data"
#define ALG_RUN "alg is running"
#define ALG_DATA_SAVE "alg is saving data result"

//   .Modification over by wumingzi. 2022:03:17,Thursday,12:26:43.
enum NetworkMode
{
    NETWORKMODE_DEFAULT = 0,   //  default [proxy or xsce direct connect]
    NETWORKMODE_PORTSHARE = 1, //  port share
    NETWORKMODE_GATEWAY = 2,   //  gateway direct connect
};

typedef struct _TaskInfo
{
    std::string taskId = "taskId";         // node name, should be unnique with in the same XDP instance
    uint32_t curStatke = 1;                // 1 means data preparing, 2 means alg running, 3 means alg failure/completed,4 means alg completed succesfully. 5 means alg aborted.
    std::string curStatus = "alg running"; // data alg running messages.
    uint32_t totalDataVol = 1;             // total data volumne to proceed
    uint32_t fininshedDataVol = 1;         // data volumne fininshed. for lr training, it mabybe indicate the iteration round finnished. for basic computation, it indicates the data size.

    // add more info for task abort and alg chain status  .Modified by wumingzi/wumingzi. 2022:03:17,Thursday,12:14:45.
    std::vector<std::thread::id> thidVec;              //for alg thread management
    std::string subStatus = "";                        //for alg chain. subStatus means the name of one of the alg chains.
    std::string subStep = "";                          //alg step in the alg of subStatus.
    std::map<std::string, std::uint64_t> subProg;      //the step progress.
    std::map<std::string, std::uint64_t> subProgTotal; //total data number of the step.
    int *stopCmd = nullptr;                            //notify alg thread to stop alg when set to 1 or more.

    int subStatusNum = 1; //indicates the number of algs to run sequentially in this task.
    int subStepNum = 0;   //indicates the number of sub steps in this alg [subStatus]
    //if subStepNum is zero, then alg progress is saved in fininshedDataVol.

    //   .Modification over by wumingzi/wumingzi. 2022:03:17,Thursday,12:15:06.
    bool taskRptOver = false; // indicates the task result is reported to scheduler
    int type;                 // task type indicates the algorithm
    int algId;                // task alg index ndicates the algorithm
    int waitTime;             // wait time limit of hours from creation to execution
    time_t createTime;        // task creation time in time struct
    time_t exeTime;           // task execution time in time struct

    double createTimeClk; // clock() time to calculate time gap.
    double exeTimeClk;    // clock() time when begin to execute.

    std::string desp;       //  description information

    int64_t result_rows{-1}; //indicate the sample number of alg result
    int64_t sample_rows{-1}; //indicate the sample number of alg input data
} TaskInfo;

enum mapInfoType
{
    MAPTYPE_NULL = 0,
    PROGRESS = 1,
    LOG = 2,
    RESULT = 3,
};

//   .Modification over by /wumingzi. 2022:02:15,Tuesday,12:00:25.
typedef struct _PartyInfo
{
    std::string addr = "127.0.0.1";
    int port;
    int role;
} PartyInfo;

// define global config prameters....  Modified by wumingzi on 2021/4/26 4:11 pm.
typedef struct _GlobalCfg
{
    std::string nodeId = "";
    std::string nodeName = "";
    std::string cfgFile;

    // not used
    PartyInfo localParty;
    PartyInfo rmtParty;

    // log config....  Modified by wumingzi on 2021/5/13 8:13 pm.
    std::string logRoot = "./";
    // log config, add by light, 2022.03.17
    int loglevel = 1;    //loglevel，0-6，DEBUG,INFO,WARN,ERROR,DPANIC,PANIC,
    int logtype = 0;     //logtype，0-4，PRINT,SAVE_ADD,SAVE_OVER,PRINT_SAVE,SEND
    int timespan = 1000; //print time interval, only for (PRINT)type (ms).

    // for aby2
    int role = 0;
    std::string aby2Circuit = "bin/circ"; // by light 0224.22.10

    // for debug use.....  Modified by wumingzi on 2021/5/13 2:53 pm.
    int shareType = 0;
    int rptTime = 0;
    uint64_t tdata = 0;

    // for multi task use
    std::string taskFile = "taskLog";
    std::string webMgtIp; // web management module ip used for restful api
    int webMgtPort;       // web management module port used for restful api

    int portHttp; // local http listen port,such as 4000
    int portBase; // local port base number for mpc task.
    int portNum;  // local port number for mpc multi task.
    uint32_t *port = nullptr;

    int nodeType = 0; // default 0 means scheduler xsce type.
    std::string algSupport = "";
    std::vector<uint64_t> algIndex;

    // add by light, 2022.03.16
    bool TlsFlag = false; // whether use TLS connection or not, default is false

    std::string localSchedAddr = "0.0.0.0"; //grpc listen ip/port user in xsce
    int localSchedPort = 7310;

    std::vector<std::string> errMsg; // to save global config error message
    std::vector<std::string> logMsg; // to save global log message, optional.

    std::vector<std::string> tlsCertSrv;   // to save tls cert file for remote server side(scheduler)
    std::vector<std::string> tlsCertLocal; // to save tls cert file for local side

    //   .Modification over by /wumingzi. 2022:02:15,Tuesday,12:04:03.
    // mutex definition
    std::mutex *mutexTaskFile;
    std::condition_variable *conTaskFile;
    bool mapTaskFileWritable;
    int mapTaskFileWaitMax;

    std::mutex *mutexMapTaskDel;
    std::condition_variable *conMapTask;
    bool mapTaskWritable;
    int mapTaskWaitMax;

    std::mutex *mutexMapTaskRptDel;
    std::condition_variable *conMapTaskRpt;
    bool mapTaskRptWritable;
    int mapTaskRptWaitMax;

    std::mutex *mutexMapTaskLogSaveDel;
    std::condition_variable *conMapTaskLogSave;
    bool mapTaskLogSaveWritable;
    int mapTaskLogSaveWaitMax;

    std::mutex *mutexPort;
    std::condition_variable *conPort;
    bool portWritable;
    int portWaitMax;
} GlobalCfg;

// ABY options...  Modified by wumingzi on 2021/5/12 3:26 pm.
#define ABY_SERVER_C 0
#define ABY_CLIENT_C 1

#define BOOL_SHARING 1
#define ARITH_SHARING 2
#define YAO_SHARING 3

// aby alg parameters....  Modified by wumingzi on 2021/5/12 3:24 pm.
typedef struct _OptAlg
{
    uint32_t dbg = 0;
    std::string nodeToken = "abcdefhg";
    uint32_t role = 0;
    uint32_t oriRole = 0; // the original role. some alg may change role
    char *sa = nullptr;
    std::string addr = "";
    std::string cfgFile;
    uint32_t port = 0;
    uint32_t secuK = 0;
    uint32_t bitlen = 0;
    uint32_t simdLen = 0;
    uint32_t stdoutFlag = 1;
    uint32_t type = 0;
    uint32_t cpuNum = 0;

    std::string algIndexStr;
    std::vector<uint64_t> algIndexVec;
    bool regressionFlag = false;

    std::vector<uint64_t> portVec;
    std::vector<std::string> ipVec;
    int64_t portNum = 1; // indicates port numbers.
    int64_t thdNum = 1;  // indicates thd numbers.
    int64_t aby2ThdNum = 1;

    // for scheduler use  .Modified by wumingzi/wumingzi. 2022:02:15,Tuesday,12:17:22.
    // use taskJobId and globalCfg to find the current task job info.
    std::string taskJobId = "";
    GlobalCfg *globalCfg = nullptr;
    bool optCopyOver = false;

    //   .Modification over by wumingzi/wumingzi. 2022:02:15,Tuesday,12:17:36.
    // for multi thread
    int64_t thdIdex = 0;  // current thread index;
    bool thdOver = false; // true means current thread is over.
    int64_t startLoc = 0; // alg data offset calculated in current thread
    int64_t dataLen = 0;  // alg data length calculated in current thread

    // std::vector<PartyInfo> partyList;
    PartyInfo localParty;
    PartyInfo rmtParty;
    std::string circuitDir = "bin/circ"; // by light 2022.0305

    uint32_t rptTime = 1;
    int tdata = 1000;
    int tdata2 = 2;
    uint32_t circuitType = 0;
    int alg = 10;
    int algInChainIndex = 0;
    std::vector<uint64_t> rltVec;
    std::vector<uint64_t> *rltVecPtr;

    // for data file
    std::string dataFn;
    int64_t row = 0;
    int64_t col = 0;
    int64_t headLine = 1; // for sample data file: the first headline rows are omited
    int64_t colLine = 0;  // for sample data file: the first colLine columns are omited

    int64_t featureIdIndex = -1; // for sample data file: -1 means no feature id provided.
    int64_t sampleIdIndex = -1;  // for sample data file: -1 means no sample id provided.

    int64_t lableHeadLine = 0; // for lable data file: the first headline rows are omited
    int64_t lableColLine = 0;  // for lable data file: the first colLine columns are omited

    int64_t lableIndex = -1; // lable value in sample data column index.
    int64_t lableParty = -1; // speicfy which party's label is valid in training in vertical mode.

    std::vector<uint64_t> colIndexVec;
    std::vector<uint64_t> rowIndexVec;
    std::string rltFn = "rlt";

    // for training data  .Modified by wumingzi/wumingzi. 2021:08:26,Thursday,15:32:13.
    int64_t sampleNum = 0;
    int64_t featureNum = 0;
    double **sampleDataBuf = nullptr;
    double *lableDataBuf = nullptr;

    //   .Modified by /wumingzi. 2021:09:04,Saturday,09:43:39.
    // for training test data  .Modified by /wumingzi. 2021:08:26,Thursday,15:32:13.
    // for test data to verify training result.
    int64_t testSampleNum = 0;
    double **testSampleDataBuf = nullptr;
    double *testLableDataBuf = nullptr;

    // for debug use
    int64_t exitCode = 0;
    int64_t port3 = 1;  // to avoid osu session ch bug
    int64_t port4 = 2;  // to avoid osu session ch bug
    int64_t testN = 3;  // show data numer in alg.
    int64_t intMul = 1; // define integar mulplication factor
    int64_t intAdd = 1; // define integar addtion factor
    //int64_t errThd = 1000; // define double accuracy threshold.
    int64_t errThd = 3;  // modified by light, 2022.05.12
    int64_t showCnt = 3; //for debug. added by pgm 2022.6.20
    
    // for psi alg
    uint64_t neles = 0;
    uint64_t rmtNeles = 0;
    uint32_t hashLen = 128;
    uint8_t *hashBuf = nullptr;
    std::vector<uint64_t> *hashIndex;
    uint64_t oprfWidth = 632;
    uint64_t oprfLogHeight = 20;
    uint64_t oprfHashLenInBytes = 10;
    uint64_t oprfBucket1 = 8;
    uint64_t oprfBucket2 = 8;

    // for basic computation use
    int64_t comptMode = 0;

    // for joint statistics use
    int64_t statMode = 0;

    // for feature WoE alg
    int64_t groupNum = 10;
    double ivMin = 0.0;
    double ivMax = 10.0;
    int64_t topNum = 10;

    int64_t minSampleNum = 0;
    int64_t minFeatureNum = 0;
    int learningRate = 10;
    std::string lableFile = "";
    std::string sampleFile = "";
    std::string testSampleFile = "";
    std::string testLableFile = "";

    int64_t batchSize = 0;
    int64_t iteration = 20;
    uint64_t floatDecision = 16;
    std::string colIndexFile = "";
    std::string sampleIndexFile = "";

    // 0 means no need to run psi in ml alg, 1 means runing psi
    int64_t psiFlag = 0;

    // 0 means default vertical fl, 1 selects horizontal fl
    int64_t flMode = 0;
    bool lableRequired = true;

    // for pir alg.max string lengh in char.
    int64_t dataRowLen = 128;

    // use bucketNum to replace dataLen  .Modified by wumingzi. 2023:02:21,Tuesday,10:52:48.
    int64_t bucketNum = 1;
    //   .Modification over by wumingzi. 2023:02:21,Tuesday,10:52:58.
    
    // for spdz common algs.
    std::string formula_str;
    std::vector<int> n_inputs;
    int loop = 1;

    // for debug
    int printFlag = 0;

    // for task status
    std::string taskId = "";
    std::string logDir = "";
    std::string logall = "";
    bool algOver = false;
    std::string logfn;
    uint32_t status = 0;
    std::string msg = "";
    void *statusPtr = nullptr;
    std::string chName = "";

    int *stopCmd = nullptr; //notify alg thread to stop alg when set to 1 or more.

    // guard condition   .Modified by /wumingzi. 2021:08:25,Wednesday,12:21:02.
    std::mutex mutexLogSave;
    std::condition_variable conLogSave;

    // add random seed  & progress  .Modified by /wumingzi. 2021:07:23,Friday,10:05:09.
    uint64_t *progress = nullptr;
    uint64_t commonSeed = 0xAA55AA55AA55AA55;
    uint64_t commonSeed1 = 0xAA55AA55AA55AA55;
    uint64_t inertalSeed = 0x1122334455667788;
    uint64_t inertalSeed1 = 0x1122334455667788;

    //  add by ligang
    uint32_t networkmode = NETWORKMODE_DEFAULT;
    std::string gateway_cert_path;              //  cert path，valid when networkmode=2
    std::string endpointlist_spdz;
    std::string endpointlist_spdz_gw;

    xsce_ose_taskstatus::TaskStatus task_status;
} OptAlg;

typedef struct _LogSaveOpt
{
    std::mutex *mutexLogSave = nullptr;        // store mutex pointer for multi-thread
    std::queue<std::string> *msgQue = nullptr; // store alg running msg.
    int64_t logLevel = 0;
} LogSaveOpt;
// modfication over.  Modified by .pan on 2021/5/12 3:24 pm.
} // namespace xsce_ose
