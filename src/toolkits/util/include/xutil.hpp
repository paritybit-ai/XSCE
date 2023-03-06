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
 * @file xutil.hpp
 * @author Created by wumingzi. 2022:07:21,Thursday,00:18:36.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef UTIL_XUTIL_HPP
#define UTIL_XUTIL_HPP

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "xlog.h"

namespace xsce_ose
{
// #define DEBUG
//define delay micro
#define SleepMsec(x) usleep((x) << 10)
#define SleepSec(x) usleep((x) << 20)
#define LEFTFIX(x) std::dec << std::setiosflags(std::ios::fixed) << std::setprecision(x)
    typedef struct _ShmFilePara
    {
        std::string fn;
        std::size_t offset;
        std::size_t len;
        std::size_t offsetMask;
        uint64_t mask = (1UL << 63) - 1;
        uint32_t mode;
        unsigned char *buf = nullptr;
        bool debugFlag = true;

        //for psi alg use in vector string reading mode.
        std::vector<int64_t> *lineLocVec = nullptr;
        std::vector<std::string> *strVec = nullptr;
        std::vector<std::string> *lineStrVec = nullptr;
        uint64_t lineStart = 0; //the first valid row(0 means row[headLine], 1 means row[headline+1]
        uint64_t lineLen = 0;
        uint64_t colIdx = 0;   //column index to save
        uint64_t headLine = 0; //the number of headlines are omited.
        uint64_t maxStrLen = 0;

        // for slice data read  .Modified by wumingzi. 2022:04:17,Sunday,09:56:06.
        //read string data to different vector indexed by the first char accordingto charIdx.
        std::vector<std::vector<std::string> > *strVecSlice = nullptr;
        std::vector<std::vector<std::string> > *lineStrVecSlice = nullptr;
        std::vector<unsigned char> *charIdx;
        int thdIdx = 0;       // index of this thread
        int thdNum = 1;       //total thread number
        bool thdOver = false; //indicates thread is over or not.
        int cpuNum = 0;       //binding cpu numnber.
        //   .Modification over by wumingzi. 2022:04:17,Sunday,09:56:12.
    } ShmFilePara;

    template <typename T>
    void copyBufData(T *src, T *dst, int64_t len)
    {
        for (int64_t i = 0; i < len; i++)
        {
            dst[i] = src[i];
        }
    }

    template <typename datatype>
    int64_t saveVec2File(std::vector<datatype> &oriVec, std::string fn)
    {
        int64_t flushLen = 100000;

        uint64_t len = oriVec.size();
        std::ofstream outf;
        outf.open(fn.data());
        if (outf.fail())
        {
            LOG_ERROR("open file error at " << fn);
            return -1;
        }

        for (uint64_t i = 0; i < len; i++)
        {
            auto data = oriVec.at(i);
            outf << data << std::endl;
            if (0 == (i + 1) % flushLen)
                outf.flush();
        }

        outf.close();
        return len;
    }

    template <typename T>
    void show_vec_buf(T *vec, int col, std::string str)
    {
        auto len = col;
        std::cout << str << ":";
        for (int64_t i = 0; i < len; i++)
        {
            std::cout << LEFTFIX(8) << vec[i] << ",";
        }
        std::cout << "\n";
    }

    template <typename T>
    void show_mtx_buf(T **mtx, int row, int col, std::string str)
    {
        auto len = row;
        for (int64_t i = 0; i < len; i++)
        {
            show_vec_buf(mtx[i], col, str);
        }
    }

    void show_mtx(std::vector<std::vector<float> > &mtx);
    void show_vec(std::vector<float> &vec);

    void showBlk(int a, int b);
    void showBlk(int a, int b, std::stringstream &log);

    int getStrMtxFromCsvFile(std::string fn, std::vector<std::vector<std::string> > &rlt);
    int getRowStrVecFromCsvFile(std::string fn, std::vector<std::string> &rlt, int headLine);
    int64_t getStrColFromMtx(std::vector<std::string> &strPsi, std::vector<std::vector<std::string> > &strMtx, int64_t row, int headLine);
    int64_t getRowStrVecFromCsvFileWithCol(std::string fn, std::vector<std::string> &id_vec, std::vector<std::string> &data_vec, int headLine, int col);

