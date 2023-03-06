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

#include "lr.h"
#include "common/pub/include/util.h"
#include "common/pub/include/ot.h"
#include "PSI/include/psi.h"
#include "pir/include/pir.h"

#include "toolkits/util/include/xutil.hpp"
#include "common/pub/include/globalCfg.h"

#include <thread>
#include <functional>

using namespace std;
using namespace xsceLogRegAlg;
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
typedef std::function<int64_t(OptAlg *, OseOpt *)> UtFunction;
int64_t launchThread(UtFunction &func, std::vector<OptAlg> *opt_vec, std::vector<OseOpt> *ose_vec);
int64_t test_all(int role, int64_t srvSize, int64_t cliSize, int step, int alg, std::string ip, int port);

// int64_t pirUtest(std::vector<OptAlg> *opt_vec, std::vector<OseOpt> *ose_vec);
int64_t lrUtest(OptAlg *opt, OseOpt *pir_opt);

//for generate test data use
void initLrTrainData(int sample_num, int feature_num, std::vector<float> &model);
void initLrOptBuf(OptAlg *opt, double **sampleDataBuf, double *labelDataBuf, int row, int col);

template <typename T>
void vec_mul_vec(std::vector<T> &vec1, std::vector<T> &vec2, T &rlt)
{
    auto len1 = vec1.size();
    auto len2 = vec2.size();
    rlt = 0;
    if (len1 != len2 || len1 < 1)
        return;

    T mul_rlt = 0;
    for (int64_t i = 0; i < len1; i++)
    {
        mul_rlt += vec1.at(i) * vec2.at(i);
    }

    rlt = mul_rlt;
}

template <typename T>
void mtx_mul_vec(std::vector<std::vector<T> > &mtx, std::vector<T> &vec2, std::vector<T> &rlt)
{
    auto len1 = mtx.size();
    auto len2 = vec2.size();
    rlt.resize(0);
    if (len1 < 1 || len2 < 1)
        return;
    rlt.resize(len1);

    for (int64_t i = 0; i < len1; i++)
    {
        std::vector<T> &cur_vec = mtx.at(i);
        T mul_rlt = 0;
        vec_mul_vec(cur_vec, vec2, mul_rlt);
        rlt.at(i) = mul_rlt;
    }
}

template <typename T>
void save_mtx(std::vector<std::vector<T> > &mtx, std::string fn, int start_col, int save_col)
{
    int64_t flushLen = 100000;
    std::string sep = ",";
    auto len = mtx.size();
    std::ofstream outf;
    outf.open(fn.data());
    if (outf.fail())
    {
        std::cout << "open file error at " << fn << std::endl;
    }

    auto end_col = start_col + save_col;
    for (int64_t i = 0; i < len; i++)
    {
        std::vector<T> &cur_row = mtx.at(i);
        auto total_len = cur_row.size();
        for (int64_t j = start_col; j < end_col && j < total_len; j++)
        {
            auto data = cur_row.at(j);
            outf << LEFTFIX(8) << data << sep;
        }
        outf << "\n";

        if (0 == (i + 1) % flushLen)
            outf.flush();
    }

    outf.close();
}

template <typename T>
void save_vec(std::vector<T> &mtx, std::string fn)
{
    int64_t flushLen = 100000;
    std::string sep = ",";
    auto len = mtx.size();
    std::ofstream outf;
    outf.open(fn.data());
    if (outf.fail())
    {
        std::cout << "open file error at " << fn << std::endl;
    }

    for (int64_t i = 0; i < len; i++)
    {
        auto data = mtx.at(i);
        outf << LEFTFIX(8) << data;
        outf << "\n";

        if (0 == (i + 1) % flushLen)
            outf.flush();
    }

    outf.close();
}





void readLrTestData(OptAlg *opt);
// int getStrMtxFromCsvFile(std::string fn, std::vector<std::vector<std::string> > &rlt);
void readFloatDataMtx(std::string fn, std::vector<std::vector<float> > &mtx);
void readFloatDataVec(std::string fn, std::vector<float> &vec);


#endif //XSCE_THIRD_PARTY_PIR_ALG_MAIN_H
