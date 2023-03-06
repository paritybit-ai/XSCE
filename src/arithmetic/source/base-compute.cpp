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
 * @file base-compute.cpp
 * @author Created by chonglou. 2022:05:28
 * @brief 
 * @version 
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "base-compute.h"
#include "base-compute.hpp"

#include <unistd.h>
#include <math.h>
#include <sys/io.h>
#include <sys/stat.h>

#include <fstream>
#include <algorithm>
#include <string>
#include <thread>

#include "toolkits/util/include/xlog.h"

#define OUT_FILE_PATH "Player-Data"

/**
 * @brief Set the up files object
 * 
 * @param spdzalg 
 * @return int 
 */
int setupFiles(SPDZAlg *spdzalg)
{
    if (!spdzalg)
    {
        LOG_ERROR("bad parameter for spdz framework.");
        return -1;
    }
    std::string dir(OUT_FILE_PATH);
    if (access(dir.c_str(), 0) == -1)
    {
        mkdir(dir.c_str(), S_IRWXU | S_IRWXG);
    }

    return 0;
}

/**
 * @brief init spdz input file
 * 
 * @param spdzalg 
 * @return int 
 */
int initInputFiles(SPDZAlg *spdzalg)
{
    std::string middlename = spdzalg->middle_prefix + "-P" + std::to_string(spdzalg->role) + "-";
    size_t thd_num = spdzalg->thread_num;
    size_t total_data_size = spdzalg->input.size();
    size_t data_size_per_thd = total_data_size / thd_num;
    if (total_data_size % thd_num)
    {
        LOG_ERROR("The total amount of data cannot be divided by thread num.");
        return -1;
    }

    std::thread threads[MAX_THREAD_NUM];

    if (1 == thd_num)
    {
        threads[0] = std::thread([=]
            {
                std::ofstream ostream(middlename+std::to_string(0), std::ofstream::out|std::ofstream::trunc);   // xxx-Px-0, input in main thread
                for (vector<double>::size_type ix=0; ix!=data_size_per_thd; ++ix){
                    ostream << spdzalg->input[ix] << std::endl;
                }
                ostream.close(); });
    }
    else
    {
        for (size_t i = 0; i < thd_num; i++)
        {
            threads[i] = std::thread([=]
                {
                    std::ofstream ostream(middlename+std::to_string(i+1), std::ofstream::out|std::ofstream::trunc);     // input from xxx-Px-1 ~ xxx-Px-n, main thread not used for calculation
                    for (vector<double>::size_type ix=0; ix!=data_size_per_thd; ++ix){
                        ostream << spdzalg->input[i*data_size_per_thd+ix] << std::endl;
                    }
                    ostream.close(); });
        }
    }

    for (size_t i = 0; i < thd_num; i++)
    {
        threads[i].join();
    }

    return 0;
}

/**
 * @brief init input file for variance algorithm
 *
 * @param spdzalg
 * @param data_size
 * @param sum
 * @param square_sum
 * @return int
 */
int initInputFiles(SPDZAlg *spdzalg, size_t data_size, double sum, double square_sum)
{
    std::string middle_name = spdzalg->middle_prefix + "-P" + std::to_string(spdzalg->role) + "-0";
    std::ofstream ostream;
    ostream.open(middle_name, std::ofstream::out | std::ofstream::trunc);
    if (ostream.fail())
    {
        LOG_ERROR("open file failed.");
        return -1;
    }
    ostream << data_size << std::endl;
    ostream << sum << std::endl;
    ostream << square_sum << std::endl;

    return 0;
}

/**
 * @brief resolve output
 *
 * @param spdzalg
 * @return int
 */
int resolveOutput(SPDZAlg *spdzalg)
{
    if (0 == spdzalg->output_file.size())
    {
        return 0;
    }
    string filename;
    size_t real_thd_num = spdzalg->thread_num + 1; // spdzalg->thread_num is only child thread num.

    remove(spdzalg->ip_file_name.c_str());   // ignore ip file if it does not exist.

    if (1 == spdzalg->thread_num)
    {
        filename = spdzalg->output_file + "-P" + std::to_string(spdzalg->role) + "-0";
        rename(filename.c_str(), spdzalg->output_file.c_str());
    }
    else
    {
        ofstream ostream(spdzalg->output_file, ios::out | ios::trunc);
        for (size_t i = 0; i < real_thd_num; i++)
        {
            filename = spdzalg->output_file + "-P" + std::to_string(spdzalg->role) + "-" + std::to_string(i);
            ifstream istream(filename, ios::in);
            string line;
            while (getline(istream, line))
                ostream << line << endl;
            istream.close();
            remove(filename.c_str());
        }
        ostream.close();
    }

    if (!access(spdzalg->pub_input_file.c_str(), F_OK))
    {
        remove(spdzalg->pub_input_file.c_str());
    }

    for (size_t i = 0; i < real_thd_num; i++)
    {
        std::string middlename = spdzalg->middle_prefix + "-P" + std::to_string(spdzalg->role) + "-" + std::to_string(i);
        if (!access(middlename.c_str(), F_OK))
        {
            remove(middlename.c_str());
        }
    }

    return 0;
}