    void mergeSortUIntBuf(uint32_t *arr, uint32_t dataByteLen, int64_t begin, int64_t end, std::vector<uint64_t> &index);
    void mergeUIntBuf(uint32_t *array, uint32_t dataByteLen, int64_t left, int64_t mid, int64_t right, std::vector<uint64_t> &index);
    int64_t binaryKeySearchUIntBuf(uint32_t *arr, uint32_t dataByteLen, int64_t n, uint32_t *key);

    //compare multiple uint32 memory data.
    bool cmpUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len);
    bool eqUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len);
    bool gtUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len);
    bool getUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len);
    bool ltUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len);
    bool letUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len);

    bool avgUInt128BufSimple(uint32_t *src1, uint32_t *src2, uint32_t *dst);

    //swap multiple uint 32 moemory data.
    void swapUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len);
    void copyUInt32Buf(uint32_t *dst, uint32_t *src, uint64_t len);

    //sort functions for multiple uint32  memory data.
    void mergeSortUIntBuf(uint32_t *arr, uint32_t dataByteLen, int64_t begin, int64_t end, std::vector<uint64_t> &index);
    void mergeUIntBuf(uint32_t *array, uint32_t dataByteLen, int64_t begin, int64_t mid, int64_t end, std::vector<uint64_t> &index);

    int64_t binaryKeySearchUIntBuf(uint32_t *arr, uint32_t dataByteLen, int64_t n, uint32_t *key);

    //mem tool
    double **allocateDoubleMtxFull(uint64_t row, uint64_t col);
    float **allocateFloatMtxFull(uint64_t row, uint64_t col);
    int **allocateIntMtxFull(uint64_t row, uint64_t col);
    unsigned char **allocateUCharMtxFull(uint64_t row, uint64_t col);
    uint64_t **allocateUInt64MtxFull(uint64_t row, uint64_t col);
    uint32_t **allocateUInt32MtxFull(uint64_t row, uint64_t col);

    double *allocateDoubleVec(uint64_t row);
    float *allocateFloatVec(uint64_t row);
    int *allocateIntVec(uint64_t row);
    unsigned char *allocateUCharVec(uint64_t row);
    uint32_t *allocateUInt32Vec(uint64_t row);
    uint64_t *allocateUInt64Vec(uint64_t row);

    void freeDoubleMtxFull(double **mtx, uint64_t row, uint64_t col);
    void freeFloatMtxFull(float **mtx, uint64_t row, uint64_t col);
    void freeIntMtxFull(int **mtx, uint64_t row, uint64_t col);
    void freeUCharMtxFull(unsigned char **mtx, uint64_t row, uint64_t col);
    void freeUInt64MtxFull(uint64_t **mtx, uint64_t row, uint64_t col);
    void freeUInt32MtxFull(uint32_t **mtx, uint64_t row, uint64_t col);

    void freeDoubleVec(double *mtx);
    void freeFloatVec(float *mtx);
    void freeIntVec(int *mtx);
    void freeUInt32Vec(uint32_t *mtx);
    void freeUInt64Vec(uint64_t *mtx);
    void freeUCharVec(unsigned char *mtx);

    // shm file read  .Modified by wumingzi. 2022:07:13,Wednesday,21:51:10.
    int64_t getStrVecFromCsvFileShm(std::string fn, std::vector<std::string> &rlt, int col);
    int64_t getStrDataFromFileShm(std::string fn, std::vector<std::string> &rlt, int64_t head, int64_t col, std::vector<std::string> *dataVecPtr);
    uint64_t getFileLineStrShm(std::string fn, std::vector<std::string> &rlt, int64_t fsize);
    std::size_t shmFileReadLineStr(ShmFilePara *para);
    uint64_t getPirDataFileShm(std::string fn, std::vector<std::string> &rlt, std::vector<std::string> &idVec, int col, int64_t fsize);

    uint64_t readFileSize(std::string fn);
    uint64_t findBufChar(unsigned char *buf, std::size_t len, unsigned char ch, int64_t index);
    std::string findBufStr(unsigned char *buf, std::size_t len, std::string &deli, int64_t index);
    std::string findBufStrNolf(unsigned char *buf, std::size_t len, std::string &deli, int64_t index);

    uint64_t getFileLineHeadShm(std::string fn, std::vector<int64_t> &vec, int64_t fsize, int64_t headLine = 0);
    void shareMemFileRead(std::string fn, int64_t base, int64_t offset, int64_t col, int64_t showLen);
} // namespace xsce_ose

#endif
