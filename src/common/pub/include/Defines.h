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

#pragma once

#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Crypto/RCurve.h>
#include <cryptoTools/Crypto/RandomOracle.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/Timer.h>

#include <libOTe/TwoChooseOne/IknpOtExtSender.h>
#include <libOTe/TwoChooseOne/IknpOtExtReceiver.h>

#define OPRF_PSI_STEP1 "[ Transfer base OT ]"
#define OPRF_PSI_STEP2 "[ Transfer set transformed ]"
#define OPRF_PSI_STEP3 "[ Transfer matrix  and transposed ]"
#define OPRF_PSI_STEP4 "[ Hash outputs compute ]"
#define OPRF_PSI_STEP5 "[ Oprf output compare ]"
#define OPRF_PSI_STEP6 "[ Oprf psi over ]"

namespace util
{
	using i64 = oc::i64;
	using u64 = oc::u64;
	using i32 = oc::i32;
	using u32 = oc::u32;
	using i16 = oc::i16;
	using u16 = oc::u16;
	using i8 = oc::i8;
	using u8 = oc::u8;
	using block = oc::block;

	using BitVector = oc::BitVector;

	template <typename T>
	using span = oc::span<T>;

	using PRNG = oc::PRNG;
	using AES = oc::AES;
	using AESDec = oc::AESDec;
	using RandomOracle = oc::RandomOracle;

	using IknpOtExtSender = oc::IknpOtExtSender;
	using IknpOtExtReceiver = oc::IknpOtExtReceiver;

	using IOService = oc::IOService;
	using Endpoint = oc::Endpoint;
	using Channel = oc::Channel;
	using EpMode = oc::EpMode;

	using TimeUnit = oc::Timer::timeUnit;
	using Timer = oc::Timer;

	using Curve = osuCrypto::REllipticCurve;
	using Point = osuCrypto::REccPoint;
	using Number = osuCrypto::REccNumber;
} // namespace util
