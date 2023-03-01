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

#define DEBUG_EX 1
#define LOG_FLAG 1

#ifndef LOG_DEBUG

#ifdef DEBUG_EX
#define LOG_DEBUG(loginfo)                                                \
    {                                                                     \
        CUR_TIME_LOG(" [DEBUG] ");                                        \
        std::cout << strlog << loginfo << PRINT_FILE_LINE() << std::endl; \
    }
#else
#define LOG_DEBUG(loginfo)
#endif

#endif

#ifdef LOG_FLAG

#ifndef LOG_INFO
#define LOG_INFO(loginfo)                                                 \
    {                                                                     \
        CUR_TIME_LOG(" [INFO] ");                                         \
        std::cout << strlog << loginfo << PRINT_FILE_LINE() << std::endl; \
    }
#endif

#ifndef LOG_WARN
#define LOG_WARN(loginfo)                                                 \
    {                                                                     \
        CUR_TIME_LOG(" [WARN] ");                                         \
        std::cout << strlog << loginfo << PRINT_FILE_LINE() << std::endl; \
    }
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(loginfo)                                                \
    {                                                                     \
        CUR_TIME_LOG(" [ERROR] ");                                        \
        std::cout << strlog << loginfo << PRINT_FILE_LINE() << std::endl; \
    }
#endif

#ifndef LOG_FATAL
#define LOG_FATAL(loginfo)                                                \
    {                                                                     \
        CUR_TIME_LOG(" [FATAL] ");                                        \
        std::cout << strlog << loginfo << PRINT_FILE_LINE() << std::endl; \
    }
#endif

#else
#define LOG_INFO(loginfo)
#define LOG_WARN(loginfo)
#define LOG_ERROR(loginfo)
#define LOG_FATAL(loginfo)
#endif
