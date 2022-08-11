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
 * @author Created by wumingzi.  2022:07:21,Thursday,00:09:48.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "main.h"

#include <vector>
#include <thread>
#include <random>

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Crypto/RCurve.h>

#include "toolkits/util/include/xlog.h"
#include "toolkits/util/include/xutil.hpp"
#include "PSI/include/psi.h"
#include "common/pub/include/Defines.h"
#include "common/pub/include/util.h"

using namespace std;
using namespace util;
using namespace xsce_ose;

const block commonSeed = oc::toBlock(123456);

u64 senderSize;
u64 receiverSize;
u64 width;
u64 height;
u64 logHeight;
u64 hashLengthInBytes;
u64 bucket, bucket1, bucket2;
std::string ip;

// psi example code  .Modified by wumingzi. 2022:07:14,Thursday,10:02:36.
// OptAlg struct is used to pass parameters to different kinds of algorithms including psi and pir.
// key parameters note below:
// role:  spcify the node role type. 0 means server, 1 means client
// dataFn:  spcify the data input file.
// rltFn:  spcify the name of result file.
// ipVec: holds the ip address of each party, the ip number of ipVec should be the same as party number.
// portVec: holds the port number of each party,the port number of portVec should be the same as party number
// ipVec and portVec should be the same order for both server and client parties.
// addr: holds the ip address of local party.
// port: holds the port number of local party
// localParty: holds the ip address and port of local party.
// rmtParty: holds the ip address and port of remote party.

//other parameters:
// thdNum: the thread number to run alg;
// thdIdex: indicates the index of current thread;
// commonSeed: 1st part of 128-bit common random seed;
// commonSeed1: 2nd part of 128-bit common random seed;
// common seed shsould be the same for both server and client parties.
// inertalSeed: 1st part of 128-bit internal random seed;
// inertalSeed1: 2nd part of 128-bit internal random seed;
// internal seed should be different between  server and client parties.

//psi alg related parameters listed below:
//headline: specify the number of head rows which will be omitted when reading data
//col:      specify the column of id data
//input file data only support csv format

