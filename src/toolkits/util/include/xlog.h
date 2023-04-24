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
 * @file xlog.h
 * @author Created by haiyu.  2022:07:21,Thursday,00:10:48.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include "replace.h"
#include <iostream>
#include <string>

#define PRINT_FILE_LINE() (                                                           \
    {                                                                                 \
        std::string strFile(__FILE__);                                                \
        size_t pos = strFile.find_last_of("/");                                       \
        if (pos != std::string::npos)                                                 \
        {                                                                             \
            strFile = strFile.substr(pos + 1);                                        \
        }                                                                             \
        strFile = std::string(" (") + strFile + ":" + std::to_string(__LINE__) + ")"; \
        strFile;                                                                      \
    })
#define CUR_TIME_LOG(level)                                   \
    time_t timep = 0;                                         \
    ::time(&timep);                                           \
    char szbuf[256] = {0};                                    \
    std::string strlog = std::string(ctime_r(&timep, szbuf)); \
    strlog.pop_back();                                        \
    strlog += level;
#define SYS_ERROR() strerror(errno) << "(" << errno << ")"

#ifndef LOG_BASE
// 计算 __VA_ARGS__ 参数个数,最大支持64个参数
#define VA_ARGS_COUNT_PRIVATE(\
         _0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, \
        _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
        _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
        _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
        _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
        _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
        _60, _61, _62, _63, _64, N, ...) N

#define VA_ARGS_COUNT(...) VA_ARGS_COUNT_PRIVATE(0, ##__VA_ARGS__,\
        64, 63, 62, 61, 60, \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
         9,  8,  7,  6,  5,  4,  3,  2,  1,  0)

#define LOG_BASE_ARGS1(level, level_str, loginfo)     \
                    {   \
                        CUR_TIME_LOG(level_str);                                         \
                        std::cout << strlog << loginfo << PRINT_FILE_LINE() << std::endl; \
                    }
#define LOG_BASE_ARGS2(level, level_str, logger, loginfo)     \
                    {   \
                        if (nullptr != logger)  \
                        {   \
                            std::ostringstream oss; oss << loginfo; \
                            logger->log(__FILE__, __LINE__, level, oss.str().c_str()); \
                        }   \
                        else \
                        {   \
                            LOG_BASE_ARGS1(level, level_str, loginfo); \
                        }   \
                    }
#define BASE_ARGS_(index) LOG_BASE_ARGS ## index
#define BASE_ARGS(index) BASE_ARGS_(index)
#define LOG_BASE(level, level_str, ...) { BASE_ARGS(VA_ARGS_COUNT(__VA_ARGS__))(level, level_str, __VA_ARGS__); }

enum Level {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
};

#endif

#define DEBUG_EX 1
#define LOG_FLAG 1

#ifndef LOG_DEBUG

#ifdef DEBUG_EX
#define LOG_DEBUG(...)  { LOG_BASE(DEBUG, " [DEBUG] ", __VA_ARGS__); }
#else
#define LOG_DEBUG(loginfo)
#endif

#endif

#ifdef LOG_FLAG

#ifndef LOG_INFO
#define LOG_INFO(...)  { LOG_BASE(INFO, " [INFO] ", __VA_ARGS__); }
#endif

#ifndef LOG_WARN
#define LOG_WARN(...) { LOG_BASE(WARN, " [WARN] ", __VA_ARGS__); }
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(...) { LOG_BASE(ERROR, " [ERROR] ", __VA_ARGS__); }
#endif

#ifndef LOG_FATAL
#define LOG_FATAL(...) { LOG_BASE(DPANIC, " [FATAL] ", __VA_ARGS__); }
#endif

#else
#define LOG_INFO(loginfo)
#define LOG_WARN(loginfo)
#define LOG_ERROR(loginfo)
#define LOG_FATAL(loginfo)
#endif
