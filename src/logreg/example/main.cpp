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
using namespace xsceLogRegAlg;
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

// logistic regression (vertical 2 parties) example code  .Modified by wumingzi. 2023:02:15,Wednesday,14:34:49.
//   .Modified by wumingzi. 2022:07:15,Thursday,11:50:30.
// OptAlg struct is used to pass parameters to different kinds of algorithms including logistic regression.
// key parameters note below:

// role:  spcify the node role type. 0 means server, 1 means client

//training data input:
// sampleNum: specify sample number
// featureNum: specify feature number
// sampleDataBuf: ** double pointer. sampleNum * featureNum  sample value.
// lableDataBuf: * double pointer. samplenNum label value. l

//testing data input (the same feature as featureNum):
// testSampleNum: specify sample number
// testSampleDataBuf: ** double pointer. testSampleNum * featureNum  sample value.
// testLableDataBuf: * double pointer. testSampleNum label value. l


//training parameters:
//iteration: traning iteration number.
//learningRate: learning rate factor. supposing 5,then actual learning rate is 1/(2^5)
//batchSize:  training batch size.

// communication address related parameters:
// ipVec: holds the ip address of each party, the ip number of ipVec should be the same as party number.
// portVec: holds the port number of each party,the port number of portVec should be the same as party number
// ipVec and portVec should be the same order for both server and client parties.
// addr: holds the ip address of local party.
// port: holds the port number of local party
// localParty: holds the ip address and port of local party.
// rmtParty: holds the ip address and port of remote party.


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

	cmd.setDefault("st", 1);
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
		<< "||  Logistic regression 2 parties vertical   ||\n"
		<< "===============================================\n"
		<< "Experimenet flag:\n"
		<< " -r 0    to run a logistic regression  server.\n"
		<< " -r 1    to run a logistic regression  client.\n"
		<< "\n"
		<< "Parameters:\n"
		<< " -ss     #sample number(server and client should have the same sample number).\n"
		<< " -cs     #training batch size.\n"
		<< " -st     #training iteration number.\n"
		<< " -type   #1 means use spdz inner product , 0 means plaintext inner product\n"
		<< " -ip     #local ip address.\n"
		<< " -iprmt  #remote ip address .\n"
		<< " -port   #port .");

	LOG_INFO("role=" << role << ",srv size=" << srvSize << ",cli size=" << cliSize << ",addr=" << addr);
	LOG_INFO("step=" << step << ",seed=" << seed << ",seed1=" << seed1 << ",type=" << type << ",alg=" << alg);

	//step1: init optAlg for psi alg;
	OptAlg optAlg;
	std::vector<OptAlg> optVec(2);
	AlgStatus status;
	GlobalCfg cfg;

	int row = 100;
	int col = 10;
	int simd = 1000;
	int batchSize = 10;
	int iter = 1;

	if (srvSize > row)
	{
		row = srvSize;
		LOG_INFO( "set sample num to srvSize=" << srvSize );
	}

	
	if (cliSize > batchSize)
	{
		batchSize = cliSize;
		LOG_INFO( "set cliSize num to cliSize=" << cliSize );
	}
	if (step > iter)
	{
		iter = step;
		LOG_INFO( "set iter num to step=" << step );
	}
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
		optVec[i].simdLen = simd;	   //not used here

		optVec[i].batchSize = batchSize; //not used here
		optVec[i].iteration = iter;		 //not used here

		optVec[i].globalCfg = &cfg;							  //not used here
		optVec[i].taskId = "lr training" + std::to_string(i); //not used here

		//set ip addr and port
		if (0 == i)
		{
			optVec[i].addr = ip;
			optVec[i].port = port;
			optVec[i].localParty.addr = ip;
			optVec[i].localParty.port = port;
			optVec[i].localParty.role = i;
			optVec[i].rmtParty.addr = ipRmt;
			optVec[i].rmtParty.port = port+1; 	//client use different port
			optVec[i].inertalSeed = 0xaabbccddee;
			optVec[i].inertalSeed1 = 0xaabbccddee;
		}
		else
		{
			optVec[i].addr = ipRmt;
			optVec[i].port = port;		//use server port
			optVec[i].localParty.addr = ipRmt;
			optVec[i].localParty.port = port+1; 	//client use different port
			optVec[i].localParty.role = i;
			optVec[i].rmtParty.addr = ip;
			optVec[i].rmtParty.port = port;
		}

		//init ip vector &port vector
		optVec[i].ipVec.resize(2);
		optVec[i].ipVec[role % 2] = ip;
		optVec[i].ipVec[(role + 1) % 2] = ipRmt;
		optVec[i].portVec.push_back(port);
		optVec[i].portVec.push_back(port+1); 	//client use different port

		optVec[i].rmtParty.role = (i + 1) % 2;
		optVec[i].rltFn = "rlt" + std::to_string(i);
	}

	//step 2: generate training data to file.
	std::vector<float> model;  
	initLrTrainData(row, col, model);

	//read training data to opt buffer
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

	if (0 == role)
		readLrTestData(&optVec.at(0));
	else
		readLrTestData(&optVec.at(1));



	 if (1 == alg)
	{
		if (0 == role)
		{
			LOG_INFO("role=" << optVec[0].role);
			LOG_INFO("begin to run lr server ");
			logReg2PartyAlg(&optVec.at(0));
		}
		else
		{
			LOG_INFO("role=" << optVec[1].role);
			LOG_INFO("begin to run lr client ");
			logReg2PartyAlg(&optVec.at(1));
		}
	}

	LOG_INFO("lr test over.");
	return 0;
}

