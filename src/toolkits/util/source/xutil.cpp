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
 * @file xutil.cpp
 * @author Created by wumingzi. 2022:07:21,Thursday,00:18:36.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "xutil.hpp"

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>

namespace xsce_ose
{
    void showBlk(int a, int b)
    {
        for (int i = 0; i < a; i++)
        {
            if (0 == b)
                LOG_INFO("");
            if (1 == b)
                LOG_INFO("........................................................................");
            if (2 == b)
                LOG_INFO("------------------------------------------------------------------------");
            if (3 == b)
                LOG_INFO("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        }
    }

    void showBlk(int a, int b, std::stringstream &log)
    {
        for (int i = 0; i < a; i++)
        {
            if (0 == b)
                log << "" << std::endl;
            if (1 == b)
                log << "........................................................................" << std::endl;
            if (2 == b)
                log << "------------------------------------------------------------------------" << std::endl;
            if (3 == b)
                log << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
        }
    }
    void show_mtx(std::vector<std::vector<float> > &mtx)
    {
        auto len = mtx.size();
        for (std::size_t i = 0; i < len; i++)
        {
            show_vec(mtx.at(i));
        }
    }
    void show_vec(std::vector<float> &vec)
    {
        auto len = vec.size();
        for (std::size_t i = 0; i < len; i++)
        {
            std::cout << LEFTFIX(8) << vec.at(i) << ",";
        }
        std::cout << "\n";
    }

    int getStrMtxFromCsvFile(std::string fn, std::vector<std::vector<std::string> > &rlt)
    {
        std::ifstream in;
        int n = 0;
        char pedSep = ',';

        std::string lineStr;
        rlt.resize(0);
        in.open(fn, std::ios::in); //read only mode
        if (in.fail())             //open file error
        {
            return 0;
        }
        else
        {
            while (getline(in, lineStr))
            {

                if (lineStr.length() < 1)
                    continue;

                // delete "\n" or "\r\n"
                while (lineStr.size() >= 1)
                {
                    if (lineStr.at(lineStr.size() - 1) == '\n' || lineStr.at(lineStr.size() - 1) == '\r')
                    {
                        lineStr.pop_back();
                    }
                    else
                    {
                        break;
                    }
                }

                // Save as a two-dimensional table structure
                std::stringstream ss(lineStr);
                std::string str;
                std::vector<std::string> lineArray;
                // seperate by ','
                while (getline(ss, str, pedSep))
                    lineArray.push_back(str);

                rlt.push_back(lineArray);

                n++;
            }
        }
        in.close();

        return rlt.size();
    }

    int64_t getStrColFromMtx(std::vector<std::string> &strPsi, std::vector<std::vector<std::string> > &strMtx, int64_t row, int headLine)
    {
        int rlt = 0;

        int64_t totalRow = strMtx.size();
        if (totalRow < 1)
        {
            LOG_ERROR("getStrColFromMtx input str mtx row is  error=" << totalRow);
            return rlt;
        }

        int64_t totalCol = strMtx.at(0).size();
        int64_t col = row;

        if (col >= totalCol || col < 0)
        {
            LOG_ERROR("getStrColFromMtx input col is error=" << col);
            return rlt;
        }

        if (headLine >= totalRow || headLine < 0)
        {
            LOG_ERROR("getStrColFromMtx input headLine is error=" << headLine);
            return rlt;
        }

        strPsi.resize(0);

        for (int64_t i = headLine; i < totalRow; i++)
        {
            if (col < (int64_t)strMtx.at(i).size())
            {
                strPsi.push_back(strMtx.at(i).at(col));
            }
        }

        rlt = strPsi.size();

        if (rlt < totalRow)
        {
            LOG_ERROR("getStrColFromMtx input file, some rows are invalid=" << totalRow - rlt);
        }

        return rlt;
    }

    int getRowStrVecFromCsvFile(std::string fn, std::vector<std::string> &rlt, int headLine)
    {
        std::ifstream in;
        size_t maxStrLen = 0;
        std::string pedSep = " ,\t";
        int rowCnt = 0;

        std::string lineStr;
        rlt.resize(0);
        in.open(fn, std::ios::in); //read only mode
        if (in.fail())             //open file error
        {
            return 0;
        }
        else
        {
            while (getline(in, lineStr))
            {
                if (lineStr.length() < 1)
                    continue;

                if (rowCnt < headLine)
                {
                    rowCnt++;
                    continue;
                }
                rlt.push_back(lineStr);

                if (lineStr.length() > maxStrLen)
                    maxStrLen = lineStr.length();
            }
        }
        in.close();

        return maxStrLen;
    }

    int64_t getRowStrVecFromCsvFileWithCol(std::string fn, std::vector<std::string> &id_vec, std::vector<std::string> &data_vec, int headLine, int col)
    {
        std::ifstream in;
        size_t maxStrLen = 0;
        // std::string pedSep = ",";
        char pedSep = ',';
        int rowCnt = 0;
        int64_t rlt = -1;

        if (col < 0 || headLine < 0)
        {
            LOG_ERROR("getRowStrVecFromCsvFileWithCol input col/headline is error=" << col << ",headLine=" << headLine);
            return rlt;
        }

        std::size_t col_index = (std::size_t)col;

        std::string lineStr;
        data_vec.resize(0);
        id_vec.resize(0);

        in.open(fn, std::ios::in); //read only mode
        if (in.fail())             //open file error
        {
            LOG_ERROR("getRowStrVecFromCsvFileWithCol open file error=" << fn);
            return rlt;
        }
        else
        {
            while (getline(in, lineStr))
            {
                if (lineStr.length() < 1)
                    continue;

                if (rowCnt < headLine)
                {
                    rowCnt++;
                    continue;
                }

                // 删除换行符 "\n" 或 "\r\n"
                while (lineStr.size() >= 1)
                {
                    if (lineStr.at(lineStr.size() - 1) == '\n' || lineStr.at(lineStr.size() - 1) == '\r')
                    {
                        lineStr.pop_back();
                    }
                    else
                    {
                        break;
                    }
                }

                //here to split string to vec
                // 存成二维表结构
                std::stringstream ss(lineStr);
                std::string str;
                std::vector<std::string> lineArray;
                // 按照逗号分隔
                while (getline(ss, str, pedSep))
                    lineArray.push_back(str);

                if (lineArray.size() > col_index)
                {

                    id_vec.push_back(lineArray.at(col_index));
                    data_vec.push_back(lineStr);

                    if (lineStr.length() > maxStrLen)
                        maxStrLen = lineStr.length();
                }
            }
        }
        in.close();

        return maxStrLen;
    }

    //sort functions for multiple uint32  memory data.   .Modified by wumingzi. 2021:07:20,Tuesday,14:33:41.
    void mergeSortUIntBuf(uint32_t *arr, uint32_t dataByteLen, int64_t begin, int64_t end, std::vector<uint64_t> &index)
    {

        if (begin >= end)
            return; // Returns recursivly

        auto mid = begin + (end - begin) / 2;
        mergeSortUIntBuf(arr, dataByteLen, begin, mid, index);
        mergeSortUIntBuf(arr, dataByteLen, mid + 1, end, index);
        mergeUIntBuf(arr, dataByteLen, begin, mid, end, index);
    }

    //sort multiple   uint32 memory data.
    void mergeUIntBuf(uint32_t *array, uint32_t dataByteLen, int64_t left, int64_t mid, int64_t right, std::vector<uint64_t> &index)
    {
        auto const subArrayOne = mid - left + 1;
        auto const subArrayTwo = right - mid;
        std::vector<uint64_t> leftIndex;
        std::vector<uint64_t> rightIndex;

        // Create temp arrays
        uint32_t *leftArray = allocateUInt32Vec(subArrayOne * dataByteLen);
        uint32_t *rightArray = allocateUInt32Vec(subArrayTwo * dataByteLen);

        // Copy data to temp arrays leftArray[] and rightArray[]
        for (auto i = 0; i < subArrayOne; i++)
        {
            copyUInt32Buf(&leftArray[i * dataByteLen], &array[(left + i) * dataByteLen], dataByteLen);
            leftIndex.push_back(index[left + i]);
        }
        for (auto j = 0; j < subArrayTwo; j++)
        {
            copyUInt32Buf(&rightArray[j * dataByteLen], &array[(mid + 1 + j) * dataByteLen], dataByteLen);
            rightIndex.push_back(index[mid + 1 + j]);
        }

        auto indexOfSubArrayOne = 0,       // Initial index of first sub-array
            indexOfSubArrayTwo = 0;        // Initial index of second sub-array
        int64_t indexOfMergedArray = left; // Initial index of merged array
                                           // Merge the temp arrays back into array[left..right]
        while (indexOfSubArrayOne < subArrayOne && indexOfSubArrayTwo < subArrayTwo)
        {
            if (letUInt32Buf(&leftArray[indexOfSubArrayOne * dataByteLen], &rightArray[indexOfSubArrayTwo * dataByteLen], dataByteLen))
            {
                copyUInt32Buf(&array[indexOfMergedArray * dataByteLen], &leftArray[indexOfSubArrayOne * dataByteLen], dataByteLen);
                // for index  .Modified by wumingzi/wumingzi. 2021:07:18,Sunday,21:44:10.
                index[indexOfMergedArray] = leftIndex[indexOfSubArrayOne];
                //   .Modification over by wumingzi/wumingzi. 2021:07:18,Sunday,21:47:50.

                indexOfSubArrayOne++;
            }
            else
            {
                copyUInt32Buf(&array[indexOfMergedArray * dataByteLen], &rightArray[indexOfSubArrayTwo * dataByteLen], dataByteLen);

                // for index  .Modified by wumingzi/wumingzi. 2021:07:18,Sunday,21:44:10.
                index[indexOfMergedArray] = rightIndex[indexOfSubArrayTwo];
                //   .Modification over by wumingzi/wumingzi. 2021:07:18,Sunday,21:47:28.

                indexOfSubArrayTwo++;
            }
            indexOfMergedArray++;
        }
        // Copy the remaining elements of
        // left[], if there are any
        while (indexOfSubArrayOne < subArrayOne)
        {
            copyUInt32Buf(&array[indexOfMergedArray * dataByteLen], &leftArray[indexOfSubArrayOne * dataByteLen], dataByteLen);
            // for index  .Modified by wumingzi/wumingzi. 2021:07:18,Sunday,21:44:10.
            index[indexOfMergedArray] = leftIndex[indexOfSubArrayOne];
            //   .Modification over by wumingzi/wumingzi. 2021:07:18,Sunday,21:47:10.

            indexOfSubArrayOne++;
            indexOfMergedArray++;
        }
        // Copy the remaining elements of
        // right[], if there are any
        while (indexOfSubArrayTwo < subArrayTwo)
        {
            copyUInt32Buf(&array[indexOfMergedArray * dataByteLen], &rightArray[indexOfSubArrayTwo * dataByteLen], dataByteLen);
            // for index  .Modified by wumingzi/wumingzi. 2021:07:18,Sunday,21:44:10.
            index[indexOfMergedArray] = rightIndex[indexOfSubArrayTwo];
            //   .Modification over by wumingzi/wumingzi. 2021:07:18,Sunday,21:46:46.

            indexOfSubArrayTwo++;
            indexOfMergedArray++;
        }

        freeUInt32Vec(leftArray);
        freeUInt32Vec(rightArray);
    }

    // multiple uint32 data binary search. n should be the total length of data itme. not n-1.  .Modified by wumingzi/wumingzi. 2021:07:20,Tuesday,16:31:09.
    int64_t binaryKeySearchUIntBuf(uint32_t *arr, uint32_t dataByteLen, int64_t n, uint32_t *key)
    {
        int64_t low = 0, high = n - 1;
        int64_t mid = 0;

        while (low <= high)
        {
            mid = (low + high) / 2;

            if (eqUInt32Buf(key, arr + mid * dataByteLen, dataByteLen))
                return mid;

            if (ltUInt32Buf(arr + mid * dataByteLen, key, dataByteLen))
                low = mid + 1;
            else
                high = mid - 1;
        }

        return -1;
    }

    bool cmpUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len)
    {
        if (nullptr == src || nullptr == dst)
            return false;

        for (uint64_t i = 0; i < len; i++)
        {
            if (*(src + i) != *(dst + i))
                return false;
        }

        return true;
    }

    //  equal compare .Modified by /wumingzi. 2021:07:20,Tuesday,14:07:14.
    bool eqUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len)
    {
        if (nullptr == src || nullptr == dst)
            return false;

        for (uint64_t i = 0; i < len; i++)
        {
            if (*(src + i) != *(dst + i))
                return false;
        }

        return true;
    }