/**
 * @brief resolve spdz output and store the results in vector
 *
 * @param spdzalg
 * @param store_rlt_vec
 * @return int
 */
int resolveOutput(SPDZAlg *spdzalg, bool store_rlt_vec)
{
    if (!store_rlt_vec)
    {
        return resolveOutput(spdzalg); //  default output to file
    }

    if (0 == spdzalg->output_file.size())
    {
        return 0;
    }

    //  output to vector
    string filename;
    size_t real_thd_num = spdzalg->thread_num + 1; // spdzalg->thread_num is only child thread num.

    remove(spdzalg->ip_file_name.c_str());   // Ignore if file do not exists.

    if (1 == spdzalg->thread_num)
    {
        filename = spdzalg->output_file + "-P" + std::to_string(spdzalg->role) + "-0";
        rename(filename.c_str(), spdzalg->output_file.c_str());
        ifstream istream(spdzalg->output_file.c_str(), ios::in);
        string line;
        while (getline(istream, line))
        {
            double d = atof(line.c_str());
            spdzalg->result.push_back(d);
        }
        istream.close();
        remove(spdzalg->output_file.c_str());
    }
    else
    {
        ofstream ostream(spdzalg->output_file, ios::out | ios::trunc);
        for (size_t i = 0; i < real_thd_num; i++)
        {
            filename = spdzalg->output_file + "-P" + std::to_string(spdzalg->role) + "-" + std::to_string(i);
            ifstream istream(filename, ios::in);
            string line;
            while (getline(istream, line))
            {
                ostream << line << endl;
                double d = atof(line.c_str());
                spdzalg->result.push_back(d);
            }
            istream.close();
            remove(filename.c_str());
        }
        ostream.close();
    }

    if (!access(spdzalg->pub_input_file.c_str(), F_OK))
    {
        remove(spdzalg->pub_input_file.c_str());
    }

    for (size_t i = 0; i < real_thd_num; i++)
    {
        std::string middlename = spdzalg->middle_prefix + "-P" + std::to_string(spdzalg->role) + "-" + std::to_string(i);
        if (!access(middlename.c_str(), F_OK))
        {
            remove(middlename.c_str());
        }
    }

    return 0;
}

/**
 * @brief spdz computation
 *
 * @param spdzalg
 * @param parties
 * @param machine_type
 * @return int
 */
int runSPDZComputation(SPDZAlg *spdzalg, int parties, int machine_type=EMACHINETYPE_NONE)
{
    if (0 == spdzalg->middle_prefix.size())
    {
        spdzalg->middle_prefix = "tmp";
    }
    spdzalg->pub_input_file = "Programs/Public-Input/" + spdzalg->alg_index_str;
    ofstream ofile;
    ofile.open(spdzalg->pub_input_file, fstream::out);
    ofile << spdzalg->input.size();
    ofile.close();

    if (1 == spdzalg->input_mode)
    {
        return runSPDZComputation_mem(spdzalg, parties, machine_type);
    }

    setupFiles(spdzalg);
    initInputFiles(spdzalg);

    // calc threads start inside
    runSPDZMachine<void>(spdzalg, parties, machine_type);

    resolveOutput(spdzalg);

    return 0;
}

/**
 * @brief inner product computation for lr alg
 *
 * @param spdzalg
 * @param parties
 * @param chunk
 * @return int
 */
int runProDotComputation(SPDZAlg *spdzalg, int nParties, int chunk)
{
    spdzalg->pub_input_file = "Programs/Public-Input/" + spdzalg->alg_index_str;
    ofstream ofile;
    ofile.open(spdzalg->pub_input_file, fstream::out);
    ofile << spdzalg->input.size()/chunk << endl;
    ofile << spdzalg->input.size();
    ofile.close();

    if (1 == spdzalg->input_mode)
    {
        return runSPDZComputation_mem(spdzalg, nParties, true);
    }

    setupFiles(spdzalg);
    initInputFiles(spdzalg);

    // calc threads start inside
    int rlt = runSPDZMachine<void>(spdzalg, nParties);
    if (rlt < 0)
    {
        return rlt;
    }

    resolveOutput(spdzalg, true);

    return 0;
}

