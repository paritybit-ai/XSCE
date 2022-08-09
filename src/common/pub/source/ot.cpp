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
 * @file ot.cpp
 * @author Created by wumingzi. 2022:05:11,Wednesday,23:09:57.
 * @brief 
 * @version 
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ot.h"
#include "Defines.h"
#include "toolkits/util/include/xlog.h"

namespace xsce_ose
{

	using namespace std;
	using namespace oc;

	int getLog2(unsigned int x)
	{
		static const unsigned char log2[256] = {
			0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
		int l = -1;
		while (x >= 256)
		{
			l += 8;
			x >>= 8;
		}
		return l + log2[x];
	}

	// first calculte the next 2^m=x, then return m.  .Modified by wumingzi/wumingzi. 2021:07:13,Tuesday,13:57:28.
	int getNextLog2(unsigned int x)
	{
		return getLog2(nextPowOf2(x));
	}

	int64_t NcoOtSend(NcoOtOpt &opt)
	{
		int64_t rlt = -1;

		std::vector<uint64_t> seed(2);

		uint64_t *msgBuf = (uint64_t *)opt.dataBuf;
		int64_t sendMsgCnt = opt.dataNum;
		int64_t msgSize = opt.dataByteLen / 8;
		std::string addr = opt.ip + ":" + std::to_string(opt.port);

		seed[0] = opt.seedBaseOt;
		seed[1] = opt.seedOprf;
		LOG_INFO("1-out-of-N OT: snd addr=" << addr << ",msg Cnt=" << sendMsgCnt << ",msgSize=" << msgSize << ",msgBuf=" << msgBuf);

		if (nullptr == msgBuf)
		{
			LOG_ERROR("NcoOtSend msg buf is invalid.exit");
			return rlt;
		}

		if (sendMsgCnt < 1)
		{
			LOG_ERROR("NcoOtSend sendMsgCnt is invalid.exit");
			return rlt;
		}

		rlt = OT_NChooseOne_Util_Send(addr, seed, msgBuf, sendMsgCnt, msgSize);

		LOG_INFO("ot nt send over. rlt=" << rlt);

		return rlt;
	}

	int64_t NcoOtRecv(NcoOtOpt &opt)
	{
		int rlt = -1;
		std::string addr = opt.rmtIp + ":" + std::to_string(opt.rmtPort);

		int64_t chooseCnt = opt.chooseIndex.size();
		LOG_INFO("1-out-of-N OT: rcv addr=" << addr << ",choose msg Cnt=" << chooseCnt << ",");

		if (chooseCnt < 1)
		{
			LOG_ERROR("NcoOtRecv size=" << chooseCnt << " is invalid.exit");
			return rlt;
		}
		int64_t chooseMsgCnt = 1;
		uint64_t choice = opt.chooseIndex[0];

		LOG_INFO("ot nt choose . choice=" << choice);

		OT_NChooseOne_Util_Receive(addr, chooseMsgCnt, choice, opt.rltIndexVec);

		int64_t len = opt.rltIndexVec.size();
		for (int64_t i = 0; i < len; i++)
		{
			LOG_INFO(i << ":" << opt.rltIndexVec.at(i));
		}
		LOG_INFO("ot nt choose over. len=" << len);

		return rlt;
	}

	int64_t OT_NChooseOne_Util_Send(std::string &ip_port, std::vector<uint64_t> &seed, uint64_t *sendMessages, int64_t msgCnt, int64_t msgSize)
	{
		//use 128 bit block.
		int64_t rlt = -1;
		uint64_t numChosenMsgs = msgCnt;
		uint64_t msgLenDword = msgSize;

		LOG_INFO("OT_NChooseOne_Util_Send. send msg cnt=" << numChosenMsgs << ",msg len=" << msgLenDword << ",msg size=" << msgSize << " bytes");
		if (numChosenMsgs < 1)
		{
			LOG_ERROR("OT_NChooseOne_Util_Send: numChosenMsgs is invalid = " << numChosenMsgs);
			return rlt;
		}
		if (nullptr == sendMessages)
		{
			LOG_ERROR("OT_NChooseOne_Util_Send: sendMessages is invalid = " << sendMessages);
			return rlt;
		}

		bool useInputRand;
		block seedBlock;
		if (seed.size() < 2)
		{
			useInputRand = false;
		}
		else
		{
			useInputRand = true;
			seedBlock = toBlock(seed[0], seed[1]);
		}

		LOG_INFO("msgs init begin");

		Matrix<block> msgs(1, numChosenMsgs);

		LOG_INFO("msgs init ok");

		uint64_t base = 0;
		for (uint64_t i = 0; i < numChosenMsgs; ++i)
		{
			base = i * msgLenDword;
			msgs[0][i] = toBlock(sendMessages[base + 1], sendMessages[base + 0]);
		}
		LOG_INFO("begin call ot");

		OT_NChooseOne_Util_Send_impl(ip_port, useInputRand, seedBlock, msgs);

		return 0;
	}

	void OT_NChooseOne_Util_Receive(std::string &ip_port, uint64_t numChosenMsgs, uint64_t choice, std::vector<uint64_t> &recvMsg)
	{
		if (numChosenMsgs == 0)
		{
			return;
		}
		if (choice >= numChosenMsgs)
		{
			//throw BadParameterException();
		}

		std::vector<u64> choices(1);
		choices[0] = choice;
		std::vector<block> recvMsgs(1);

		OT_NChooseOne_Util_Receive_impl(ip_port, numChosenMsgs, choices, recvMsgs);
		block bMsg = recvMsgs[0];
		recvMsg.resize(2);
		recvMsg[0] = bMsg.as<std::uint64_t>()[0];
		recvMsg[1] = bMsg.as<std::uint64_t>()[1];
	}

	void OT_NChooseOne_Util_Send_impl(std::string &ip_port, bool useInputRand, block &seed, Matrix<block> &sendMessages)
	{
		LOG_INFO("OT_NChooseOne_Util_Send_impl input. addr=" << ip_port << ",seed=" << seed);
		// get up the networking
		auto rr = SessionMode::Server;
		IOService ios;
		Endpoint ep(ios, ip_port, rr, "xsce-pir");
		PRNG prng;
		if (useInputRand)
			prng.SetSeed(seed);
		else
			prng.SetSeed(sysRandomSeed());

		Channel chl = ep.addChannel();

		osuCrypto::OosNcoOtSender sender;

		LOG_INFO("begin to configure");
		// all Nco Ot extenders must have configure called first. This determines
		// a variety of parameters such as how many base OTs are required.
		bool maliciousSecure = true;
		u64 statSecParam = 40;
		u64 inputBitCount = 76; // the kkrt protocol default to 128 but oos can only do 76.
		sender.configure(maliciousSecure, statSecParam, inputBitCount);

		// Generate new base OTs for the first extender. This will use
		// the default BaseOT protocol. You can also manually set the
		// base OTs with setBaseOts(...);

		LOG_INFO("begin to gen base ot");
		sender.genBaseOts(prng, chl);

		// send
		PRNG prng1(sysRandomSeed());

		// populate this with the messages that you want to send.
		//Matrix<block> sendMessages(numOTs, numChosenMsgs);
		//prng.get(sendMessages.data(), sendMessages.size());

		LOG_INFO("begin to send extension ot.");
		// perform the OTs with the given messages.
		sender.sendChosen(sendMessages, prng, chl);
		LOG_INFO("end to  send extension ot.");
		chl.close();
		ep.stop();
		ios.stop();
	}

	void OT_NChooseOne_Util_Receive_impl(std::string ip_port, int numChosenMsgs, std::vector<u64> &choices, std::vector<block> &recvMsgs)
	{
		// get up the networking
		auto rr = SessionMode::Client;
		IOService ios;
		Endpoint ep(ios, ip_port, rr, "xsce-pir");
		PRNG prng(sysRandomSeed());
		PRNG prng1(sysRandomSeed());

		// for each thread we need to construct a channel (socket) for it to communicate on.
		Channel chl = ep.addChannel();

		osuCrypto::OosNcoOtReceiver recver;

		// all Nco Ot extenders must have configure called first. This determines
		// a variety of parameters such as how many base OTs are required.
		bool maliciousSecure = true;
		u64 statSecParam = 40;
		u64 inputBitCount = 76; // the kkrt protocol default to 128 but oos can only do 76.
		recver.configure(maliciousSecure, statSecParam, inputBitCount);

		// Generate new base OTs for the first extender. This will use
		// the default BaseOT protocol. You can also manually set the
		// base OTs with setBaseOts(...);
		recver.genBaseOts(prng, chl);

		// the messages that were learned are written to recvMsgs.
		recver.receiveChosen(numChosenMsgs, recvMsgs, choices, prng, chl);
		LOG_INFO("ot rcv over.");

		if (recvMsgs.size() > 0)
		{
			LOG_INFO("ot rcv rlt=" << recvMsgs.at(0));
		}

		chl.close();
		ep.stop();
		ios.stop();
	}

	// N-1 OT functions  .Modified by wumingzi/wumingzi. 2021:07:13,Tuesday,11:29:55.
	void setRecvBaseOtsSeed(NcoOtExtReceiver &recv,
							Channel &recvChl, u64 baseCnt, u64 seed)
	{
		u64 baseCount = baseCnt;

		std::vector<block> baseRecv(baseCount);
		std::vector<std::array<block, 2>> baseSend(baseCount);
		BitVector baseChoice(baseCount);
		PRNG prng0(oc::toBlock(seed));
		baseChoice.randomize(prng0);

		prng0.get((u8 *)baseSend.data()->data(), sizeof(block) * 2 * baseSend.size());

		recv.setBaseOts(baseSend, prng0, recvChl);
	}

	void setSenderBaseOtsSeed(NcoOtExtSender &sender,
							  Channel &sendChl,
							  u64 baseCnt, u64 seed)
	{
		u64 baseCount = sender.getBaseOTCount();

		std::vector<block> baseRecv(baseCount);
		std::vector<std::array<block, 2>> baseSend(baseCount);
		BitVector baseChoice(baseCount);
		PRNG prng0(oc::toBlock(seed));
		baseChoice.randomize(prng0);

		prng0.get((u8 *)baseSend.data()->data(), sizeof(block) * 2 * baseSend.size());
		for (u64 i = 0; i < baseCount; ++i)
		{
			baseRecv[i] = baseSend[i][baseChoice[i]];
		}

		sender.setBaseOts(baseRecv, baseChoice, sendChl);
	}

	int NcoOtSendCh(NcoOtOpt &opt)
	{
		int rlt = 0;
		std::string ip = opt.ip;
		uint32_t port = opt.port;
		u64 numOTs = opt.numOTs;
		u64 inputMsgCnt = opt.msgCnt;
		uint64_t seed = opt.seedOprf;
		uint64_t seed2 = opt.seedBaseOt;
		uint32_t role = opt.role;
		uint32_t dataByteLen = opt.dataByteLen;
		uint32_t statSecuKey = opt.stasSecuKey;
		unsigned char *dataBuf = opt.dataBuf;
		bool isServer = true;

		// first check input parameters.  .Modified by wumingzi/wumingzi. 2021:07:13,Tuesday,13:59:32.
		if (opt.maxMsgCnt > 0 && inputMsgCnt > opt.maxMsgCnt)
		{
			inputMsgCnt = opt.maxMsgCnt;
			LOG_INFO("input msg cnt is too big, set to max limit=" << inputMsgCnt);
		}

		if (opt.maxNumOt > 0 && numOTs > opt.maxNumOt)
		{
			numOTs = opt.maxNumOt;
			LOG_INFO("input otNumb is too big, set to max limit=" << numOTs);
		}

		if (numOTs < 1)
		{
			LOG_ERROR("NcoOtSend input numOTs is error:" << numOTs);
			return ERROR_RLT;
		}

		if (inputMsgCnt < 2)
		{
			LOG_ERROR("NcoOtSend input inputMsgCnt is error:" << inputMsgCnt);
			return ERROR_RLT;
		}

		if (nullptr == dataBuf)
		{
			LOG_ERROR("NcoOtSend input databuf is null");
			return ERROR_RLT;
		}

		if (1 == opt.role)
		{
			isServer = false;
		}

		if (!isServer) //chooser
		{
			if (numOTs != opt.chooseIndex.size())
			{
				LOG_ERROR("NcoOtSend input choice vec size is not the same as numOTs.");
				return ERROR_RLT;
			}
		}

		if (isServer)
		{
			if (dataByteLen != 8 && dataByteLen != 16)
			{
				LOG_ERROR("NcoOtSend input dataByteLen is error:" << dataByteLen);
				return ERROR_RLT;
			}
		}

		if (statSecuKey < 40)
		{
			statSecuKey = 40;
		}
		LOG_INFO("this is NcoOt send func");

		u64 inputSize = getNextLog2(inputMsgCnt);
		oc::block blkOTSeed = sysRandomSeed();
		LOG_INFO("ot send seed=" << blkOTSeed);
		PRNG prng0(oc::toBlock(4253465, seed));
		PRNG prng1(blkOTSeed);

		LOG_INFO("NcoOt, ip:" << ip << ":" << port << ",numOt=" << numOTs
				  << ",size=" << inputSize << ",statSecuKey=" << statSecuKey
				  << ",seed=" << seed << ",role=" << role << ",seed=" << seed << ",seed2=" << seed2);

		oc::OosNcoOtSender sender;
		sender.configure(true, statSecuKey, inputSize);

		IOService ios;
		oc::Session ep0(ios, ip, port, SessionMode::Server);
		auto sendChl = ep0.addChannel();

		uint64_t baseCount = sender.getBaseOTCount();
		setSenderBaseOtsSeed(sender, sendChl, baseCount, seed2);

		auto messageCount = 1ull << inputSize;
		LOG_INFO("numOTs=" << numOTs << ",inputMsgCnt=" << inputMsgCnt << ",dataByteLen=" << dataByteLen << ",msg count=" << messageCount);

		Matrix<block> sendMessage(numOTs, messageCount);
		std::vector<block> recvMessage(numOTs);

		uint64_t row = numOTs;
		uint64_t col = messageCount;
		uint64_t total = sendMessage.size();

		LOG_INFO("Nco OT send. row=" << row << ",col=" << col << ",total=" << total << ",row*col=" << row * col);

		//fill send data.
		u64 lowData, highData;
		lowData = 0x55555555;
		highData = 0xAAAAAAAA;
		u64 base = 0;
		u64 offset = 0;
		u64 showRow = 4;
		u64 showCol = 10;

		for (uint32_t i = 0; i < numOTs; i++)
		{
			base = 0; //means each numOT round use the same mssage inpu data.

			for (uint32_t j = 0; j < messageCount; j++)
			{
				offset = j * dataByteLen + base;
				if (j >= inputMsgCnt) //padding send msg.
				{
					if (8 == dataByteLen)
					{
						sendMessage(i, j) = oc::toBlock(lowData);
					}
					else if (16 == dataByteLen)
					{
						sendMessage(i, j) = oc::toBlock(highData, lowData);
					}
				}
				else
				{

					if (8 == dataByteLen)
					{
						lowData = *(uint64_t *)(dataBuf + offset);
						sendMessage(i, j) = oc::toBlock(lowData);
					}
					else if (16 == dataByteLen)
					{
						lowData = *(uint64_t *)(dataBuf + offset);
						highData = *(uint64_t *)(dataBuf + offset + 8);
						sendMessage(i, j) = oc::toBlock(highData, lowData);
					}
				}
			}
		}

		for (u64 i = 0; i < showRow && i < numOTs; i++)
		{
			for (u64 j = 0; j < showCol && j < messageCount; j++)
			{
				LOG_INFO("send msg: row=" << i << ",col=" << j << ", data=" << sendMessage(i, j));
			}
		}

		//for sender & chooser, need to use different prng seed.
		sender.sendChosen(sendMessage, prng1, sendChl);

		return rlt;
	}

	int NcoOtRecvCh(NcoOtOpt &opt)
	{
		int rlt = 0;
		LOG_INFO("this is NcoOt recv func");

		std::string ip = opt.ip;
		uint32_t port = opt.port;
		u64 numOTs = opt.numOTs;
		u64 inputMsgCnt = opt.msgCnt;
		uint64_t seed = opt.seedOprf;
		uint64_t seed2 = opt.seedBaseOt;
		uint32_t role = opt.role;
		uint32_t dataByteLen = opt.dataByteLen;
		uint32_t statSecuKey = opt.stasSecuKey;
		unsigned char *dataBuf = opt.dataBuf;
		bool isServer = true;
		// first check input parameters.  .Modified by wumingzi/wumingzi. 2021:07:13,Tuesday,13:59:32.
		if (opt.maxMsgCnt > 0 && inputMsgCnt > opt.maxMsgCnt)
		{
			inputMsgCnt = opt.maxMsgCnt;
			LOG_INFO("input msg cnt is too big, set to max limit=" << inputMsgCnt);
		}

		if (opt.maxNumOt > 0 && numOTs > opt.maxNumOt)
		{
			numOTs = opt.maxNumOt;
			LOG_INFO("input otNumb is too big, set to max limit=" << numOTs);
		}

		if (numOTs < 1)
		{
			LOG_ERROR("NcoOtSend input numOTs is error:" << numOTs);
			return ERROR_RLT;
		}

		if (inputMsgCnt < 2)
		{
			LOG_ERROR("NcoOtSend input inputMsgCnt is error:" << inputMsgCnt);
			return ERROR_RLT;
		}

		if (nullptr == dataBuf)
		{
			LOG_ERROR("NcoOtSend input databuf is null");
			return ERROR_RLT;
		}

		if (1 == opt.role)
		{
			isServer = false;
		}

		if (!isServer)
		{
			if (numOTs != opt.chooseIndex.size())
			{
				LOG_ERROR("NcoOtSend input choice vec size is not the same as numOTs.");
				return ERROR_RLT;
			}
		}

		if (isServer)
		{
			if (dataByteLen != 8 && dataByteLen != 16)
			{
				LOG_ERROR("NcoOtSend input dataByteLen is error:" << dataByteLen);
				return ERROR_RLT;
			}
		}

		if (statSecuKey < 40)
		{
			statSecuKey = 40;
		}

		u64 inputSize = getNextLog2(inputMsgCnt);

		oc::block blkOTSeed = sysRandomSeed();
		LOG_INFO("ot rcv seed=" << blkOTSeed);
		PRNG prng2(blkOTSeed);

		PRNG prng1(oc::toBlock(42532335, seed));

		LOG_INFO("NcoOt, ip:" << ip << ":" << port << ",numOt=" << numOTs
				  << ",size=" << inputSize << ",statSecuKey=" << statSecuKey
				  << ",seed=" << seed << ",role=" << role << ",seed=" << seed << ",seed2=" << seed2);

		oc::OosNcoOtReceiver recv;
		recv.configure(true, statSecuKey, inputSize);

		IOService ios;
		oc::Session ep1(ios, ip, port, SessionMode::Client);
		auto recvChl = ep1.addChannel();
		uint64_t baseCount = recv.getBaseOTCount();

		setRecvBaseOtsSeed(recv, recvChl, baseCount, seed2);

		auto messageCount = 1ull << inputSize;
		LOG_INFO("numOTs=" << numOTs << ",msg count=" << messageCount);

		std::vector<block> recvMessage(numOTs);

		std::vector<u64> choices(numOTs);

		for (u64 i = 0; i < choices.size(); ++i)
		{
			choices[i] = opt.chooseIndex.at(i) % inputMsgCnt;
		}

		//for sender & chooser, need to use different prng seed.
		recv.receiveChosen(messageCount, recvMessage, choices, prng2, recvChl);

		for (u64 i = 0; i < choices.size(); ++i)
		{
			LOG_INFO("choice[" << i << "]=" << choices[i] << ",rcv msg=" << recvMessage[i]);
		}

		//save recv data to databuf
		unsigned char *base = 0;
		for (u64 i = 0; i < choices.size(); ++i)
		{
			base = dataBuf + i * dataByteLen;
			if (8 == dataByteLen)
			{
				*(uint64_t *)base = recvMessage.at(i).as<std::uint64_t>()[0];
			}
			else if (16 == dataByteLen)
			{
				*(uint64_t *)base = recvMessage.at(i).as<std::uint64_t>()[0];
				*(uint64_t *)(base + 8) = recvMessage.at(i).as<std::uint64_t>()[1];
			}
		}
		return rlt;
	}

	int64_t exchangeSeed(int role, std::string ip, int port, uint8_t *seed, uint64_t len, const char *pFLag)
	{
		Curve curve;
		Point point(curve);

		int ecclen = point.sizeBytes();
		LOG_INFO("ecclen: " << ecclen);
		u8 pointSend[ecclen];
		u8 pointRcv[ecclen];
		auto order = curve.getGenerator();

		if (len < 8)
		{
			LOG_ERROR("exchangeSeed input seed len is too short, should be 8 bytes at least");
			return -1;
		}

		if (len > 16)
		{
			LOG_ERROR("exchangeSeed input seed len is too long, should be 16 bytes at most");
			return -1;
		}

		uint64_t seedInputLen = len / 4;

		int *seedNum;
		seedNum = (int *)seed;

		Number num1(curve);
		Number num2(curve);
		Number numTemp(curve);

		for (int64_t i = 0; i < 2; i++)
		{

			int seedFactor = seedNum[i];
			if (seedFactor < 1)
				seedFactor = 1;
			numTemp = seedFactor;
			num1 += numTemp;
		}

		if (seedInputLen > 3)
		{	for (int64_t i = 2; i < 4; i++)
			{
				int seedFactor = seedNum[i];
				if (seedFactor < 1)
					seedFactor = 1;
				numTemp = seedFactor;
				num2 += numTemp;
			}
		}

		Point p1(curve), p2(curve), p3(curve), p4(curve);
		p1 = order * num1;
		p1 = p1 * num2;

		p1.toBytes(pointSend);

		IOService ios;
		std::string ipaddr;
		EpMode mode;
		if (0 == role)
		{
			ipaddr = "0.0.0.0:" + to_string(port);
			mode = EpMode::Server;
		}
		else
		{
			ipaddr = ip + ":" + to_string(port);
			mode = EpMode::Client;
		}
		string strSessionName = "exchange-seed";
		if (NULL != pFLag)
		{
			strSessionName = strSessionName + ":" + pFLag;
		}
		LOG_INFO("exchangeSeed-sessionname:" << strSessionName);
		Endpoint ep(ios, ipaddr, mode, strSessionName);
		Channel ch = ep.addChannel();

		ch.send(pointSend, ecclen);
		ch.recv(pointRcv, ecclen);
		p2.fromBytes(pointRcv);

		//here to generate secret result
		p3 = p2 * num1;
		p3 = p3 * num2;

		p3.toBytes(pointSend);
		unsigned char hash[16];

		getMd5Char((unsigned char *)pointSend, ecclen, (unsigned char *)hash);

		int bufLen = 8;
		if (16 == len)
			bufLen = 16;

		for (int i = 0; i < bufLen; i++)
		{
			seed[i] = hash[i];
		}

		ch.close();
		ep.stop();
		ios.stop();

		return 0;
	}

	int getMd5Char(unsigned char *input, uint32_t len, unsigned char *output)
	{
		MD5_CTX x;

		MD5_Init(&x);
		MD5_Update(&x, (char *)input, len);
		MD5_Final(output, &x);

		return 0;
	}

	//tcp data send function
	int64_t srvSendBuf(NcoOtOpt &opt, uint64_t *buf, uint64_t len)
	{
		std::string ip = opt.ip;
		uint32_t port = opt.port;

		IOService ios;
		oc::Session ep0(ios, ip, port, SessionMode::Server);
		auto sendChl = ep0.addChannel();
		LOG_INFO("srvSendBuf sending unit64_t len=" << len);
		sendChl.send(len);
		sendChl.send(buf, len);
		return 0;
	}

	int64_t clientRcvBuf(NcoOtOpt &opt, uint64_t **buf, uint64_t *len)
	{
		int64_t rlt = -1;

		std::string ip = opt.ip;
		uint32_t port = opt.port;

		IOService ios;
		oc::Session ep1(ios, ip, port, SessionMode::Client);
		auto recvChl = ep1.addChannel();

		uint64_t rcvLen = 0;
		recvChl.recv(rcvLen);
		LOG_INFO("srvSendBuf rcving unit64_t len=" << rcvLen);
		*len = rcvLen;

		if (rcvLen < 1)
		{
			return rlt;
		}

		uint64_t *dataBuf = (uint64_t *)calloc(rcvLen, sizeof(uint64_t));

		if (nullptr == dataBuf)
		{
			LOG_ERROR("databuf allocate error");
			return rlt;
		}

		recvChl.recv(dataBuf, rcvLen);

		*buf = dataBuf;
		LOG_INFO("clientRcvBuf, dataBuf=" << dataBuf);

		return 0;
	}

}
