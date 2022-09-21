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
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
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

#include "toolkits/util/include/xutil.hpp"
#include "common/pub/include/globalCfg.h"

using namespace std;
using namespace xscePirAlg;
using namespace xsce_ose;

const block commonSeed = oc::toBlock(123456);

u64 senderSize;
u64 receiverSize;
u64 width;
u64 height;
u64 logHeight;
u64 hashLengthInBytes;
u64 bucket, bucket1, bucket2;
string ip;

// pir example code  .Modified by wumingzi. 2022:07:15,Thursday,11:50:30.
// OptAlg struct is used to pass parameters to different kinds of algorithms including psi and pir.
// key parameters note below:
// role:  spcify the node role type. 0 means server, 1 means client
// dataFn:  spcify the data input file.
// rltFn:  spcify the name of result file.

// communication address related parameters:
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
// common seed should be the same for both server and client parties.
// inertalSeed: 1st part of 128-bit internal random seed;
// inertalSeed1: 2nd part of 128-bit internal random seed;
// internal seed should be different between  server and client parties.

//pir alg related parameters listed below:
//dataLen: specify the bucket pool number. input data will be split into multiple bucket pool
//headline: specify the number of head rows which will be omitted when reading data
//col:      specify the column of id data
//input file data only support csv format

int main(int argc, char **argv)
{
	oc::CLP cmd;
	cmd.parse(argc, argv);

	cmd.setDefault("r", 2);
	int role = cmd.get<u64>("r");

	cmd.setDefault("ss", 100);
	int64_t srvSize = cmd.get<u64>("ss");

	cmd.setDefault("cs", 10);
	uint64_t cliSize = cmd.get<u64>("cs");

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

	LOG_INFO(
		"\n===============================================\n"
		<< "||  Private Information Retrieve Algorithm   ||\n"
		<< "===============================================\n"
		<< "Experimenet flag:\n"
		<< " -r 0    to run a pir query server.\n"
		<< " -r 1    to run a pir query client.\n"
		<< " -r 2    to run a pir query  in 2 threads(by default).\n"
		<< "\n"
		<< "Parameters:\n"
		<< " -ss     #elements on server side.\n"
		<< " -cs     #elements on client side.\n"
		<< " -st     #elements value step on client side.\n"
		<< " -ip     #local ip address.\n"
		<< " -iprmt  #remote ip address .\n"
		<< " -port   #port .");

	LOG_INFO("role=" << role << ",srv size=" << srvSize << ",cli size=" << cliSize << ",addr=" << addr);
	LOG_INFO("step=" << step << ",seed=" << seed << ",seed1=" << seed1 << ",cli size=" << cliSize << ",addr=" << addr);

	//step1: init optAlg for psi alg;
	OptAlg optAlg;
	std::vector<OptAlg> optVec(2);
	AlgStatus status;

	for (int64_t i = 0; i < 2; i++)
	{
		optVec[i].role = i;			   //set role
		optVec[i].thdNum = 1;		   //use one thread to run psi
		optVec[i].thdIdex = 0;		   //set thread index to 0
		optVec[i].commonSeed = seed;   //set 1st 64 bits of random seed
		optVec[i].commonSeed1 = seed1; //set 2nd 64 bits random seed
		optVec[i].dataLen = 2;		   //set bucket pool number.
		optVec[i].headLine = 0;		   //set 0 headline to be omitted.
		optVec[i].col = 0;			   //set id column to 0.
		optVec[i].statusPtr = &status; //not used here

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
	std::vector<int64_t> psi_cli_rlt;
	std::vector<int64_t> psi_srv_rlt;
	std::vector<int64_t> pir_srv_rlt;

	//for mtx send test
	std::vector<std::vector<int64_t> > send_mtx;
	std::vector<std::vector<int64_t> > rcv_mtx;

	std::string id;
	std::string data;
	int num = 0;
	std::vector<std::string> fnVec;
	fnVec.push_back("srv");
	fnVec.push_back("cli");
	for (int64_t i = 0; i < 2; i++)
	{
		optVec[i].dataFn = fnVec.at(i);
	}

	for (int64_t i = 0; i < srvSize; i++)
	{
		num = i * step;
		id = std::to_string(num);
		data = std::to_string(num) + "," + "00_____" + std::to_string(num);
		idxSrvVec.push_back(id);
		dataVec.push_back(data);
	}

	for (uint64_t i = 0; i < cliSize; i++)
	{
		num = i * step;
		id = std::to_string(num);
		idxCliVec.push_back(id);
	}

	for (int64_t i = 0; i < srvSize; i++)
	{
		std::vector<int64_t> vec;
		for (int64_t j = 0; j < cliSize; j++)
		{
			vec.push_back(j);
		}
		send_mtx.push_back(vec);
	}

	if (!role)
	{
		LOG_INFO("pir server running....");
		pirStr2PartyAlgTerminalBatch(&optVec.at(0), idxSrvVec, dataVec, psi_cli_rlt, pir_srv_rlt);

		int64_t len2 = pir_srv_rlt.size();
		LOG_INFO("pir result  ,srv rlt num=" << len2);

		for (size_t i = 0; i < len2; i++)
		{
			int64_t index = pir_srv_rlt.at(i);
			if (index >= 0 && index < dataVec.size())
				LOG_INFO("rlt str[" << i << "]=" << dataVec.at(index) << ",srv rlt[" << i << "]=" << pir_srv_rlt.at(i));
		}

		// std::cout << "begin to send mtx" << std::endl;
		// sendInt64Mtx(&optVec.at(0), send_mtx);
	}
	else
	{
		LOG_INFO("pir client running....");

		pirStr2PartyAlgTerminalBatch(&optVec.at(1), idxCliVec, rltVec, psi_cli_rlt, psi_srv_rlt);

		//here check pir result
		auto rlt_len = rltVec.size();
		showBlk(2, 2);
		int64_t len1 = psi_cli_rlt.size();
		LOG_INFO("pir result  number=" << rlt_len << ",psi_cli_rlt num=" << len1);

		for (size_t i = 0; i < rlt_len; i++)
		{
			auto index = psi_cli_rlt.at(i);
			if (index >= 0 && index < idxCliVec.size())
				LOG_INFO("rlt str[" << i << "]=" << rltVec.at(i) << ",cli rlt[" << i << "]=" << index << ",id=" << idxCliVec.at(index));
		}

		// std::cout << "begin to rcv mtx" << std::endl;
		// rcvInt64Mtx(&optVec.at(1), rcv_mtx);
		// int64_t mtx_len = rcv_mtx.size();
		// for (int64_t i = 0; i < mtx_len; i++)
		// {
		// 	int64_t row_len = rcv_mtx.at(i).size();
		// 	std::cout << "mtx row " << i << " size="<<row_len << std::endl;
		// 	for (int64_t j = 0; j < row_len; j++)
		// 	{
		// 		std::cout << "row[" << j << "]=" << rcv_mtx.at(i).at(j) << std::endl;
		// 	}
		// }
	}

	return 0;
}
