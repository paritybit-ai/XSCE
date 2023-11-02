#include <iostream>
#include "c_offline_interface.h"
#include "offline_interface.h"

offline_cpp::OprfArgs OprfArgsToCpp(OprfArgs args) {
    // cpp args有默认值，需要先判断args中字段是否设置，避免覆盖默认值;
    offline_cpp::OprfArgs cpp_args;
    if (args.hashLen != 0) {
        cpp_args.hashLen = args.hashLen;
    }
    if (args.width != 0) {
        cpp_args.width = args.width;
    }
    if (args.logHeight != 0) {
        cpp_args.logHeight = args.logHeight;
    }
    if (args.height != 0) {
        cpp_args.height = args.height;
    }
    if (args.hashLengthInBytes != 0) {
        cpp_args.hashLengthInBytes = args.hashLengthInBytes;
    }
    if (args.hash1LengthInBytes != 0) {
        cpp_args.hash1LengthInBytes = args.hash1LengthInBytes;
    }
    if (args.bucket1 != 0) {
        cpp_args.bucket1 = args.bucket1;
    }
    if (args.bucket2 != 0) {
        cpp_args.bucket2 = args.bucket2;
    }
    if (args.commonSeed != 0) {
        cpp_args.commonSeed = args.commonSeed;
    }
    if (args.commonSeed1 != 0) {
        cpp_args.commonSeed1 = args.commonSeed1;
    }
    if (args.inertalSeed != 0) {
        cpp_args.inertalSeed = args.inertalSeed;
    }
    if (args.inertalSeed1 != 0) {
        cpp_args.inertalSeed1 = args.inertalSeed1;
    }
    return cpp_args;
}
OprfArgs OprfArgsFromCpp(offline_cpp::OprfArgs cpp_args) {
    OprfArgs args;
    args.hashLen = cpp_args.hashLen;
    args.width = cpp_args.width;    
    args.logHeight = cpp_args.logHeight;
    args.height = cpp_args.height;
    args.hashLengthInBytes = cpp_args.hashLengthInBytes;
    args.hash1LengthInBytes = cpp_args.hash1LengthInBytes;
    args.bucket1 = cpp_args.bucket1;
    args.bucket2 = cpp_args.bucket2;
    args.commonSeed = cpp_args.commonSeed;
    args.commonSeed1 = cpp_args.commonSeed1;
    args.inertalSeed = cpp_args.inertalSeed;
    args.inertalSeed1 = cpp_args.inertalSeed1;
    return args;
}

std::string VecToString(const VecUint8& vec) {
    std::string res(vec.data, vec.data + vec.size);
    return res;
}
VecUint8 StringToVec(const std::string& str) {
    VecUint8 res;
    res.size = str.size();
    res.data = (uint8_t*)malloc(str.size());
    memcpy((uint8_t*)res.data, (uint8_t*)str.data(), str.size());
    res.need_delete = true;
    return res;
}

// 如果没有调用到特化StructToCpp，则会链接错误，TODO: 主动引发编译错误
// 例如：T block, N c接口Block
template <typename T, typename N>
T StructToCpp(const N& data) {
    static_assert(std::is_same<T, N>::value, "Wrong struct to cpp");
    return data;
}
// 从cpp结构体，恢复C接口结构体
// 例如：T block, N c接口Block
template <typename T, typename N>
N StructFromCpp(const T& data) {
    static_assert(std::is_same<T, N>::value, "Wrong struct to cpp");
    return data;
}

template <>
oprf_psi_offline::block StructToCpp(const Block& data) {
    return oc::toBlock(data.buf1, data.buf2);
}
template <>
Block StructFromCpp(const oprf_psi_offline::block& data) {
    std::array<uint64_t, 2> tmp = data.as<uint64_t>();
    return {tmp[0], tmp[1]};
}

