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
 * @file pir_ose.h
 * @author Created by haiyu. 2022:08:03,Wednesday,18:20.
 * @brief 
 * @version 
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once
#include "src/common/pub/include/globalCfg.h"
#include <vector>
#include <string>


namespace xscePirAlg {
extern int64_t pir2PartyAlgTerminalBatch(xsce_ose::OptAlg *optAlg);
extern int64_t pirStr2PartyAlgTerminalBatch(xsce_ose::OptAlg *optAlg, std::vector<std::string> &id_vec,
                                         std::vector<std::string> &data_vec,
                                        //  std::vector<std::int64_t> &psi_cli_rlt,
                                        //  std::vector<std::int64_t> &psi_srv_rlt,
                                         std::vector<xsce_ose::PirOutput>* output);
                                        //  std::vector<std::tuple<uint64_t, std::string, std::string>>* id_str_rlt,
                                        //  std::vector<std::string>* cli_pir_rlt);
extern int64_t pirStr2PartyAlgTerminalBatch(xsce_ose::OptAlg *optAlg, std::vector<std::string> &&id_vec,
                                         std::vector<std::string> &&data_vec,
                                        //  std::vector<int64_t> &psi_cli_rlt,
                                        //  std::vector<int64_t> &psi_srv_rlt,
                                         std::vector<xsce_ose::PirOutput>* output);
                                        //  std::vector<std::tuple<uint64_t, std::string, std::string>>* id_str_rlt,
                                        //  std::vector<std::string>* cli_pir_rlt);

} // namespace xscePirAlg