// add ut test cases  .Modified by wumingzi. 2022:12:17,Saturday,19:39:14.
int64_t launchThread(UtFunction &func, std::vector<OptAlg> *opt_vec, std::vector<OseOpt> *ose_vec)
{
	int64_t rlt = -1;

	if (nullptr == opt_vec)
	{
		LOG_INFO( "input  is null. exit" );
		return rlt;
	}

	int thread_num = opt_vec->size();
	LOG_INFO( "launchThread thd_num=" << thread_num );

	if (thread_num < 1)
	{
		LOG_INFO( "thread_num  is invalid=" << thread_num );
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
											   LOG_INFO( "inside thread. i=" << i << ",role=" << opt_vec->at(i).role );
											   func(&opt_vec->at(0), &ose_vec->at(0));
										   }
										   else
										   {
											   LOG_INFO( "inside thread. i=" << i << ",role=" << opt_vec->at(i).role );
											   func(&opt_vec->at(i), &ose_vec->at(i));
										   }
									   }));
	}

	for (auto &t : thrds)
		t.join();

	rlt = 0;
	return rlt;
}

void initLrTrainData(int sample_num, int feature_num, std::vector<float> &model)
{

	if (sample_num < 1 || feature_num < 2)
	{
		LOG_ERROR("input num is null..");
		return;
	}

	std::string tname = "initLrTrainData ";
	auto row = sample_num;
	auto col = feature_num;
	auto srv_col = feature_num / 2;
	auto cli_col = col - srv_col;

	LOG_INFO(tname << "row=" << row << ",col=" << col << ",srv_col=" << srv_col << ",cli_col=" << cli_col);

	std::vector<std::vector<float> > train_data_part; // one train data part
	std::vector<float> labels;						  // labels only for server, such as 0/1
	std::vector<float> param_w;
	std::vector<std::vector<float> > test_data_part;

	train_data_part.resize(row);
	test_data_part.resize(row);
	labels.resize(row);

	std::default_random_engine generator(232345);
	std::normal_distribution<double> distribution1(0, 3);

	model.resize(feature_num);
	bool print = true;
	//init model
	for (int64_t i = 0; i < feature_num; ++i)
	{
		model.at(i) = (int)fabs(distribution1(generator)) + 1;
		LOG_INFO( "model[" << i << "]=" << model.at(i) );
	}

	//get noise here
	double mNoise = 0.1;
	double mSd = 2;

	std::normal_distribution<double> distribution(mNoise, mSd);

	//init sample data
	for (int64_t i = 0; i < row; i++)
	{
		std::vector<float> &cur_row = train_data_part.at(i);
		cur_row.resize(col);
		for (int64_t j = 0; j < col; j++)
		{
			cur_row.at(j) = distribution(generator);
		}

		float cur_label = 0;
		float original_label = 0;
		vec_mul_vec(cur_row, model, cur_label);
		float noise = distribution(generator);

		original_label = cur_label + noise;
		labels.at(i) = original_label > 0;

		if (print)
		{
			LOG_INFO( "row " << i << ": label=" << labels.at(i) << ",original_label=" << original_label << ":");
			for (int64_t j = 0; j < col; j++)
			{
				LOG_INFO( cur_row.at(j) << ",");
			}
			LOG_INFO( "\n");
		}
	}

	//save server label.
	std::string model_fn = "model_data";
	std::string label_fn = "label_data";
	std::string srv_fn = "srv_data";
	std::string cli_fn = "cli_data";
	save_vec(model, model_fn);
	save_vec(labels, label_fn);
	save_mtx(train_data_part, srv_fn, 0, srv_col);
	save_mtx(train_data_part, cli_fn, srv_col, cli_col);

	//init client data
}

