#ifndef C_OFFLINE_INTERFACE_OPRF_PSI_H_
#define C_OFFLINE_INTERFACE_OPRF_PSI_H_

extern "C" {

// 离线oprf psi c接口
struct OprfArgs {
    uint64_t hashLen;
    uint32_t width;
    uint32_t logHeight;
    uint32_t height;
    uint32_t hashLengthInBytes;
    uint64_t hash1LengthInBytes;
    uint32_t bucket1;
    uint32_t bucket2;
    uint64_t commonSeed;
    uint64_t commonSeed1;
    uint64_t inertalSeed;
    uint64_t inertalSeed1;
};
struct Block {
    uint64_t buf1;
    uint64_t buf2;
};

struct DoubleBlock {
    Block block1;
    Block block2;
};

struct PairUint64 {
    uint64_t data1;
    uint64_t data2;
};

struct VecBlock {
    Block* data;
    uint64_t size;
    bool need_delete;
};
struct VecDoubleBlock {
    DoubleBlock* data;
    uint64_t size;
    bool need_delete;
};
struct VecUint8 {
    uint8_t* data;
    uint64_t size;
    bool need_delete;
};
typedef struct VecVecUint8 {
    VecUint8* data;
    uint64_t size;
    bool need_delete;
} VecVecChar;
struct VecPairUint64 {
    PairUint64* data;
    uint64_t size;
    bool need_delete;
};
struct VecUint64 {
    uint64_t* data;
    uint64_t size;
    bool need_delete;
};


// 离线算法每一阶段结果保存
struct ClientOfflineDumpData {
    VecBlock receiver_set;
    uint64_t receiver_size;
    uint64_t sender_size;
    VecDoubleBlock ot_messages;
    VecVecUint8 trans_hash_inputs;
    VecPairUint64 pair_result; // server和client结果对, first为client，second为server结果
    VecUint64 result; // client结果，由pair_result得来
};
// 离线算法每一阶段结果保存
struct ServerOfflineDumpData {
    VecBlock sender_set;
    uint64_t sender_size;
    uint64_t receiver_size;
    VecBlock ot_messages;
    VecUint8 choices;
    VecVecUint8 trans_hash_inputs;
    VecUint64 result;
};

struct ClientOfflineInput {
    OprfArgs args;
    // recv_data替换网络中的recv
    VecUint8 recv_data;
    VecVecChar ids;
    ClientOfflineDumpData dump_data;
};
struct ClientOfflineOutput {
    ClientOfflineDumpData dump_data;
    // send_data替换网络中的send
    VecUint8 send_data;
};

struct ServerOfflineInput {
    OprfArgs args;
    VecUint8 recv_data;
    VecVecChar ids;
    ServerOfflineDumpData dump_data;
};
struct ServerOfflineOutput {
    ServerOfflineDumpData dump_data;
    VecUint8 send_data;
};

struct OfflineStruct {
    ClientOfflineInput client_input;
    ClientOfflineOutput client_output;
    ServerOfflineInput server_input;
    ServerOfflineOutput server_output;
};

void ReleaseMemory(const OfflineStruct *input);

ServerOfflineOutput ServerOne_PreDealPSIIds(ServerOfflineInput input); // 发送
ClientOfflineOutput ClientOne_PreDealPSIIds(ClientOfflineInput input); // 接收

// 阶段2，计算OT
ClientOfflineOutput ClientTwo_ReceiverBaseOTs(ClientOfflineInput input, const char* ip, uint32_t port);
ServerOfflineOutput ServerTwo_SenderBaseOTs(ServerOfflineInput input, const char* ip, uint32_t port);

// 阶段3，计算matrix
ClientOfflineOutput ClientThree_Matrix(ClientOfflineInput input); // 发送
ServerOfflineOutput ServerThree_Matrix(ServerOfflineInput input); // 接收

// 阶段4，计算oprf结果
ServerOfflineOutput ServerFour_HashOutput(ServerOfflineInput input); // 发送
ClientOfflineOutput ClientFour_HashOutput(ClientOfflineInput input); // 接收

// 阶段5，client发送结果给server
ClientOfflineOutput ClientFive_SendResult(ClientOfflineInput input); // 发送
ServerOfflineOutput ServerFive_RecvResult(ServerOfflineInput input); // 接收

} // extern "C"
#endif