template <>
std::array<oprf_psi_offline::block, 2> StructToCpp(const DoubleBlock& data) {
    std::array<oprf_psi_offline::block, 2> res;
    res[0] = StructToCpp<oprf_psi_offline::block, Block>(data.block1);
    res[1] = StructToCpp<oprf_psi_offline::block, Block>(data.block2);
    return res;
}
template <>
DoubleBlock StructFromCpp(const std::array<oprf_psi_offline::block, 2>& data) {
    DoubleBlock res;
    res.block1 = StructFromCpp<oprf_psi_offline::block, Block>(data[0]);
    res.block2 = StructFromCpp<oprf_psi_offline::block, Block>(data[1]);
    return res;
}

template <>
std::pair<uint64_t, uint64_t> StructToCpp(const PairUint64& data) {
    return std::make_pair(data.data1, data.data2);
}
template <>
PairUint64 StructFromCpp(const std::pair<uint64_t, uint64_t>& data) {
    return {data.first, data.second};
}

template <typename T, typename U>
std::vector<T> VecToCpp(U vec);
template <>
std::vector<uint8_t> StructToCpp<std::vector<uint8_t>, VecUint8>(const VecUint8& data) {
    return VecToCpp<uint8_t, VecUint8>(data);
}
template <typename T, typename U>
U VecFromCpp(const std::vector<T>& vec);
template <>
VecUint8 StructFromCpp(const std::vector<uint8_t>& data) {
    return VecFromCpp<uint8_t, VecUint8>(data);
}

template <>
std::string StructToCpp<std::string, VecUint8>(const VecUint8& data) {
    return VecToString(data);
}
template <>
VecUint8 StructFromCpp(const std::string& data) {
    return StringToVec(data);
}

template <>
oprf_psi_offline::BitVector StructToCpp<oprf_psi_offline::BitVector, VecUint8>(const VecUint8& data) {
    oprf_psi_offline::BitVector res;
    res.append(data.data, data.size);
    return res;
}
template <>
VecUint8 StructFromCpp(const oprf_psi_offline::BitVector& data) {
    VecUint8 res;
    res.size = data.size();
    res.data = (uint8_t*)malloc(res.size * sizeof(uint8_t));
    res.need_delete = true;
    memcpy((uint8_t*)res.data, (uint8_t*)data.data(), res.size * sizeof(uint8_t));
    return res;
}

// 例如：T block, U：c接口VecBlock, N：c接口Block
template <typename T, typename U>
std::vector<T> VecToCpp(U vec) {
    typedef typename std::remove_pointer<decltype(vec.data)>::type N;
    std::vector<T> res;
    res.resize(vec.size);
    for (int i = 0; i < vec.size; ++i) {
        res[i] = StructToCpp<T, N>(vec.data[i]);
    }
    return res;
}
template <typename T, typename U>
U VecFromCpp(const std::vector<T>& vec) {
    typedef typename std::remove_pointer<decltype(std::declval<U>().data)>::type N;
    U res;
    res.size = vec.size();
    res.data = (N*)malloc(res.size * sizeof(N));
    res.need_delete = true;

    for (int i = 0; i < vec.size(); ++i) {
        res.data[i] = StructFromCpp<T, N>(vec[i]);
    }
    return res;
}

