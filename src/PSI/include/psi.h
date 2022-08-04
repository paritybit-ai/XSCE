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
 * @file psi.h
 * @author Created by haiyu.  2022:07:21,Thursday,00:10:48.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <vector>
#include "common/pub/include/Defines.h"
#include "common/pub/include/globalCfg.h"

namespace xscePsiAlg
{
    using OptAlg = xsce_ose::OptAlg;
    int64_t hashbufPsiAlgClient(uint64_t *hashBufInput,
                                int64_t psiLen,
                                std::vector<uint64_t> &rltVec,
                                OptAlg *optAlg,
                                std::vector<uint64_t> &srvIndexVec,
                                int roleSwitch = 1);

    int64_t hashbufPsiAlgClient(uint64_t *hashBufInput,
                                int64_t psiLen,
                                std::vector<uint64_t> &rltVec,
                                OptAlg *optAlg,
                                std::vector<uint64_t> &srvIndexVec,
                                std::vector<util::block>& oprf_values,
                                int roleSwitch = 1);
} // namespace xscePsiAlg