    //  greater  than compare in big-edian mode.  Modified by /wumingzi. 2021:07:20,Tuesday,14:07:14.
    bool gtUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len)
    {
        if (nullptr == src || nullptr == dst)
            return false;

        for (uint64_t i = len - 1; i >= 0; i--)
        {
            if (*(src + i) > *(dst + i))
                return true;
            else if (*(src + i) < *(dst + i))
                return false;
        }

        return false;
    }

    //  greater  or equal than compare in big-edian mode.  Modified by /wumingzi. 2021:07:20,Tuesday,14:07:14.
    bool getUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len)
    {
        if (nullptr == src || nullptr == dst)
            return false;

        for (uint64_t i = len - 1; i >= 0; i--)
        {
            if (*(src + i) > *(dst + i))
                return true;
            else if (*(src + i) < *(dst + i))
                return false;
        }

        return true;
    }

    bool avgUInt128BufSimple(uint32_t *src1, uint32_t *src2, uint32_t *dst)
    {
        bool rlt = false;
        if (nullptr == src1 || nullptr == src2 || nullptr == dst)
            return rlt;

        uint64_t s1_high = 0;
        uint64_t s1_low = 0;
        uint64_t s2_high = 0;
        uint64_t s2_low = 0;
        uint64_t avg_high = 0;
        uint64_t avg_low = 0;
        uint64_t tmp = 0;

        uint64_t *s1 = (uint64_t *)src1;
        uint64_t *s2 = (uint64_t *)src2;
        uint64_t *d = (uint64_t *)dst;

        s1_high = s1[1];
        s1_low = s1[0];
        s2_high = s2[1];
        s2_low = s2[0];

        if (s1_high < s2_high)
        {
            return rlt;
        }
        else if (s1_high == s2_high && s1_low < s2_low)
        {
            return rlt;
        }

        uint64_t gap = s1_high - s2_high;
        uint64_t gap2 = s1_low - s2_low;
        uint64_t maxU64 = std::numeric_limits<uint64_t>::max();

        if (0 == gap)
        {
            avg_high = s1_high;
            //gap2 is supposed to be positive number here.
            avg_low = s2_low + gap2 / 2;
        }
        else if (1 == gap)
        {
            //gap2 maybe positive or negtive
            if (s1_low <= s2_low)
            {
                tmp = maxU64 - s2_low;
                avg_low = tmp + s1_low;
                avg_high = s2_high;
            }
            else //gap2 is positive here.
            {
                avg_low = s2_low + gap2 / 2;
                avg_high = s1_high;
            }
        }
        else
        {
            avg_high = s2_high + gap / 2;
            avg_low = 0;
        }

        d[0] = avg_low;
        d[1] = avg_high;

        return true;
    }

    //  less than compare in big-edian mode .Modified by /wumingzi. 2021:07:20,Tuesday,14:07:14.
    bool ltUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len)
    {
        if (nullptr == src || nullptr == dst)
            return false;

        for (uint64_t i = len - 1; i >= 0; i--)
        {
            if (*(src + i) < *(dst + i))
                return true;
            else if (*(src + i) > *(dst + i))
                return false;
        }

        return false;
    }

    //  less or equal than compare in big-edian mode .Modified by /wumingzi. 2021:07:20,Tuesday,14:07:14.
    bool letUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len)
    {
        if (nullptr == src || nullptr == dst)
            return false;

        for (uint64_t i = len - 1; i >= 0; i--)
        {
            if (*(src + i) < *(dst + i))
                return true;
            else if (*(src + i) > *(dst + i))
                return false;
        }

        return true;
    }

    // swap uint 32 data for consecutive len itmes  .Modified by /wumingzi. 2021:07:20,Tuesday,14:25:14.
    void swapUInt32Buf(uint32_t *src, uint32_t *dst, uint64_t len)
    {
        for (uint64_t i = 0; i < len; i++)
        {
            src[i] ^= dst[i];
            dst[i] ^= src[i];
            src[i] ^= dst[i];
        }
    }

    // swap uint 32 data for consecutive len itmes  .Modified by /wumingzi. 2021:07:20,Tuesday,14:25:14.
    void copyUInt32Buf(uint32_t *dst, uint32_t *src, uint64_t len)
    {
        for (uint64_t i = 0; i < len; i++)
        {
            dst[i] = src[i];
        }
    }

    // mem tool  .Modified by wumingzi. 2022:05:17,Tuesday,15:04:28.
    double *allocateDoubleVec(uint64_t row)
    {
        double *mtx;

        mtx = (double *)calloc((uint64_t)row + 1, sizeof(double));
        if (!mtx)
        {
            return nullptr;
        }

        mtx[row] = 0;
        return mtx;
    }

    float *allocateFloatVec(uint64_t row)
    {
        float *mtx;

        mtx = (float *)calloc((uint64_t)row + 1, sizeof(float));
        if (!mtx)
        {
            return nullptr;
        }

        mtx[row] = 0;
        return mtx;
    }

    int *allocateIntVec(uint64_t row)
    {
        int *mtx;

        mtx = (int *)calloc((uint64_t)row + 1, sizeof(int));
        if (!mtx)
        {
            return nullptr;
        }

        mtx[row] = 0;
        return mtx;
    }

    unsigned char *allocateUCharVec(uint64_t row)
    {
        unsigned char *mtx;

        mtx = (unsigned char *)calloc((uint64_t)row + 1, sizeof(unsigned char));
        if (!mtx)
        {

            return nullptr;
        }

        mtx[row] = 0;
        return mtx;
    }

    uint32_t *allocateUInt32Vec(uint64_t row)
    {
        uint32_t *mtx;

        mtx = (uint32_t *)calloc((uint64_t)row + 1, sizeof(uint32_t));
        if (!mtx)
        {
            return nullptr;
        }

        mtx[row] = 0;
        return mtx;
    }

    uint64_t *allocateUInt64Vec(uint64_t row)
    {
        uint64_t *mtx;

        mtx = (uint64_t *)calloc((uint64_t)row + 1, sizeof(uint64_t));
        if (!mtx)
        {
            return nullptr;
        }

        mtx[row] = 0;
        return mtx;
    }

    double **allocateDoubleMtxFull(uint64_t row, uint64_t col)
    {
        double **mtx;
        uint64_t i;

        mtx = (double **)calloc((uint64_t)row + 1, sizeof(double *));
        if (mtx == NULL)
        {
            return nullptr;
        }

        for (i = 0; i < row; i++)
        {
            mtx[i] = (double *)calloc((uint64_t)col, sizeof(double));
            if (!mtx[i])
            {
                return nullptr;
            }
        }
        mtx[row] = nullptr;

        return (mtx);
    }

    float **allocateFloatMtxFull(uint64_t row, uint64_t col)
    {
        float **mtx;
        uint64_t i;

        mtx = (float **)calloc((uint64_t)row + 1, sizeof(float *));
        if (mtx == NULL)
        {
            return nullptr;
        }

        for (i = 0; i < row; i++)
        {
            mtx[i] = (float *)calloc(col, sizeof(float));
            if (!mtx[i])
            {
                return nullptr;
            }
        }
        mtx[row] = nullptr;

        return (mtx);
    }

    unsigned char **allocateUCharMtxFull(uint64_t row, uint64_t col)
    {
        unsigned char **mtx;
        uint64_t i;

        mtx = (unsigned char **)calloc((uint64_t)row + 1, sizeof(unsigned char *));
        if (mtx == NULL)
        {
            return nullptr;
        }

        for (i = 0; i < row; i++)
        {
            mtx[i] = (unsigned char *)calloc(col, sizeof(unsigned char));
            if (!mtx[i])
            {
                return nullptr;
            }
        }
        mtx[row] = nullptr;
        return (mtx);
    }

    int **allocateIntMtxFull(uint64_t row, uint64_t col)
    {
        int **mtx;
        uint64_t i;

        mtx = (int **)calloc((uint64_t)row + 1, sizeof(int *));
        if (mtx == NULL)
        {
            return nullptr;
        }

        for (i = 0; i < row; i++)
        {
            mtx[i] = (int *)calloc(col, sizeof(int));
            if (!mtx[i])
            {
                return nullptr;
            }
        }
        mtx[row] = NULL;

        return (mtx);
    }

    uint64_t **allocateUInt64MtxFull(uint64_t row, uint64_t col)
    {
        uint64_t **mtx;
        uint64_t i;

        mtx = (uint64_t **)calloc((uint64_t)row + 1, sizeof(uint64_t *));
        if (mtx == NULL)
        {
            return nullptr;
        }

        for (i = 0; i < row; i++)
        {
            mtx[i] = (uint64_t *)calloc(col, sizeof(uint64_t));
            if (!mtx[i])
            {
                return nullptr;
            }
        }
        mtx[row] = NULL;

        return (mtx);
    }

    uint32_t **allocateUInt32MtxFull(uint64_t row, uint64_t col)
    {
        uint32_t **mtx;
        uint64_t i;

        mtx = (uint32_t **)calloc((uint64_t)row + 1, sizeof(uint32_t *));
        if (mtx == NULL)
        {
            return nullptr;
        }

        for (i = 0; i < row; i++)
        {
            mtx[i] = (uint32_t *)calloc(col, sizeof(uint32_t));
            if (!mtx[i])
            {
                return nullptr;
            }
        }
        mtx[row] = NULL;

        return (mtx);
    }

    //free memory buffer.
    void freeDoubleMtxFull(double **mtx, uint64_t row, uint64_t col)
    {
        for (uint64_t i = 0; i < row + 1; i++)
        {
            if (mtx[i])
            {
                freeDoubleVec(mtx[i]);
                mtx[i] = NULL;
            }
        }

        free(mtx);
    }

    void freeFloatMtxFull(float **mtx, uint64_t row, uint64_t col)
    {
        for (uint64_t i = 0; i < row + 1; i++)
        {
            if (mtx[i])
            {
                freeFloatVec(mtx[i]);
                mtx[i] = NULL;
            }
        }

        free(mtx);
    }

    void freeIntMtxFull(int **mtx, uint64_t row, uint64_t col)
    {
        for (uint64_t i = 0; i < row + 1; i++)
        {
            if (mtx[i])
            {
                freeIntVec(mtx[i]);
                mtx[i] = NULL;
            }
        }

        free(mtx);
    }

    void freeUCharMtxFull(unsigned char **mtx, uint64_t row, uint64_t col)
    {
        for (uint64_t i = 0; i < row + 1; i++)
        {
            if (mtx[i])
            {
                freeUCharVec(mtx[i]);
                mtx[i] = NULL;
            }
        }

        free(mtx);
    }

    void freeUInt64MtxFull(uint64_t **mtx, uint64_t row, uint64_t col)
    {
        for (uint64_t i = 0; i < row + 1; i++)
        {
            if (mtx[i])
            {
                freeUInt64Vec(mtx[i]);
                mtx[i] = NULL;
            }
        }

        free(mtx);
    }

    void freeUInt32MtxFull(uint32_t **mtx, uint64_t row, uint64_t col)
    {
        for (uint64_t i = 0; i < row + 1; i++)
        {
            if (mtx[i])
            {
                freeUInt32Vec(mtx[i]);
                mtx[i] = NULL;
            }
        }

        free(mtx);
    }

    void freeUInt32Vec(uint32_t *mtx)
    {
        if (mtx)
            free(mtx);
    }

    void freeIntVec(int *mtx)
    {
        if (mtx)
            free(mtx);
    }

    void freeUInt64Vec(uint64_t *mtx)
    {
        if (mtx)
            free(mtx);
    }

    void freeUCharVec(unsigned char *mtx)
    {
        if (mtx)
            free(mtx);
    }

    void freeFloatVec(float *mtx)
    {
        if (mtx)
            free(mtx);
    }

    void freeDoubleVec(double *mtx)
    {
        if (mtx)
            free(mtx);
        return;
    }

    //shm huge file read   .Modified by wumingzi. 2022:07:13,Wednesday,21:47:29.
    int64_t getStrVecFromFileShm(std::string fn, std::vector<std::string> &rlt)
    {
        if (fn.length() < 1)
        {
            LOG_ERROR("input file error.fn=" << fn);
            return -1;
        }

        //read line feedback loc first.
        std::string datafn = fn;
        uint64_t showCnt = 3;
        int64_t readFileSize = 0; //0 means readl entile file data.

        //here call huge file read function in abyAlg.h
        uint64_t readRlt = getFileLineStrShm(datafn, rlt, readFileSize);

        LOG_INFO("read huge file ,valid line=" << readRlt);
        for (uint64_t i = 0; i < readRlt && i < showCnt; i++)
        {
            LOG_INFO(i << "=" << rlt[i] << "--");
        }

        return rlt.size();
    }

    std::size_t shmFileReadLineStr(ShmFilePara *para)
    {
        using namespace boost::interprocess;

        std::size_t readLen = 0;
        std::vector<std::string> *lineStrVecPtr = para->lineStrVec;
        std::vector<std::string> *idStrVecPtr = para->strVec;

        std::vector<int64_t> *lineLocVecPtr = para->lineLocVec;
        bool lineStrFlag = false; //no need to save whole line string in default.
        uint64_t showLineNum = 1000000;
        uint64_t showCnt = 3;
        int64_t maxStrLen = para->maxStrLen;

        if (nullptr != lineStrVecPtr)
        {
            lineStrFlag = true;
            LOG_INFO("need to save each line str.");
        }
        else
        {
            LOG_INFO("no need to save each line str.");
            lineStrFlag = false;
        }

        std::string fn;
        if (para->fn.empty())
        {
            LOG_ERROR("shmFileReadStr input file name error.");
            return readLen;
        }
        else
        {
            fn = para->fn;
        }

        if (nullptr == lineLocVecPtr)
        {
            LOG_ERROR("shmFileReadStr input line loc vec ptr  null");
            return readLen;
        }

        uint64_t totalLineNum = para->lineLocVec->size();

        uint64_t lineStart = para->lineStart;
        uint64_t lineLen = para->lineLen;
        uint64_t lineEnd = lineStart + lineLen;
        uint64_t col = para->colIdx;

        if (lineEnd >= totalLineNum)
        {
            LOG_ERROR("shmFileReadStr input line number error. lineEnd=" << lineEnd);
            return readLen;
        }

        if (lineStrFlag)
        {
            uint64_t lineStrVecSize = para->lineStrVec->size();
            if (lineStrVecSize != lineLen)
            {
                LOG_ERROR("shmFileReadStr input lineStrVec number error=" << lineStrVecSize);
                return readLen;
            }
        }

        std::size_t fileBase = (std::size_t)para->lineLocVec->at(lineStart) + 1; //1 means next char from feedline loc.
        std::size_t fileEnd = (std::size_t)para->lineLocVec->at(lineEnd);
        std::size_t fileLen = fileEnd - fileBase;

        if (para->debugFlag)
        {
            LOG_DEBUG("shmFileReadStr. line base=" << lineStart << ",line len=" << lineLen << ",lineEnd=" << lineEnd << ",col Idx=" << col);
            LOG_DEBUG("mapped_region read file base=" << fileBase << ",file len=" << fileLen << ", fileEnd=" << fileEnd);
        }

        const boost::interprocess::mode_t mode = boost::interprocess::read_only;
        boost::interprocess::file_mapping fm(fn.c_str(), mode);
        boost::interprocess::mapped_region region(fm, boost::interprocess::read_only, (std::size_t)fileBase, fileLen);

        const char *bytes = reinterpret_cast<const char *>(region.get_address());
        std::size_t size = region.get_size();
        readLen = size;
        para->buf = (unsigned char *)bytes;

        int64_t lineNum = 0;
        int64_t curBase = 0;
        int64_t nextBase = 0;
        int64_t curLen = 0;
        int64_t offset = 0;

        std::string deli = ",\t";

        for (uint64_t i = 0; i < lineLen; i++)
        {
            std::string str;

            curBase = (std::size_t)para->lineLocVec->at(lineStart + i);

            if (i <= lineLen - 1)
            {
                nextBase = (std::size_t)para->lineLocVec->at(lineStart + i + 1);
            }
            else
            {
                nextBase = curBase;
            }
            curLen = nextBase - curBase;
            if (curLen > maxStrLen)
                maxStrLen = curLen;

            if (nullptr != idStrVecPtr)
            {
                str = findBufStr((unsigned char *)(bytes + offset), curLen, deli, col);
            }

            //here copy entilre line string.
            if (lineStrFlag && curLen > 1)
            {
                std::string lineStr((const char *)&bytes[offset], curLen - 1); //curLen - 1 means removing last linefeed char.
                if (i < showCnt)
                {
                    lineStrVecPtr->at(i) = lineStr;
                }
                if (nullptr != idStrVecPtr)
                {
                    idStrVecPtr->at(i) = str;
                }
            }

            lineNum++;

            offset += curLen;

            if (0 == (i + 1) % showLineNum)
            {
                LOG_INFO("read line number=" << i + 1);
            }
        }

        readLen = lineNum;
        para->maxStrLen = maxStrLen;

        return readLen;
    }

    uint64_t getFileLineHeadShm(std::string fn, std::vector<int64_t> &vec, int64_t fsize, int64_t headLine)
    {
        LOG_INFO("mapped_region:");
        using namespace boost::interprocess;

        const boost::interprocess::mode_t mode = boost::interprocess::read_only;
        boost::interprocess::file_mapping fm(fn.c_str(), mode);

        int64_t head = headLine;
        if (head < 0)
        {
            LOG_ERROR("input headline is invalid = " << head);
            return 0;
        }

        std::size_t fileSzie;
        if (0 == fsize)
        {
            fileSzie = (std::size_t)readFileSize(fn);
        }
        else
        {
            fileSzie = fsize;
        }

        int64_t base = 0;
        int64_t offset = 30; //read 1GB each time.
        int64_t loop = 0;

        std::size_t fileBase = (std::size_t)1 << base;
        std::size_t fileLen = (std::size_t)1 << offset;

        //first line start with 0 index.
        vec.push_back(-1);
        char linefed = '\n';
        while (fileBase < fileSzie)
        {
            if (fileBase + fileLen > fileSzie)
            {
                // fileBase = 0;
                fileLen = fileSzie - fileBase;
            }

            //begin to read file
            //Map the whole file with read-write permissions in this process
            LOG_INFO(loop++ << ":fileBase=" << (std::size_t)fileBase << ",fileLen=" << fileLen << ",fileSzie=" << fileSzie);
            boost::interprocess::mapped_region region(fm, boost::interprocess::read_only, (std::size_t)fileBase, fileLen);
            const char *bytes = reinterpret_cast<const char *>(region.get_address());
            std::size_t size = region.get_size();

            LOG_INFO("read data map over, begin to show data...size=" << size);
            int64_t cnt = 0;
            for (size_t i = 0; i < size; i++)
            {
                if (linefed == (unsigned char)bytes[i])
                {
                    if (cnt >= head) //valid row data.
                    {
                        vec.push_back(fileBase + i);
                    }
                    else
                    {
                        //invalid row. vec[0] hold linefeed loc of the last invalid row.
                        vec[0] = fileBase + i;
                    }
                    cnt++;
                }
            }

            //loop last
            fileBase += fileLen;
        }

        return fileSzie;
    }

    uint64_t getPirDataFileShm(std::string fn, std::vector<std::string> &rlt, std::vector<std::string> &idVec, int col, int64_t fsize)
    {
        int64_t readRlt = -1;
        std::string pedSep = " ,\t";

        if (fn.length() < 1)
        {
            LOG_ERROR("getPirDataFileShm input file error.fn=" << fn);
            return readRlt;
        }

        //read line feedback loc first.
        std::string datafn = fn;
        std::vector<int64_t> rltVec;
        int64_t row = 0;
        getFileLineHeadShm(datafn, rltVec, 0, row);

        uint64_t showCnt = 3;
        uint64_t vecLen = rltVec.size();
        LOG_INFO("getPirDataFileShm   show shm read file string. read line=" << vecLen << ",id col=" << col);
        for (uint64_t i = 0; i < vecLen && i < showCnt; i++)
        {
            LOG_INFO("line " << i << "deli loc=" << rltVec[i]);
        }

        //here read file string each line.
        ShmFilePara para;
        para.fn = datafn;
        LOG_INFO("origin vecLen = " << vecLen);
        vecLen = vecLen - 1;
        LOG_INFO("final vecLen = " << vecLen);

        para.lineLocVec = &rltVec;
        rlt.resize(vecLen);
        idVec.resize(vecLen);

        para.lineStart = 0;
        para.lineLen = vecLen;
        para.colIdx = col;
        if (para.colIdx < 0)
        {
            para.colIdx = 0;
        }

        para.lineStrVec = &rlt; //no need to save each line
        para.strVec = &idVec;
        para.debugFlag = true;

        readRlt = shmFileReadLineStr(&para);

        LOG_INFO("getPirDataFileShm read huge file ,valid line=" << readRlt);

        return rlt.size();
    }

    uint64_t getFileLineStrShm(std::string fn, std::vector<std::string> &rlt, int64_t fsize)
    {
        int64_t readRlt = -1;
        std::string pedSep = " ,\t";

        if (fn.length() < 1)
        {
            LOG_ERROR("input file error.fn=" << fn);
            return readRlt;
        }

        //read line feedback loc first.
        std::string datafn = fn;
        std::vector<int64_t> rltVec;
        int64_t row = 0;
        getFileLineHeadShm(datafn, rltVec, 0, row);

        uint64_t showCnt = 3;
        uint64_t vecLen = rltVec.size();
        LOG_INFO("show shm read file string. read line=" << vecLen);
        for (uint64_t i = 0; i < vecLen && i < showCnt; i++)
        {
            LOG_INFO("line " << i << "deli loc=" << rltVec[i]);
        }

        //here read file string each line.
        ShmFilePara para;
        para.fn = datafn;
        LOG_INFO("origin vecLen = " << vecLen);
        vecLen = vecLen - 1;
        LOG_INFO("final vecLen = " << vecLen);

        para.lineLocVec = &rltVec;
        rlt.resize(vecLen);

        para.lineStart = 0;
        para.lineLen = vecLen;
        para.colIdx = 0;
        para.lineStrVec = &rlt; //no need to save each line
        para.debugFlag = true;

        readRlt = shmFileReadLineStr(&para);

        LOG_INFO("read huge file ,valid line=" << readRlt);

        return rlt.size();
    }

    uint64_t getFileLineShm(std::string fn, std::vector<int64_t> &vec, int64_t fsize)
    {
        using namespace boost::interprocess;

        LOG_INFO("mapped_region:");
        const boost::interprocess::mode_t mode = boost::interprocess::read_only;
        boost::interprocess::file_mapping fm(fn.c_str(), mode);

        std::size_t fileSzie;

        if (0 == fsize)
        {
            fileSzie = (std::size_t)readFileSize(fn);
        }
        else
        {
            fileSzie = fsize;
        }

        int64_t base = 0;
        int64_t offset = 30; //read 1GB each time.
        int64_t loop = 0;

        std::size_t fileBase = (std::size_t)1 << base;
        std::size_t fileLen = (std::size_t)1 << offset;

        //first line start with 0 index.
        vec.push_back(-1);
        char linefed = '\n';

        while (fileBase < fileSzie)
        {

            if (fileBase + fileLen > fileSzie)
            {
                // fileBase = 0;
                fileLen = fileSzie - fileBase;
            }

            //begin to read file
            //Map the whole file with read-write permissions in this process
            LOG_INFO(loop++ << ":fileBase=" << (std::size_t)fileBase << ",fileLen=" << fileLen << ",fileSzie=" << fileSzie);
            boost::interprocess::mapped_region region(fm, boost::interprocess::read_only, (std::size_t)fileBase, fileLen);

            const char *bytes = reinterpret_cast<const char *>(region.get_address());
            std::size_t size = region.get_size();
            LOG_INFO("read data map over, begin to show data...size=" << size);

            for (size_t i = 0; i < size; i++)
            {
                if (linefed == (unsigned char)bytes[i])
                {
                    vec.push_back(fileBase + i);
                }
            }

            //loop last
            fileBase += fileLen;
        }

        return fileSzie;
    }

    void shareMemFileRead(std::string fn, int64_t base, int64_t offset, int64_t col, int64_t showLen)
    {
        using namespace boost::interprocess;

        LOG_INFO("mapped_region:");
        const boost::interprocess::mode_t mode = boost::interprocess::read_only;
        boost::interprocess::file_mapping fm(fn.c_str(), mode);

        std::size_t fileSzie = (std::size_t)readFileSize(fn);
        std::size_t fileBase = (std::size_t)1 << base;
        std::size_t fileLen = (std::size_t)1 << offset;
        if (fileBase + fileLen > fileSzie)
        {
            fileBase = 0;
            fileLen = 1024 * 1024;
        }

        //Map the whole file with read-write permissions in this process
        LOG_INFO("fileBase=" << (std::size_t)fileBase << ",fileLen=" << fileLen << ",fileSzie=" << fileSzie);
        boost::interprocess::mapped_region region(fm, boost::interprocess::read_only, (std::size_t)fileBase, fileLen);

        const char *bytes = reinterpret_cast<const char *>(region.get_address());
        std::size_t size = region.get_size();

        LOG_INFO("read data over, begin to show data...size=" << size);

        int64_t lineLen = 0;
        size_t cnt = 0;
        unsigned char c;
        int64_t lineNum = 0;

        std::stringstream log;
        while ('\n' != (c = (unsigned char)bytes[cnt++]))
        {
            if (cnt >= size)
                break;
            lineNum++;
            log << c;
        }
        LOG_INFO(log.str())

        LOG_INFO("line num is:" << lineNum);

        std::stringstream log1;
        for (int64_t i = 0; i < col;)
        {
            if (cnt >= size)
                break;
            unsigned char c = (unsigned char)bytes[cnt++];
            if ('\n' == c)
            {
                log1 << c;
                i++;
                lineLen = 0;
                continue;
            }
            else
            {
                if (lineLen < showLen)
                {
                    log1 << c;
                    lineLen++;
                }
            }
        }
        LOG_INFO(log1.str())
    }

    uint64_t readFileSize(std::string fn)
    {
        uint64_t size = 0;

        std::ifstream infile;

        infile.open(fn); // open input file
        if (!infile.is_open())
        {
            LOG_ERROR("readFile open error:" << fn);
            return size;
        }

        infile.seekg(0, std::ios::end); // go to the end
        size = infile.tellg();          // report location (this is the length)

        return size;
    }

    //   .Modified by wumingzi/wumingzi. 2022:05:07,Saturday,17:45:20.
    std::string findBufStrNolf(unsigned char *buf, std::size_t len, std::string &deli, int64_t index)
    {
        std::string str;
        std::string emptyStr = "";
        int64_t number = 0;
        unsigned char c;
        int deliLen = deli.length();
        bool newStr = true;
        bool validChar = false;
        unsigned char linefeed = '\n';

        for (std::size_t i = 0; i < len; i++)
        {
            c = (unsigned char)buf[i];
            validChar = true;
            for (int64_t j = 0; j < deliLen; j++)
            {
                if (c == deli[j] || c == linefeed)
                {
                    validChar = false;
                    break;
                }
            }

            if (validChar)
            {
                if (!newStr)
                {
                    newStr = true;
                    str = "";
                }
                str.push_back(c);
            }
            else
            {
                if (newStr) //new str is over.
                {
                    if (index == number)
                    {
                        return str;
                    }
                    number++;
                    newStr = false;
                }
            }
        }

        if (index == number)
            return str;
        else
            return emptyStr;
    }
    //   .Modification over by wumingzi/wumingzi. 2022:05:07,Saturday,17:45:34.

    //get the index string seperated by deli char in char buf.
    std::string findBufStr(unsigned char *buf, std::size_t len, std::string &deli, int64_t index)
    {
        std::string str;
        std::string emptyStr = "";
        int64_t number = 0;
        unsigned char c;
        int deliLen = deli.length();
        bool newStr = true;
        bool validChar = false;

        for (std::size_t i = 0; i < len; i++)
        {
            c = (unsigned char)buf[i];
            validChar = true;
            for (int64_t j = 0; j < deliLen; j++)
            {
                if (c == deli[j])
                {
                    validChar = false;
                    break;
                }
            }

            if (validChar)
            {
                if (!newStr)
                {
                    newStr = true;
                    str = "";
                }
                str.push_back(c);
            }
            else
            {
                if (newStr) //new str is over.
                {
                    if (index == number)
                    {
                        return str;
                    }
                    number++;
                    newStr = false;
                }
            }
        }

        if (index == number)
            return str;
        else
            return emptyStr;
    }

    //find the index ch char in buf.
    uint64_t findBufChar(unsigned char *buf, std::size_t len, unsigned char ch, int64_t index)
    {
        uint64_t rlt = -1;
        int64_t number = 0;

        for (std::size_t i = 0; i < len; i++)
        {
            if (ch == (unsigned char)buf[i])
            {
                rlt = (uint64_t)i;
                if (number == index)
                {
                    break;
                }
                else
                {
                    number++;
                }
            }
        }

        return rlt;
    }
} // namespace xsce_ose