/**
 * @brief spdz computation for variance algorithm
 *
 * @param spdzalg
 * @param data_size
 * @param sum
 * @param square_sum
 * @param parties
 * @return int
 */
int runSPDZComputation(SPDZAlg *spdzalg, size_t data_size, double sum, double square_sum, int parties)
{
    if (0 == spdzalg->middle_prefix.size())
    {
        spdzalg->middle_prefix = "tmp";
    }
    if (1 == spdzalg->input_mode)
    {
        return runSPDZComputation_mem(spdzalg, data_size, sum, square_sum, parties);
    }

    setupFiles(spdzalg);
    initInputFiles(spdzalg, data_size, sum, square_sum);

    // run spdz machine
    runSPDZMachine<void>(spdzalg, parties);

    resolveOutput(spdzalg);

    return 0;
}

/**
 * @brief spdz computation for memory input
 *
 * @param spdzalg
 * @param parties
 * @param machine_type
 * @param store_rlt_vec
 * @return int
 */
// for two-party input
inline int runSPDZComputation_mem(SPDZAlg *spdzalg, int parties, int machine_type, bool store_rlt_vec)
{
    //  Init Input Data
    size_t thd_num = spdzalg->thread_num;
    size_t total_data_size = spdzalg->input.size();
    size_t data_size_per_thd = total_data_size / thd_num;
    if (total_data_size % thd_num)
    {
        LOG_ERROR("The total amount of data cannot be divided by thread num.");
        return -1;
    }

    int nValueSize = thd_num+1;
    vector<double> * value = new vector<double>[nValueSize];
    int nOffset = 0;
    if (thd_num > 1)
    {
        nOffset = 1;
    }
    for (size_t i=0; i<thd_num; ++i)
    {
        size_t nCopySize = data_size_per_thd;
        if (((i+1)*data_size_per_thd) > total_data_size)
        {
            nCopySize = total_data_size-(i*data_size_per_thd);
        }

        value[i+nOffset].resize(nCopySize, 0);
        memcpy(value[i+nOffset].data(), (char*)(spdzalg->input.data()+i*nCopySize), nCopySize*sizeof(double));
    }

    runSPDZMachine(spdzalg, parties, machine_type, value, nValueSize);

    delete[] value;

    resolveOutput(spdzalg, store_rlt_vec);

    return 0;
}

/**
 * @brief spdz computation(variance algorithm) for memory input
 *
 * @param spdzalg
 * @param data_size
 * @param sum
 * @param square_sum
 * @param parties
 * @return int
 */
inline int runSPDZComputation_mem(SPDZAlg *spdzalg, size_t data_size, double sum, double square_sum, int parties)
{
    vector<double> vec_input[1];
    vec_input[0].push_back(data_size);
    vec_input[0].push_back(sum);
    vec_input[0].push_back(square_sum);

    // calc threads start inside
    vector<double> * value = (vector<double> *)vec_input;
    runSPDZMachine(spdzalg, parties, EMACHINETYPE_NONE, value, 1);

    resolveOutput(spdzalg);

    return 0;
}

/**
 * @brief 2-party addition(a+b)
 * 
 * @param spdzalg 
 * @return int 
 */
int runAdd2(SPDZAlg *spdzalg)
{
    switch (spdzalg->thread_num)
    {
        case 1:
            spdzalg->alg_index_str = "add2";
            break;
        case 16:
            spdzalg->alg_index_str = "add2m16";
            break;
        case 32:
            spdzalg->alg_index_str = "add2m32";
            break;
        default:
            LOG_ERROR("Unsupported thread num: " << spdzalg->thread_num);
            return -1;
    }

    return runSPDZComputation(spdzalg, 2);
}

/**
 * @brief 2-party multiplication(a*b)
 *
 * @param spdzalg
 * @return int
 */
int runMul2(SPDZAlg *spdzalg)
{
    switch (spdzalg->thread_num)
    {
        case 1:
            spdzalg->alg_index_str = "mul2";
            break;
        case 16:
            spdzalg->alg_index_str = "mul2m16";
            break;
        case 32:
            spdzalg->alg_index_str = "mul2m32";
            break;
        default:
            LOG_ERROR("Unsupported thread num: " << spdzalg->thread_num);
            return -1;
    }

    return runSPDZComputation(spdzalg, 2);
}

/**
 * @brief 2-party comparison(a<b)
 *
 * @param spdzalg
 * @return int
 */
