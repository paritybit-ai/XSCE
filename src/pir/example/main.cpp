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

	// for psi_pir_label sue  .Modified by wumingzi. 2022:12:17,Saturday,18:32:42.
	cmd.setDefault("alg", 0);
	int64_t alg = cmd.get<int64_t>("alg");

	cmd.setDefault("type", 0);
	int64_t type = cmd.get<int64_t>("type");

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
		<< " -alg    #specify which pir alg to run. 0 means normal pir, 1 means psi_pir_lable mode.\n"
		<< " -type   #valid when alg= 1,specify pir alg in psi mode  whether client query data from server. 0 means yes, 2000 means no. \n"
		<< " -ip     #local ip address.\n"
		<< " -iprmt  #remote ip address .\n"
		<< " -port   #port .");

	LOG_INFO("role=" << role << ",srv size=" << srvSize << ",cli size=" << cliSize << ",addr=" << addr);
	LOG_INFO("step=" << step << ",seed=" << seed << ",seed1=" << seed1 << ",type=" << type << ",alg=" << alg);

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
		optVec[i].type = type;		   //not used here

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
	if (role == 0 || role == 2)
	{
		saveVec2File(dataVec, fnVec.at(0));
	}
	if (role == 1 || role == 2)
	{
		saveVec2File(idxCliVec, fnVec.at(1));
	}

	if (2 == role )
	{
		LOG_INFO("run pir alg in 2 thread.");
		std::vector<std::thread> thrds;
		for (int64_t i = 0; i < 2; i++)
		{
			LOG_INFO("thread[" << i << "]  run pir now ");
			thrds.emplace_back(std::thread([i, role, &optVec]()
										   {
											   if (0 == i)
											   {
												   LOG_INFO("inside thread. i=" << i << ",role=" << optVec[i].role);
												   LOG_INFO("run pir server ");
												   pir2PartyAlgTerminalBatch(&optVec.at(0));
											   }
											   else
											   {
												   LOG_INFO("inside thread. i=" << i << ",role=" << optVec[i].role);
												   LOG_INFO("run pir client ");
												   pir2PartyAlgTerminalBatch(&optVec.at(1));
											   }
										   }));
		}

		for (auto &t : thrds)
			t.join();

		showBlk(2, 2);
		LOG_INFO("server thread and client thread are all over......");

		//check psi result.
		//here check pir result
		rltVec.resize(0);
		int headline = optVec[1].headLine;
		std::string rltfn = optVec[1].rltFn;
		getRowStrVecFromCsvFile(rltfn, rltVec, 0);
		auto rlt_len = rltVec.size();
		showBlk(2, 2);
		LOG_INFO("pir result  number=" << rlt_len);

		if (rlt_len != cliSize - headline)
		{
			LOG_INFO("pir result error number=" << rlt_len);
			return -1;
		}

		for (size_t i = 0; i < rlt_len; i++)
		{
			LOG_INFO(i << ":" << rltVec.at(i));
		}
		return 0;
	}

	if (10 == alg) //for debug use now.
	{
		LOG_INFO("pir ut tests  running....");
		test_all(role, srvSize, cliSize, step, alg, ip, port);
		return 0;
	}
	else if (0 == alg)
	{
		std::cout << "enter normal pir mode.alg=" << alg << std::endl;
		if (!role)
		{
			LOG_INFO("pir server running....");
			pir2PartyAlgTerminalBatch(&optVec.at(0));
		}
		else
		{
			LOG_INFO("pir client running....");

			pir2PartyAlgTerminalBatch(&optVec.at(1));

			//here check pir result
			rltVec.resize(0);
			int headline = optVec[1].headLine;
			std::string rltfn = optVec[1].rltFn;
			getRowStrVecFromCsvFile(rltfn, rltVec, 0);
			auto rlt_len = rltVec.size();
			showBlk(2, 2);
			LOG_INFO("pir result  number=" << rlt_len);

			if (rlt_len != cliSize - headline)
			{
				LOG_INFO("pir result error number=" << rlt_len);
				return -1;
			}

			for (size_t i = 0; i < rlt_len; i++)
			{
				LOG_INFO(i << ":" << rltVec.at(i));
			}
		}
	}
	else if (1 == alg)
	{
		std::vector<int64_t> psi_cli_rlt;
		std::vector<int64_t> psi_srv_rlt;
		std::vector<std::string> cli_data_vec;

		if (!role)
		{
			LOG_INFO("pir supports psi mode. server running....");
			pirStr2PartyAlgTerminalBatch(&optVec.at(0), idxSrvVec, dataVec, psi_cli_rlt, psi_srv_rlt);
			auto rlt_len = psi_srv_rlt.size();
			LOG_INFO("psi_srv_rlt result  number=" << rlt_len);

			for (size_t i = 0; i < rlt_len; i++)
			{
				auto index = psi_srv_rlt.at(i);
				LOG_INFO("psi_srv_rlt[" << i << "]:" << index << "=" << idxSrvVec.at(index));
			}
		}
		else
		{
			LOG_INFO("pir supports psi mode. client running....");

			pirStr2PartyAlgTerminalBatch(&optVec.at(1), idxCliVec, cli_data_vec, psi_cli_rlt, psi_srv_rlt);

			//here check pir result
			rltVec.resize(0);
			int headline = optVec[1].headLine;
			std::string rltfn = optVec[1].rltFn;
			getRowStrVecFromCsvFile(rltfn, rltVec, 0);
			auto rlt_len = psi_cli_rlt.size();
			showBlk(2, 2);
			auto cli_data_vec_len = cli_data_vec.size();
			LOG_INFO("psi_cli_rlt result  number=" << rlt_len << ",cli_data_vec_len=" << cli_data_vec_len);

			for (size_t i = 0; i < rlt_len; i++)
			{
				auto index = psi_cli_rlt.at(i);
				if (index < rlt_len && i < cli_data_vec_len)
				{
					LOG_INFO("psi_cli_rlt[" << i << "]:" << index << "=" << idxCliVec.at(index) << ","
											<< ",srv data=" << cli_data_vec.at(i));
				}
				else if (index < rlt_len)
				{
					LOG_INFO("psi_cli_rlt[" << i << "]:" << index << "=" << idxCliVec.at(index) << ",no cli data vec");
				}
			}
		}
	}

	LOG_INFO("pir test over.");
	return 0;
}