offline_cpp::ClientOfflineDumpData ClientOfflineDumpDataToCpp(const ClientOfflineDumpData& dump_data) {
    offline_cpp::ClientOfflineDumpData res;
    res.receiver_set = VecToCpp<oprf_psi_offline::block, VecBlock>(dump_data.receiver_set);
    res.receiver_size = dump_data.receiver_size;
    res.sender_size = dump_data.sender_size;
    res.ot_messages = VecToCpp<std::array<oprf_psi_offline::block, 2>, VecDoubleBlock>(dump_data.ot_messages);
    res.trans_hash_inputs = VecToCpp<std::vector<uint8_t>, VecVecUint8>(dump_data.trans_hash_inputs);
    res.pair_result = VecToCpp<std::pair<uint64_t, uint64_t>, VecPairUint64>(dump_data.pair_result);
    res.result = VecToCpp<uint64_t, VecUint64>(dump_data.result);
    return res;
}
ClientOfflineDumpData ClientOfflineDumpDataFromCpp(const offline_cpp::ClientOfflineDumpData& dump_data) {
    ClientOfflineDumpData res;
    res.receiver_set = VecFromCpp<oprf_psi_offline::block, VecBlock>(dump_data.receiver_set);
    res.receiver_size = dump_data.receiver_size;
    res.sender_size = dump_data.sender_size;
    res.ot_messages = VecFromCpp<std::array<oprf_psi_offline::block, 2>, VecDoubleBlock>(dump_data.ot_messages);
    res.trans_hash_inputs = VecFromCpp<std::vector<uint8_t>, VecVecUint8>(dump_data.trans_hash_inputs);
    res.pair_result = VecFromCpp<std::pair<uint64_t, uint64_t>, VecPairUint64>(dump_data.pair_result);
    res.result = VecFromCpp<uint64_t, VecUint64>(dump_data.result);
    return res;
}

offline_cpp::ClientOfflineInput ClientInputToCpp(const ClientOfflineInput& input) {
    offline_cpp::ClientOfflineInput res;
    res.args = OprfArgsToCpp(input.args);
    res.recv_data = VecToCpp<uint8_t, VecUint8>(input.recv_data);
    // res.ids = VecVecToVecString(input.ids);
    res.ids = VecToCpp<std::string, VecVecChar>(input.ids);
    res.dump_data = ClientOfflineDumpDataToCpp(input.dump_data);
    return res;
}
ClientOfflineInput ClientInputFromCpp(const offline_cpp::ClientOfflineInput& input) {
    ClientOfflineInput res;
    res.args = OprfArgsFromCpp(input.args);
    res.recv_data = VecFromCpp<uint8_t, VecUint8>(input.recv_data);
    // res.ids = VecVecFromVecString(input.ids);
    res.ids = VecFromCpp<std::string, VecVecChar>(input.ids);
    res.dump_data = ClientOfflineDumpDataFromCpp(input.dump_data);
    return res;
}
offline_cpp::ClientOfflineOutput ClientOutputToCpp(const ClientOfflineOutput& output) {
    offline_cpp::ClientOfflineOutput res;
    res.dump_data = ClientOfflineDumpDataToCpp(output.dump_data);
    res.send_data = VecToCpp<uint8_t, VecUint8>(output.send_data);
    return res;
}
ClientOfflineOutput ClientOutputFromCpp(const offline_cpp::ClientOfflineOutput& output) {
    ClientOfflineOutput res;
    res.dump_data = ClientOfflineDumpDataFromCpp(output.dump_data);
    res.send_data = VecFromCpp<uint8_t, VecUint8>(output.send_data);
    return res;
}

offline_cpp::ServerOfflineDumpData ServerOfflineDumpDataToCpp(const ServerOfflineDumpData& dump_data) {
    offline_cpp::ServerOfflineDumpData res;
    res.sender_set = VecToCpp<oprf_psi_offline::block, VecBlock>(dump_data.sender_set);
    res.sender_size = dump_data.sender_size;
    res.receiver_size = dump_data.receiver_size;
    res.ot_messages = VecToCpp<oprf_psi_offline::block, VecBlock>(dump_data.ot_messages);
    res.choices = StructToCpp<oprf_psi_offline::BitVector, VecUint8>(dump_data.choices);
    res.trans_hash_inputs = VecToCpp<std::vector<uint8_t>, VecVecUint8>(dump_data.trans_hash_inputs);
    res.result = VecToCpp<uint64_t, VecUint64>(dump_data.result);
    return res;
}
ServerOfflineDumpData ServerOfflineDumpDataFromCpp(const offline_cpp::ServerOfflineDumpData& dump_data) {
    ServerOfflineDumpData res;
    res.sender_set = VecFromCpp<oprf_psi_offline::block, VecBlock>(dump_data.sender_set);
    res.sender_size = dump_data.sender_size;
    res.receiver_size = dump_data.receiver_size;
    res.ot_messages = VecFromCpp<oprf_psi_offline::block, VecBlock>(dump_data.ot_messages);
    res.choices = StructFromCpp<oprf_psi_offline::BitVector, VecUint8>(dump_data.choices);
    res.trans_hash_inputs = VecFromCpp<std::vector<uint8_t>, VecVecUint8>(dump_data.trans_hash_inputs);
    res.result = VecFromCpp<uint64_t, VecUint64>(dump_data.result);
    return res;
}