int runCmp2(SPDZAlg *spdzalg)
{
    switch (spdzalg->thread_num)
    {
        case 1:
            spdzalg->alg_index_str = "cmp2";
            break;
        case 16:
            spdzalg->alg_index_str = "cmp2m16";
            break;
        case 32:
            spdzalg->alg_index_str = "cmp2m32";
            break;
        default:
            LOG_ERROR("Unsupported thread num: " << spdzalg->thread_num);
            return -1;
    }

    return runSPDZComputation(spdzalg, 2);
}

/**
 * @brief 2-party median-value(Given two arrays, to find the median-value)
 *
 * @param spdzalg
 * @return int
 */
int runMid2(SPDZAlg *spdzalg)
{
    spdzalg->alg_index_str = "mid2";
    // the inputs of median algorithm should be ordered.
    std::sort(spdzalg->input.begin(), spdzalg->input.end());
    return runSPDZComputation(spdzalg, 2);
}

/**
 * @brief 2-party variance(Given two arrays, to compute the variance)
 *
 * @param spdzalg
 * @return int
 */
int runVar2(SPDZAlg *spdzalg)
{
    spdzalg->alg_index_str = "var2";
    size_t data_size = spdzalg->input.size();
    double sum = 0;
    double square_sum = 0;

    for (size_t i = 0; i < data_size; i++)
    {
        sum += (spdzalg->input)[i];
        square_sum += pow((spdzalg->input)[i], 2);
    }

    return runSPDZComputation(spdzalg, data_size, sum, square_sum, 2);
}

/**
 * @brief 2-party inner-product(Given two arrays, to compute the inner product)
 *
 * @param spdzalg
 * @return int
 */
int runInnerProd2(SPDZAlg *spdzalg)
{
    spdzalg->alg_index_str = "prodot2";
    return runSPDZComputation(spdzalg, 2);
}

/**
 * @brief 2-party inner-product for lr alg
 *
 * @param spdzalg
 * @param chunk
 * @return int
 */
int runProDot2(SPDZAlg *spdzalg, int chunk)
{
    spdzalg->alg_index_str = "lr-prodot2";
    spdzalg->thread_num = 1;
    return runProDotComputation(spdzalg, 2, chunk);
}

/**
 * @brief 3-party addition(a+b+c)
 *
 * @param spdzalg
 * @return int
 */
int runAdd3(SPDZAlg *spdzalg)
{
    switch (spdzalg->thread_num)
    {
        case 1:
            spdzalg->alg_index_str = "add3";
            break;
        case 16:
            spdzalg->alg_index_str = "add3m16";
            break;
        case 32:
            spdzalg->alg_index_str = "add3m32";
            break;
        default:
            LOG_ERROR("Unsupported thread num: " << spdzalg->thread_num);
            return -1;
    }

    return runSPDZComputation(spdzalg, 3);
}

/**
 * @brief 3-party multiplication(a*b*c)
 *
 * @param spdzalg
 * @return int
 */
int runMul3(SPDZAlg *spdzalg)
{
    switch (spdzalg->thread_num)
    {
        case 1:
            spdzalg->alg_index_str = "mul3";
            break;
        case 16:
            spdzalg->alg_index_str = "mul3m16";
            break;
        case 32:
            spdzalg->alg_index_str = "mul3m32";
            break;
        default:
            LOG_ERROR("Unsupported thread num: " << spdzalg->thread_num);
            return -1;
    }

    return runSPDZComputation(spdzalg, 3);
}

/**
 * @brief 3-party median-value(Given three arrays, to find the median-value)
 *
 * @param spdzalg
 * @return int
 */
int runMid3(SPDZAlg *spdzalg)
{
    spdzalg->alg_index_str = "mid3";
    std::sort(spdzalg->input.begin(), spdzalg->input.end());

    return runSPDZComputation(spdzalg, 3, EMACHINETYPE_SEMI);
}

/**
 * @brief 3-party variance(Given three arrays, to compute the variance)
 *
 * @param spdzalg
 * @return int
 */
int runVar3(SPDZAlg *spdzalg)
{
    spdzalg->alg_index_str = "var3";
    size_t data_size = spdzalg->input.size();
    double sum = 0;
    double square_sum = 0;

    for (size_t i = 0; i < data_size; i++)
    {
        sum += (spdzalg->input)[i];
        square_sum += pow((spdzalg->input)[i], 2);
    }

    return runSPDZComputation(spdzalg, data_size, sum, square_sum, 3);
}

#undef MAX_THREAD_NUM
#undef OUT_FILE_PATH