int main(int argc, char **argv)
{
	oc::CLP cmd;
	cmd.parse(argc, argv);

	cmd.setDefault("r", 2);
	int role = cmd.get<u64>("r");

	cmd.setDefault("fn", 0);
	int fn = cmd.get<u64>("fn");

	cmd.setDefault("ss", 10000);
	int64_t srvSize = cmd.get<u64>("ss");

	cmd.setDefault("cs", 10);
	int64_t cliSize = cmd.get<u64>("cs");

	cmd.setDefault("st", 100);
	int64_t step = cmd.get<u64>("st");

	cmd.setDefault("ip", "127.0.0.1");
	ip = cmd.get<string>("ip");

	cmd.setDefault("iprmt", "127.0.0.1");
	std::string ipRmt = cmd.get<string>("iprmt");

	cmd.setDefault("port", 1212);
	int port = cmd.get<int>("port");

	cmd.setDefault("seed", 123);
	int64_t seed = cmd.get<int64_t>("seed");

	cmd.setDefault("seed1", 456);
	int64_t seed1 = cmd.get<int64_t>("seed1");

	cmd.setDefault("dbg", "0");

	std::string addr = ip + ":" + std::to_string(port);
	LOG_INFO("ipaddr=" << addr);

	bucket1 = bucket2 = 1 << 8;

	LOG_INFO("\n===============================================\n"
			 << "||  Private Set Intersection Algorithm   ||\n"
			 << "===============================================\n"
			 << "Experimenet flag:\n"
			 << " -r 0    to run a psi query server.\n"
			 << " -r 1    to run a psi query client.\n"
			 << " -r 2    to run a psi query  in 2 threads(by default).\n"
			 << "\n"
			 << "Parameters:\n"
			 << " -ss     #elements on server side.\n"
			 << " -cs     #elements on client side.\n"
			 << " -st     #elements value step on client side.\n"
			 << " -ip     #local ip address.\n"
			 << " -iprmt  #remote ip address .\n"
			 << " -port   #port .\n\n");

	LOG_INFO("role=" << role << ",fn=" << fn << ",srv size=" << srvSize << ",cli size=" << cliSize << ",addr=" << addr);
	LOG_INFO("step=" << step << ",seed=" << seed << ",seed1=" << seed1 << ",cli size=" << cliSize << ",addr=" << addr);

	//step1: init optAlg for psi alg;
	OptAlg optAlg;
	std::vector<OptAlg> optVec(2);

	for (int64_t i = 0; i < 2; i++)
	{
		optVec[i].role = i;				  //set role
		optVec[i].thdNum = 1;			  //use one thread to run psi
		optVec[i].thdIdex = 0;			  //set thread index to 0
		optVec[i].commonSeed = seed;	  //set 1st 64 bits of random seed
		optVec[i].commonSeed1 = seed1;	  //set 2nd 64 bits random seed
		optVec[i].dataLen = 3;			  //for debug show number.
		optVec[i].statusPtr = &optVec[i]; //not used here

		//set ip addr and port
		if (0 == i)
		{
			optVec[i].addr = ip;
			optVec[i].port = port;
			optVec[i].localParty.addr = ip;
			optVec[i].localParty.port = port;
			optVec[i].localParty.role = i;
			optVec[i].rmtParty.addr = ipRmt;
			optVec[i].rmtParty.port = port;
			optVec[i].inertalSeed = 0xaabbccddee;
			optVec[i].inertalSeed1 = 0xaabbccddee;
		}
		else
		{
			optVec[i].addr = ipRmt;
			optVec[i].port = port;
			optVec[i].localParty.addr = ipRmt;
			optVec[i].localParty.port = port;
			optVec[i].localParty.role = i;
			optVec[i].rmtParty.addr = ip;
			optVec[i].rmtParty.port = port;
		}

		//init ip vector &port vector
        optVec[i].ipVec.resize(2);
        optVec[i].ipVec[role % 2] = ip;
        optVec[i].ipVec[(role + 1) % 2] = ipRmt;
		optVec[i].portVec.push_back(port);
		optVec[i].portVec.push_back(port);

		optVec[i].rmtParty.role = (i + 1) % 2;
		optVec[i].rltFn = "rlt" + std::to_string(i);
	}

	//step 2: generate test data file.
	std::vector<std::string> idxVec;
	std::vector<std::string> idxSrvVec;
	std::vector<std::string> idxCliVec;
	std::vector<std::string> dataVec;
	std::vector<std::string> rltVec;
	std::vector<uint64_t> psiRlt;
	std::vector<uint64_t> srvRlt;

	std::string id;
	std::string data;
	std::vector<std::string> fnVec;
	fnVec.push_back("srv");
	fnVec.push_back("cli");
	int num = 0;

	for (int64_t i = 0; i < srvSize; i++)
	{
		num = i * step;
		id = std::to_string(num);
		idxSrvVec.push_back(id);
	}

	for (int64_t i = 0; i < cliSize; i++)
	{
		num = i * step;
		id = std::to_string(num);
		idxCliVec.push_back(id);
	}

	//save test data to file
	saveVec2File(idxSrvVec, fnVec.at(0));
	saveVec2File(idxCliVec, fnVec.at(1));
	for (int64_t i = 0; i < 2; i++)
	{
		optVec[i].dataFn = fnVec.at(i);
	}

	//step 3: launch threads to run psi.
	uint32_t *hash_buf_srv = nullptr;
	uint32_t *hash_buf_cli = nullptr;
	int64_t id_num = idxVec.size();
	std::vector<int64_t> indexId(id_num);
	std::vector<int64_t> indexId_srv;
	std::vector<int64_t> indexId_cli;
	util::initSortIndex(indexId, id_num);

	int64_t id_num_srv = idxSrvVec.size();
	int64_t id_num_cli = idxCliVec.size();
	std::vector<uint64_t> psi_result;
	std::vector<uint64_t> psi_result_srv;
	std::vector<uint64_t> srv_resutl_index;
	std::vector<uint64_t> srv_resutl_index_srv;

	//convert id to hash_buf
	util::initSortIndex(indexId_srv, id_num_srv);
	util::initSortIndex(indexId_cli, id_num_cli);
	util::convertStrVec2Md5Index(idxSrvVec, hash_buf_srv, indexId_srv);
	util::convertStrVec2Md5Index(idxCliVec, hash_buf_cli, indexId_cli);

	if (2 == role)
	{
		LOG_INFO("run psi alg in 2 thread.");

		std::vector<std::thread> thrds;
		for (int64_t i = 0; i < 2; i++)
		{
			LOG_INFO("thread[" << i << "]  run psi ");
			thrds.emplace_back(std::thread([i, role, &optVec, hash_buf_srv, hash_buf_cli, id_num_srv, id_num_cli, &srv_resutl_index, &srv_resutl_index_srv, &psi_result, &psi_result_srv]()
										   {
											   LOG_INFO("inside thread. i=" << i << ",role=" << optVec[i].role);

											   if (0 == i)
											   {
												   LOG_INFO("run psi server ");
												   xscePsiAlg::hashbufPsiAlgClient((uint64_t *)hash_buf_srv, id_num_srv, psi_result_srv, &optVec[i], srv_resutl_index_srv);
											   }
											   else
											   {
												   LOG_INFO("run psi client ");
												   xscePsiAlg::hashbufPsiAlgClient((uint64_t *)hash_buf_cli, id_num_cli, psi_result, &optVec[i], srv_resutl_index);
											   }
										   }));
		}

		for (auto &t : thrds)
			t.join();

		showBlk(2, 2);
		LOG_INFO("server thread and client thread are all over......");
		uint64_t rlt_num = psi_result.size();

		//check psi result.
		if (rlt_num != (uint64_t)id_num_cli || rlt_num != srv_resutl_index.size())
		{
			LOG_INFO("psi result error.  psi result num=" << rlt_num);
		}

		LOG_INFO("psi result number=" << std::dec << rlt_num);

		uint64_t idx = 0;
		for (uint64_t i = 0; i < rlt_num; i++)
		{
			idx = psi_result.at(i);
			if (idx >= (uint64_t)id_num_cli)
			{
				LOG_INFO("psi result error at index=" << idx);
				break;
			}
			LOG_INFO("rlt[" << i << "]: local index=" << idx << ",srv index=" << srv_resutl_index.at(i) << ",data=" << idxCliVec.at(idx));
		}

		return 0;
	}

	//below run 2 psi instances, one spcifies role to 0 and another spcifies role to 1.
	if (!role)
	{
		LOG_INFO("pir server running....");
		xscePsiAlg::hashbufPsiAlgClient((uint64_t *)hash_buf_srv, id_num_srv, psi_result_srv, &optVec[0], srv_resutl_index_srv);
	}
	else
	{
		LOG_INFO("pir client running....");
		xscePsiAlg::hashbufPsiAlgClient((uint64_t *)hash_buf_cli, id_num_cli, psi_result, &optVec[1], srv_resutl_index);

		showBlk(2, 2);
		uint64_t rlt_num = psi_result.size();
		if (rlt_num != (uint64_t)id_num_cli || rlt_num != srv_resutl_index.size())
		{
			LOG_INFO("psi result error.  psi result num=" << rlt_num);
		}

		LOG_INFO("psi result number=" << std::dec << rlt_num);
		uint64_t idx = 0;
		for (uint64_t i = 0; i < rlt_num; i++)
		{
			idx = psi_result.at(i);
			if (idx >= (uint64_t)id_num_cli)
			{
				LOG_INFO("psi result error at index=" << idx);
				break;
			}
			LOG_INFO("rlt[" << i << "]: local index=" << idx << ",srv index=" << srv_resutl_index.at(i) << ",data=" << idxCliVec.at(idx));
		}
	}

	return 0;
}