// add ut test cases  .Modified by wumingzi. 2022:12:17,Saturday,19:39:14.
int64_t launchThread(UtFunction &func, std::vector<OptAlg> *opt_vec, std::vector<OseOpt> *ose_vec)
{
	int64_t rlt = -1;

	if (nullptr == opt_vec)
	{
		std::cout << "input  is null. exit" << std::endl;
		return rlt;
	}

	int thread_num = opt_vec->size();
	std::cout << "launchThread thd_num=" << thread_num << std::endl;

	if (thread_num < 1)
	{
		std::cout << "thread_num  is invalid=" << thread_num << std::endl;
		return rlt;
	}

	//here begint to launch thread
	// std::vector<std::thread> algTask(thread_num);
	std::vector<std::thread> thrds;
	int role = 0;
	for (int64_t i = 0; i < thread_num; i++)
	{

		thrds.emplace_back(std::thread([i, role, func, opt_vec, ose_vec]()
									   {
										   if (0 == i)
										   {
											   std::cout << "inside thread. i=" << i << ",role=" << opt_vec->at(i).role << std::endl;
											   func(&opt_vec->at(0), &ose_vec->at(0));
										   }
										   else
										   {
											   std::cout << "inside thread. i=" << i << ",role=" << opt_vec->at(i).role << std::endl;
											   func(&opt_vec->at(i), &ose_vec->at(i));
										   }
									   }));
	}

	for (auto &t : thrds)
		t.join();

	rlt = 0;
	return rlt;
}