offline_cpp::ServerOfflineInput ServerInputToCpp(const ServerOfflineInput& input) {
    offline_cpp::ServerOfflineInput res;
    res.args = OprfArgsToCpp(input.args);
    res.recv_data = VecToCpp<uint8_t, VecUint8>(input.recv_data);
    // res.ids = VecVecToVecString(input.ids);
    res.ids = VecToCpp<std::string, VecVecChar>(input.ids);
    res.dump_data = ServerOfflineDumpDataToCpp(input.dump_data);
    return res;
}
ServerOfflineInput ServerInputFromCpp(const offline_cpp::ServerOfflineInput& input) {
    ServerOfflineInput res;
    res.args = OprfArgsFromCpp(input.args);
    res.recv_data = VecFromCpp<uint8_t, VecUint8>(input.recv_data);
    // res.ids = VecVecFromVecString(input.ids);
    res.ids = VecFromCpp<std::string, VecVecChar>(input.ids);
    res.dump_data = ServerOfflineDumpDataFromCpp(input.dump_data);
    return res;
}
offline_cpp::ServerOfflineOutput ServerOutputToCpp(const ServerOfflineOutput& output) {
    offline_cpp::ServerOfflineOutput res;
    res.dump_data = ServerOfflineDumpDataToCpp(output.dump_data);
    res.send_data = VecToCpp<uint8_t, VecUint8>(output.send_data);
    return res;
}
ServerOfflineOutput ServerOutputFromCpp(const offline_cpp::ServerOfflineOutput& output) {
    ServerOfflineOutput res;
    res.dump_data = ServerOfflineDumpDataFromCpp(output.dump_data);
    res.send_data = VecFromCpp<uint8_t, VecUint8>(output.send_data);
    return res;
}


template <typename T>
void ReleaseStruct(T data) {
    return;
}
template <typename T>
void ReleaseVec(T vec);
template <>
void ReleaseStruct(VecUint8 data) {
    ReleaseVec(data);
}

template <typename T>
void ReleaseVec(T vec) {
    if (!vec.need_delete || vec.data == NULL) {
        return;
    }
    for (int i = 0; i < vec.size; ++i) {
        ReleaseStruct(vec.data[i]);
    }
    free(vec.data);
    vec.data = NULL;
    vec.need_delete = false;
}
void ReleaseMemory(const OfflineStruct *input) {
    if (input == NULL) {
        return;
    }
    ReleaseVec(input->client_input.recv_data);
    ReleaseVec(input->client_input.ids);
    ReleaseVec(input->client_input.dump_data.receiver_set);
    ReleaseVec(input->client_input.dump_data.ot_messages);
    ReleaseVec(input->client_input.dump_data.trans_hash_inputs);
    ReleaseVec(input->client_input.dump_data.pair_result);
    ReleaseVec(input->client_input.dump_data.result);

    ReleaseVec(input->client_output.send_data);
    ReleaseVec(input->client_output.dump_data.receiver_set);
    ReleaseVec(input->client_output.dump_data.ot_messages);
    ReleaseVec(input->client_output.dump_data.trans_hash_inputs);
    ReleaseVec(input->client_output.dump_data.pair_result);
    ReleaseVec(input->client_output.dump_data.result);

    ReleaseVec(input->server_input.recv_data);
    ReleaseVec(input->server_input.ids);
    ReleaseVec(input->server_input.dump_data.sender_set);
    ReleaseVec(input->server_input.dump_data.ot_messages);
    ReleaseVec(input->server_input.dump_data.choices);
    ReleaseVec(input->server_input.dump_data.trans_hash_inputs);
    ReleaseVec(input->server_input.dump_data.result);

    ReleaseVec(input->server_output.send_data);
    ReleaseVec(input->server_output.dump_data.sender_set);
    ReleaseVec(input->server_output.dump_data.ot_messages);
    ReleaseVec(input->server_output.dump_data.choices);
    ReleaseVec(input->server_output.dump_data.trans_hash_inputs);
    ReleaseVec(input->server_output.dump_data.result);
}

