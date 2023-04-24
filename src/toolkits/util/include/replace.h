#pragma once

#ifdef XSCE_BUILD

#define LOG_LOGGER
#include "xlog/include/loginterface.h"

#else

struct Logger {
    void log(...){
        
    }
};

#endif