int64_t test_all(int role, int64_t srvSize, int64_t cliSize, int step, int alg, std::string ip, int port)
{
	int64_t rlt = 0;
	int max_num = 100;
	int party_num = 2;
	std::vector<OptAlg> opt_vec(party_num);
	std::vector<OseOpt> ose_vec(party_num);

	std::cout << "test all functions here." << std::endl;

	if (srvSize < 1 || srvSize > 10000)
	{
		srvSize = 100;
	}
	if (cliSize < 1 || cliSize > 10000)
	{
		cliSize = 100;
	}

	if (step < 1 || step > 10000)
	{
		step = 100;
	}

	std::vector<AlgStatus> statusVec(party_num);
	std::vector<GlobalCfg> cfgVec(party_num);

	std::mutex mutexMapTaskDel0;
	std::mutex mutexMapTaskDel1;
	cfgVec[0].mutexMapTaskDel = &mutexMapTaskDel0;
	cfgVec[1].mutexMapTaskDel = &mutexMapTaskDel1;

	std::string ipRmt = ip;
	for (int64_t i = 0; i < party_num; i++)
	{
		OptAlg &seal_opt = opt_vec.at(i);
		opt_vec[i].thdNum = 1;	//use one thread to run psi
		opt_vec[i].thdIdex = 0; //set thread index to 0
		seal_opt.role = i;
		seal_opt.addr = ip;
		seal_opt.port = port;
		seal_opt.type = 0; //important for pir alg.
		seal_opt.statusPtr = &statusVec[i];
		seal_opt.globalCfg = &cfgVec[i];

		if (0 == i)
		{
			opt_vec[i].addr = ip;
			opt_vec[i].port = port;
			opt_vec[i].localParty.addr = ip;
			opt_vec[i].localParty.port = port;
			opt_vec[i].localParty.role = i;
			opt_vec[i].rmtParty.addr = ipRmt;
			opt_vec[i].rmtParty.port = port;
			opt_vec[i].inertalSeed = 0xaabbccddee;
			opt_vec[i].inertalSeed1 = 0xaabbccddee;
		}
		else
		{
			opt_vec[i].addr = ipRmt;
			opt_vec[i].port = port;
			opt_vec[i].localParty.addr = ipRmt;
			opt_vec[i].localParty.port = port;
			opt_vec[i].localParty.role = i;
			opt_vec[i].rmtParty.addr = ip;
			opt_vec[i].rmtParty.port = port;
		}

		//init ip vector &port vector
		opt_vec[i].ipVec.resize(2);
		opt_vec[i].ipVec[0] = ip;
		opt_vec[i].ipVec[1] = ipRmt;
		opt_vec[i].portVec.push_back(port);
		opt_vec[i].portVec.push_back(port);

		opt_vec[i].rmtParty.role = (i + 1) % 2;
		opt_vec[i].rltFn = "rlt" + std::to_string(i);

		//init test string data
		OseOpt &pir_opt = ose_vec.at(i);
		std::vector<std::string> &srv_data_vec = pir_opt.srv_data_vec;
		std::vector<std::string> &cli_data_vec = pir_opt.cli_data_vec;
		std::vector<std::string> &srv_id_vec = pir_opt.srv_id_vec;
		std::vector<std::string> &cli_id_vec = pir_opt.cli_id_vec;

		opt_vec[i].dataLen = 2;	 //set bucket pool number.
		opt_vec[i].headLine = 0; //set 0 headline to be omitted.
		opt_vec[i].col = 0;		 //set id column to 0.

		std::string id, data;
		srv_data_vec.resize(0);
		srv_id_vec.resize(0);
		for (int64_t i = 0; i < srvSize; i++)
		{
			auto num = i * step;
			id = std::to_string(num);
			data = std::to_string(num) + "," + "00_____" + std::to_string(num);
			srv_id_vec.push_back(id);
			srv_data_vec.push_back(data);
		}

		cli_data_vec.resize(0);
		cli_id_vec.resize(0);
		for (int64_t i = 0; i < cliSize; i++)
		{
			auto num = i * step;
			id = std::to_string(num);
			data = std::to_string(num) + "," + "00_____" + std::to_string(num);
			cli_id_vec.push_back(id);
			cli_data_vec.push_back(data);
		}
	}

	std::vector<std::string> test_name;
	std::vector<int64_t> test_rlt;
	std::string alg_name;

	//
	std::vector<int> test_flag = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	// std::vector<int> test_flag = {0, 0, 0, 4, 5, 6, 7, 8, 9, 10};
	int64_t tmp_rlt = 0;
	UtFunction ut_func;
	int test_cnt = 0;

	//test case1: test ckks dot product stream function
	alg_name = "pir support psi mode test.";
	test_name.push_back(alg_name);
	//set test type to 0
	for (int64_t i = 0; i < party_num; i++)
	{
		opt_vec.at(i).type = 0;
	}
	if (test_flag.at(test_cnt++))
	{
		ut_func = pirUtest;
		tmp_rlt = launchThread(ut_func, &opt_vec, &ose_vec);
	}

	if (ose_vec.at(1).test_rlt)
	{ //test ok
		test_rlt.push_back(1);
	}
	else
	{ //test fail
		test_rlt.push_back(0);
	}

	std::cout << "test all cases over." << std::endl
			  << std::endl
			  << std::endl
			  << std::endl;
	//here to show all test rlt
	int test_len = test_name.size();
	int err_num = 0;
	for (int64_t i = 0; i < test_len; i++)
	{
		std::cout << "Test[" << i << "]:" << test_name.at(i);
		if (test_rlt.at(i))
		{
			std::cout << " Ok.";
		}
		else
		{
			std::cout << " Error.";
			err_num++;
		}
		std::cout << std::endl;
	}
	std::cout << "Total cases=" << test_len << ",error cases=" << err_num << std::endl;

	return rlt;
}

