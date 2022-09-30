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
 * @file machinerunner.h
 * @author Created by chonglou. 2022:05:28
 * @brief 
 * @version 
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MACHINE_RUNNER_H
#define MACHINE_RUNNER_H

#include <vector>

using namespace std;

typedef enum _tagMachineType{
    EMACHINETYPE_NONE               = 0,    //  未知
    EMACHINETYPE_MASCOT             = 1,    //  
    EMACHINETYPE_SHAMIRE            = 2,    //  
    EMACHINETYPE_SEMI               = 3,    //  
}MachineType, *LPMachineType;

typedef enum _tagMemInput_Type{
    EMEMINPUTTYPE_NONE              = 0,    //  默认从文件读取
    EMEMINPUTTYPE_VECTORES_INT      = 1,    //  
    EMEMINPUTTYPE_VECTORES_FLOAT    = 2,    //  
    EMEMINPUTTYPE_VECTORES_FIX      = 3,    //  
}MemInput_Type, *LPMemInput_Type;

typedef struct _tagMemInput_Context{
    _tagMemInput_Context(){
        input = 0;
        size = 0;
        type = EMEMINPUTTYPE_NONE;
    }
    void*   input;
    int     size;
    MemInput_Type   type;
}MemInput_Context, *LPMemInput_Context;

/**
 * @brief Machine caller
 * 
 */
class MachineRunner
{
private:
    MachineRunner(){}

public:
    /**
     * @brief 
     * 
     * @param argc 
     * @param argv 
     * @param machine_type 
     * @return int 
     */
    static int Run(int argc, const char* argv[], MachineType machine_type){
        MemInput_Context meminput_ctxt;
        return Run(argc, argv, machine_type, meminput_ctxt);
    }
    /**
     * @brief 
     * 
     * @tparam T 
     * @param argc 
     * @param argv 
     * @param machine_type 
     * @param input 
     * @return int 
     */
    template<typename T>
    static int Run(int argc, const char* argv[], MachineType machine_type, T& input){
        T ar[1];
        ar[0] = std::move(input);
        int ret = Run(argc, argv, machine_type, ar);
        input = std::move(ar[0]);
        return ret;
    }
    /**
     * @brief 
     * 
     * @tparam T 
     * @tparam size 
     * @param argc 
     * @param argv 
     * @param machine_type 
     * @return int 
     */
    template<typename T, int size>
    static int Run(int argc, const char* argv[], MachineType machine_type, T (&input)[size]){
        MemInput_Context meminput_ctxt;
        meminput_ctxt.input = input;
        meminput_ctxt.size = size;
        try
        {
            if (typeid(input) == typeid(vector<int>[size])){
                meminput_ctxt.type = EMEMINPUTTYPE_VECTORES_INT;
            }
            else if (typeid(input) == typeid(vector<double>[size])){
                meminput_ctxt.type = EMEMINPUTTYPE_VECTORES_FLOAT;
            }
            else
            {
                return -1;
            }
        }
        catch(const std::exception& e)
        {
            return -1;
        }

        return Run(argc, argv, machine_type, meminput_ctxt);
    }
    /**
     * @brief 
     * 
     * @tparam T 
     * @param argc 
     * @param argv 
     * @param machine_type 
     * @param value 
     * @param size 
     * @return int 
     */
    template<typename T>
    static int Run(int argc, const char* argv[], MachineType machine_type, T* value = NULL, int size=0){
        MemInput_Context meminput_ctxt;
        if (NULL != value)
        {
            meminput_ctxt.input = value;
            meminput_ctxt.size = size;
            try
            {
                if (typeid(value) == typeid(vector<int>*)){
                    meminput_ctxt.type = EMEMINPUTTYPE_VECTORES_INT;
                }
                else if (typeid(value) == typeid(vector<double>*)){
                    meminput_ctxt.type = EMEMINPUTTYPE_VECTORES_FLOAT;
                }
                else
                {
                    return -1;
                }
            }
            catch(const std::exception& e)
            {
                return -1;
            }
        }

        return Run(argc, argv, machine_type, meminput_ctxt);
    }

private:
    static int Run(int argc, const char* argv[], MachineType machine_type, MemInput_Context& meminput_ctxt);
};

#endif  //  MACHINE_RUNNER_H
