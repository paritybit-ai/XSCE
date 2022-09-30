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
 * @file main.cpp
 * @author Created by chonglou. 2022:05:28
 * @brief 
 * @version 
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <random>

#include "base-compute.h"
#include "toolkits/util/include/xlog.h"

using namespace std;

void Help()
{
    LOG_INFO("************************************");
    LOG_INFO("example: ./mp_spdz -c 2 -r 1 -p 127.0.0.1:7878 add2");
    LOG_INFO("-c    count of parties");
    LOG_INFO("-r    role number of party");
    LOG_INFO("-p    port of number 0, the format is ip:port");
    LOG_INFO("add2  the circuit file, include: add2、mul2、cmp2、var2、mid2、add3、mul3、var3、mid3");
    LOG_INFO("************************************");
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        Help();
        return -1;
    }
    // SPDZAlg struct is used to pass parameters to different kinds of algorithms for spdz computation.
    // key parameters note below:
    // role:  specify the party(role) id.
    // input:  specify the spdz data input.
    // endpointlist_spdz:  specify the endpoint list for spdz.
    // output_file: output file.
    SPDZAlg spdzalg;
    spdzalg.input.clear();

    int nparties = -1;
    int opt = -1;
    while ((opt = getopt(argc, argv, "c:C:r:R:p:P:")) != -1)
    {
        switch (opt)
        {
        case 'c':
        case 'C':
            nparties = atoi(optarg);
            break;
        case 'r':
        case 'R':
            spdzalg.role = atoi(optarg);
            break;
        case 'p':
        case 'P':
            spdzalg.endpointlist_spdz = optarg;
            break;
        default: 
            LOG_ERROR("unknown params");
            Help();
            return -1;
        }
    }

    if (optind >= argc)
    {
        LOG_ERROR("no circuit file.");
        Help();
        return -1;
    }
    EndPoint_SPDZ ep_spdz;
//    spdzalg.middle_prefix = "input";
    spdzalg.output_file = "output";

    uniform_real_distribution<double> u(0,1);
    default_random_engine e(time(NULL));
    for (auto i = 0; i < 32; i++)
    {
        double rand = u(e) * 1000;
        spdzalg.input.push_back(rand);
    }

    std::string circuit_file = argv[optind];
    if (circuit_file == "add2")
    {
        runAdd2(&spdzalg);
    }
    else if (circuit_file == "mul2")
    {
        runMul2(&spdzalg);
    }
    else if (circuit_file == "cmp2")
    {
        runCmp2(&spdzalg);
    }
    else if (circuit_file == "var2")
    {
        runVar2(&spdzalg);
    }
    else if (circuit_file == "mid2")
    {
        runMid2(&spdzalg);
    }
    else if (circuit_file == "add3")
    {
        runAdd3(&spdzalg);
    }
    else if (circuit_file == "mul3")
    {
        runMul3(&spdzalg);
    }
    else if (circuit_file == "var3")
    {
        runVar3(&spdzalg);
    }
    else if (circuit_file == "mid3")
    {
        runMid3(&spdzalg);
    }
    else
    {
        LOG_ERROR("unknown circuit: " << circuit_file);
    }

    return 0;
}
