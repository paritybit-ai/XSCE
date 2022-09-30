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
 * @file machinerunner.cpp
 * @author Created by chonglou. 2022:05:28
 * @brief 
 * @version 
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "machinerunner.h"

#include "toolkits/util/include/xlog.h"

#include "MP-SPDZ/Machines/SPDZ.hpp"
#include "MP-SPDZ/Math/gfp.hpp"
#include "MP-SPDZ/GC/TinierSecret.h"
#include "MP-SPDZ/Processor/FieldMachine.hpp"
#include "MP-SPDZ/Protocols/SemiShare.h"
#include "MP-SPDZ/Tools/SwitchableOutput.h"
#include "MP-SPDZ/GC/SemiPrep.h"
#include "MP-SPDZ/Machines/Semi.hpp"
#include "MP-SPDZ/GC/ShareSecret.hpp"
#include "MP-SPDZ/GC/TinyPrep.hpp"
#include "MP-SPDZ/GC/PersonalPrep.hpp"
#include "MP-SPDZ/GC/TinierSharePrep.hpp"
#include "MP-SPDZ/Machines/ShamirMachine.hpp"
#include "MP-SPDZ/Protocols/ShamirShare.h"

/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @param machine_type 
 * @param meminput_ctxt 
 * @return int 
 */
int MachineRunner::Run(int argc, const char* argv[], MachineType machine_type, MemInput_Context& meminput_ctxt)
{
    static int run_sequence = 0;
    int run_sequence_tmp = __sync_fetch_and_add(&run_sequence, 1);
    
    char cmd_mi[] = "-mi";
    char meminput_addr[50] = {0};
    const char** newargv = new const char*[argc+2];
    int i=0;
    
    if (EMEMINPUTTYPE_NONE == meminput_ctxt.type)
    {
        //  default input from file
        for (i=0; i<argc; ++i)
        {
            newargv[i] = argv[i];
        }
    }
    else
    {
        //  input from some memory
        for (; i<argc-1; ++i)
        {
            newargv[i] = argv[i];
        }
        snprintf(meminput_addr, sizeof(meminput_addr), "%lu", (unsigned long)&meminput_ctxt);
        newargv[i++] = cmd_mi;
        newargv[i++] = meminput_addr;
        newargv[i++] = argv[argc-1];
    }

    try
    {
        ez::ezOptionParser opt;
        switch (machine_type)
        {
        case EMACHINETYPE_MASCOT:
            DishonestMajorityFieldMachine<Share>(i, newargv, opt);
            break;
        case EMACHINETYPE_SHAMIRE:
            ShamirMachineSpec<ShamirShare>(i, newargv);
            break;
        case EMACHINETYPE_SEMI:
            DishonestMajorityFieldMachine<SemiShare>(i, newargv, opt);
            break;
        default:
            break;
        }
    }
    catch (runtime_error& e)
    {
        LOG_ERROR("run time error seq: " << run_sequence_tmp << " error: " << e.what());
    }
    catch(...)
    {
        LOG_ERROR("run time error seq: " << run_sequence_tmp);
    }
    
    delete[] newargv;

    return 0;
}