void readLrTestData(OptAlg *opt)
{
	std::string model_fn = "model_data";
	std::string label_fn = "label_data";
	std::string srv_fn = "srv_data";
	std::string cli_fn = "cli_data";

	auto role = opt->role;

	std::vector<std::vector<float> > srv_mtx;
	std::vector<float> srv_label;
	std::vector<float> srv_model;

	//here to allocate buf
	double **sampleDataBuf = nullptr;
	double *lableDataBuf = nullptr;

	if (0 == role)
	{

		readFloatDataMtx(srv_fn, srv_mtx);
		LOG_INFO( "show srv data here.\n");
		show_mtx(srv_mtx);

		readFloatDataVec(label_fn, srv_label);
		LOG_INFO( "show srv label here.\n");
		show_vec(srv_label);

		readFloatDataVec(model_fn, srv_model);
		LOG_INFO( "show srv_model here.\n");
		show_vec(srv_model);
	}
	else
	{
		readFloatDataMtx(cli_fn, srv_mtx);
		LOG_INFO( "show cli data here.\n");
		show_mtx(srv_mtx);

		readFloatDataVec(label_fn, srv_label);
		LOG_INFO( "show srv label here.\n");
		show_vec(srv_label);

		readFloatDataVec(model_fn, srv_model);
		LOG_INFO( "show cli_model here.\n");
		show_vec(srv_model);
	}
	//here init srv data.
	auto data_row = srv_mtx.size();
	auto label_row = srv_label.size();
	int data_col = 0;

	LOG_INFO( "role=" << role << ", data_row=" << data_row << ",label_row=" << label_row );

	if (data_row < 1)
	{
		LOG_INFO( "data_row is error=" << data_row );
		return;
	}
	data_col = srv_mtx.at(0).size();
	if (data_col < 1)
	{
		LOG_INFO( "data_col is error=" << data_col );
		return;
	}

	LOG_INFO( " data_row=" << data_row << ",label_row=" << label_row << ",data_col=" << data_col );

	sampleDataBuf = allocateDoubleMtxFull(data_row, data_col);
	lableDataBuf = allocateDoubleVec(data_row);

	//set opt data buf.
	opt->sampleDataBuf = sampleDataBuf;
	opt->lableDataBuf = lableDataBuf;
	opt->sampleNum = data_row;
	opt->featureNum = data_col;

	//here copy data to buf
	for (int64_t i = 0; i < data_row; i++)
	{
		std::vector<float> &cur_row = srv_mtx.at(i);
		auto cur_len = cur_row.size();
		for (int64_t j = 0; j < cur_len; j++)
		{
			sampleDataBuf[i][j] = cur_row.at(j);
		}

		if (i < srv_label.size())
		{
			lableDataBuf[i] = srv_label.at(i);
		}
		else
		{
			lableDataBuf[i] = 0;
		}
	}

	show_mtx_buf(opt->sampleDataBuf, data_row, data_col, "sample buf");
	show_vec_buf(opt->lableDataBuf, data_row, "label buf");
	return;
}

void readFloatDataMtx(std::string fn, std::vector<std::vector<float> > &mtx)
{
	std::vector<std::vector<std::string> > str_mtx;

	auto read_rlt = getStrMtxFromCsvFile(fn, str_mtx);
	auto row = str_mtx.size();
	mtx.resize(row);
	for (int64_t i = 0; i < row; i++)
	{
		std::vector<std::string> &cur_row = str_mtx.at(i);
		std::vector<float> f_row;

		auto col = cur_row.size();
		f_row.resize(col);
		float val = 0;
		for (int64_t j = 0; j < col; j++)
		{
			std::string str = cur_row.at(j);
			try
			{
				val = std::stod(str);
			}
			catch (std::exception &e)
			{
				val = 0;
				LOG_INFO( "std::stod error:" << e.what() << ",with input=" << str );
			}
			f_row.at(j) = val;
		}

		mtx.at(i) = f_row;
	}
}

void readFloatDataVec(std::string fn, std::vector<float> &vec)
{
	std::vector<std::vector<std::string> > str_mtx;

	auto read_rlt = getStrMtxFromCsvFile(fn, str_mtx);
	auto row = str_mtx.size();
	vec.resize(row);
	for (int64_t i = 0; i < row; i++)
	{
		std::vector<std::string> &cur_row = str_mtx.at(i);

		float val = 0;

		std::string str = cur_row.at(0);
		try
		{
			val = std::stod(str);
		}
		catch (std::exception &e)
		{
			val = 0;
			LOG_INFO( "std::stod error:" << e.what() << ",with input=" << str );
		}
		vec.at(i) = val;
	}
}

void initLrOptBuf(OptAlg *opt, double **sampleDataBuf, double *labelDataBuf, int row, int col)
{
	if (nullptr == opt)
	{
		LOG_INFO( "initLrOptBuf input opt error.\n");
		return;
	}
	opt->lableDataBuf = labelDataBuf;
	opt->sampleDataBuf = sampleDataBuf;
	opt->sampleNum = row;
	opt->featureNum = col;

	return;
}