void readHugeFile(std::string fn)
{
	LOG_INFO("begin read file=" << fn);
	uint64_t readRlt = -1;
	std::vector<std::string> rowVec;
	std::vector<std::string> idVec;
	std::string pedSep = " ,\t";

	if (fn.length() < 1)
	{
		LOG_INFO("input file error.fn=" << fn);
		return;
	}

	//read line feedback loc first.
	std::string datafn = fn;
	uint64_t showCnt = 3;
	int64_t readFileSize = 0; //0 means readl entile file data.

	util::TimeUtils t1;
	//here call huge file read function in abyAlg.h
	t1.start("begin to read file.");
	int col = 0;
	readRlt = getPirDataFileShm(datafn, rowVec, idVec, col, readFileSize);

	LOG_INFO("read huge file ,valid line=" << readRlt);
	if (idVec.size() != rowVec.size())
	{
		LOG_INFO("read file error. exit");
		return;
	}

	for (uint64_t i = 0; i < readRlt && i < showCnt; i++)
	{
		LOG_INFO(i << ": id=" << idVec.at(i) << "--,data line="
				   << "--");
	}

	LOG_INFO("read file row=" << rowVec.size());
	t1.stopS("reading file over.");

	return;
}

uint64_t calPsiRlt(uint32_t incra, uint32_t incrb, uint64_t server_neles, uint64_t client_neles)
{
	//calculate psi element here.
	int gcdn = std::__gcd(incra, incrb);
	int lcmn = incra / gcdn * incrb;

	int loopa = lcmn / incra;
	int loopb = lcmn / incrb;

	uint32_t loopNums = server_neles / loopa;
	uint32_t loopNumc = client_neles / loopb;
	uint32_t finalLoop = std::min(loopNumc, loopNums);
	LOG_INFO("final loop=" << finalLoop << ",server loop=" << loopNums << ",client loop=" << loopNumc);

	return finalLoop;
}