int64_t pirUtest(OptAlg *opt, OseOpt *pir_opt)
{

	int64_t rlt = 0;
	int max_num = 100;
	std::string tname = "pirUtest ";

	std::cout << tname << " top alg test" << std::endl;
	int role = opt->role;

	std::vector<std::string> &srv_data_vec = pir_opt->srv_data_vec;
	std::vector<std::string> &cli_data_vec = pir_opt->cli_data_vec;
	std::vector<std::string> &srv_id_vec = pir_opt->srv_id_vec;
	std::vector<std::string> &cli_id_vec = pir_opt->cli_id_vec;

	//save data to file
	std::vector<std::string> fnVec;
	fnVec.push_back("srv");
	fnVec.push_back("cli");
	if (role == 0)
	{
		saveVec2File(srv_data_vec, fnVec.at(0));
		opt->dataFn = fnVec.at(0);
		pir2PartyAlgTerminalBatch(opt);
	}
	if (role == 1)
	{
		saveVec2File(cli_id_vec, fnVec.at(1));
		opt->dataFn = fnVec.at(1);
		pir2PartyAlgTerminalBatch(opt);
	}

	//here to check rlt file
	if (0 == role)
	{
		//no need to check srv data file
	}
	else
	{
		//check cli data file.
		std::vector<std::string> rltVec;
		std::vector<uint64_t> psiRlt;
		std::vector<uint64_t> srvRlt;
		rltVec.resize(0);
		int headline = opt->headLine;
		std::string rltfn = opt->rltFn;
		getRowStrVecFromCsvFile(rltfn, rltVec, 0);
		auto rlt_len = rltVec.size();
		showBlk(2, 2);
		LOG_INFO("pir result  number=" << rlt_len);
		auto cliSize = cli_id_vec.size();
		if (rlt_len != cliSize - headline)
		{
			LOG_INFO("pir result error number=" << rlt_len << ",cliSize=" << cliSize);
			pir_opt->test_rlt = false;
			return -1;
		}

		for (size_t i = 0; i < rlt_len; i++)
		{
			LOG_INFO(i << ":" << rltVec.at(i));
		}
	}
}

int64_t psi_pir_ut(int role, int64_t srvSize, int64_t cliSize, int step, int alg, std::string ip, int port)
{
	OptAlg optAlg;
	std::vector<OptAlg> optVec(2);
	AlgStatus status;
	int64_t seed1 = 1234;
	int64_t seed = 1234;
	int type = 0;
	std::string ipRmt = ip;

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
		optVec[i].type = type;		   //not used here

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
	if (role == 0 || role == 2)
	{
		saveVec2File(dataVec, fnVec.at(0));
	}
	if (role == 1 || role == 2)
	{
		saveVec2File(idxCliVec, fnVec.at(1));
	}

	return 0;
}
//   .Modification over by wumingzi. 2022:12:17,Saturday,19:40:00.