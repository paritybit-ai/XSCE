#pragma once
#include <iostream>
#include "oprf_psi/oprf_psi_client.h"
#include "oprf_psi/oprf_psi_server.h"

#include <vector>
#include <string>

// 离线oprf psi c++接口
namespace offline_cpp {
struct OprfArgs {
    OprfArgs() {
        height = 1 << (logHeight);
    }
    uint64_t hashLen{128};
    uint32_t width{632};
    uint32_t logHeight{20};
    uint32_t height;
    uint32_t hashLengthInBytes{10};
    uint64_t hash1LengthInBytes{32};
    uint32_t bucket1{256};
    uint32_t bucket2{256};
    uint64_t commonSeed{0xAA55AA55AA55AA55};
    uint64_t commonSeed1{0xAA55AA55AA55AA55};
    uint64_t inertalSeed{0x1122334455667788};
    uint64_t inertalSeed1{0x1122334455667788};
};

struct ClientOfflineDumpData {
    std::vector<oprf_psi_offline::block> receiver_set;
    uint64_t receiver_size;
    uint64_t sender_size;
    std::vector<std::array<oprf_psi_offline::block, 2>> ot_messages;
    // std::vector<oprf_psi_offline::block> recv_set;
    std::vector<std::vector<uint8_t>> trans_hash_inputs;
    std::vector<std::pair<uint64_t, uint64_t>> pair_result; // server和client结果对, first为client，second为server结果
    std::vector<uint64_t> result; // client结果，由pair_result得来
};
// 离线算法每一阶段结果保存
struct ServerOfflineDumpData {
    std::vector<oprf_psi_offline::block> sender_set;
    uint64_t sender_size;
    uint64_t receiver_size;
    std::vector<oprf_psi_offline::block> ot_messages;
    oprf_psi_offline::BitVector choices;
    std::vector<std::vector<uint8_t>> trans_hash_inputs;
    std::vector<uint64_t> result;
};

struct ClientOfflineInput {
    OprfArgs args;
    // recv_data替换channel的recv
    std::vector<uint8_t> recv_data;
    std::vector<std::string> ids;
    ClientOfflineDumpData dump_data;
};
struct ClientOfflineOutput {
    ClientOfflineDumpData dump_data;
    // send_data替换channel的send
    std::vector<uint8_t> send_data;
};

struct ServerOfflineInput {
    OprfArgs args;
    std::vector<uint8_t> recv_data;
    std::vector<std::string> ids;
    ServerOfflineDumpData dump_data;
};
struct ServerOfflineOutput {
    ServerOfflineDumpData dump_data;
    std::vector<uint8_t> send_data;
};

// 在线oprf psi
std::vector<std::pair<uint64_t, uint64_t>> ClientOnlinePsi(OprfArgs args, std::vector<std::string> ids, const std::string& ip, uint32_t port);
void ServerOnlinePsi(OprfArgs args, std::vector<std::string> ids, const std::string& ip, uint32_t port);

// 离线oprf psi
// 同一角色（server或client）顺序执行每个阶段，同一阶段中，发送执行完毕再执行接收
// 阶段1，计算hash，并交换输入size; 
ServerOfflineOutput ServerOne_PreDealPSIIds(ServerOfflineInput input); // 发送
ClientOfflineOutput ClientOne_PreDealPSIIds(ClientOfflineInput input); // 接收

// 阶段2，计算OT
ClientOfflineOutput ClientTwo_ReceiverBaseOTs(ClientOfflineInput input, const std::string& ip, uint32_t port);
ServerOfflineOutput ServerTwo_SenderBaseOTs(ServerOfflineInput input, const std::string& ip, uint32_t port);

// 阶段3，计算matrix
ClientOfflineOutput ClientThree_Matrix(ClientOfflineInput input); // 发送
ServerOfflineOutput ServerThree_Matrix(ServerOfflineInput input); // 接收

// 阶段4，计算oprf结果
ServerOfflineOutput ServerFour_HashOutput(ServerOfflineInput input); // 发送
ClientOfflineOutput ClientFour_HashOutput(ClientOfflineInput input); // 接收

// 阶段5，client发送结果给server
ClientOfflineOutput ClientFive_SendResult(ClientOfflineInput input); // 发送
ServerOfflineOutput ServerFive_RecvResult(ServerOfflineInput input); // 接收

} // namespace offline_cpp