// input和oupt的转换包装一下，减少调用代码
offline_cpp::ClientOfflineInput Change(const ClientOfflineInput& input) {
    return ClientInputToCpp(input);
}
ClientOfflineInput Change(const offline_cpp::ClientOfflineInput& input) {
    return ClientInputFromCpp(input);
}
offline_cpp::ClientOfflineOutput Change(const ClientOfflineOutput& output) {
    return ClientOutputToCpp(output);
}
ClientOfflineOutput Change(const offline_cpp::ClientOfflineOutput& output) {
    return ClientOutputFromCpp(output);
}
offline_cpp::ServerOfflineInput Change(const ServerOfflineInput& input) {
    return ServerInputToCpp(input);
}
ServerOfflineInput Change(const offline_cpp::ServerOfflineInput& input) {
    return ServerInputFromCpp(input);
}
offline_cpp::ServerOfflineOutput Change(const ServerOfflineOutput& output) {
    return ServerOutputToCpp(output);
}
ServerOfflineOutput Change(const offline_cpp::ServerOfflineOutput& output) {
    return ServerOutputFromCpp(output);
}

#define FastCall(func) Change((offline_cpp::func)(Change(input)))
ServerOfflineOutput ServerOne_PreDealPSIIds(ServerOfflineInput input) {
    // auto output = offline_cpp::ServerOne_PreDealPSIIds(ServerInputToCpp(input));
    // return ServerOutputFromCpp(output);
    return FastCall(ServerOne_PreDealPSIIds);
}
ClientOfflineOutput ClientOne_PreDealPSIIds(ClientOfflineInput input) {
    // return Change(offline_cpp::ClientOne_PreDealPSIIds(Change(input)));
    return FastCall(ClientOne_PreDealPSIIds);
}

// 阶段2，计算OT
ClientOfflineOutput ClientTwo_ReceiverBaseOTs(ClientOfflineInput input, const char* ip, uint32_t port) { 
    return Change(offline_cpp::ClientTwo_ReceiverBaseOTs(Change(input), ip, port));
}
ServerOfflineOutput ServerTwo_SenderBaseOTs(ServerOfflineInput input, const char* ip, uint32_t port) {
    return Change(offline_cpp::ServerTwo_SenderBaseOTs(Change(input), ip, port));
}

// 阶段3，计算matrix
ClientOfflineOutput ClientThree_Matrix(ClientOfflineInput input) {
    return FastCall(ClientThree_Matrix);
}
ServerOfflineOutput ServerThree_Matrix(ServerOfflineInput input) {
    return FastCall(ServerThree_Matrix);
} 

// 阶段4，计算oprf结果
ServerOfflineOutput ServerFour_HashOutput(ServerOfflineInput input) {
    return FastCall(ServerFour_HashOutput);
}
ClientOfflineOutput ClientFour_HashOutput(ClientOfflineInput input) {
    return FastCall(ClientFour_HashOutput);
}

// 阶段5，client发送结果给server
ClientOfflineOutput ClientFive_SendResult(ClientOfflineInput input) {
    return FastCall(ClientFive_SendResult);
}
ServerOfflineOutput ServerFive_RecvResult(ServerOfflineInput input) {
    return FastCall(ServerFive_RecvResult);
}
