#include "offline_interface.h"

namespace offline_cpp { 
std::vector<std::pair<uint64_t, uint64_t>> ClientOnlinePsi(OprfArgs args, std::vector<std::string> ids, const std::string& ip, uint32_t port) {
    auto receiver_set = oprf_psi_offline::PreDealPSIIds(ids);
    auto receiver_size = receiver_set.size();
    auto sender_size = oprf_psi_offline::GetSenderSize(receiver_size, ip, port);

    auto ot_messages = oprf_psi_offline::ReceiverBaseOTs(args.width, args.inertalSeed, args.inertalSeed1, ip, port);
    auto recv_set = oprf_psi_offline::ComputeRecvSet(receiver_size, args.hash1LengthInBytes, args.commonSeed, args.commonSeed1, receiver_set);
    auto trans_hash_inputs = oprf_psi_offline::ClientExchangeMetrics(args.commonSeed, args.commonSeed1, ip, port,
                            args.height, args.width, args.logHeight, receiver_size, args.bucket1,
                            recv_set, ot_messages);
    auto pair_result = oprf_psi_offline::ReceiveHashOutputsAndComputePSI(args.width, receiver_size, trans_hash_inputs, args.bucket2, args.hashLengthInBytes, sender_size, ip, port);
    return pair_result;
}

void ServerOnlinePsi(OprfArgs args, std::vector<std::string> ids, const std::string& ip, uint32_t port) {
    auto sender_set = oprf_psi_offline::PreDealPSIIds(ids);
    auto sender_size = sender_set.size();
    auto receiver_size = oprf_psi_offline::GetRecevierSize(sender_size, ip, port);

    auto sender_base_ot_result = oprf_psi_offline::SenderBaseOTs(args.width, args.inertalSeed, args.inertalSeed1, ip, port);
    auto send_set = oprf_psi_offline::ComputeSendSet(args.hash1LengthInBytes, sender_size, args.commonSeed, args.commonSeed1, sender_set);
    auto trans_hash_inputs = oprf_psi_offline::ServerExchangeMetrics(args.commonSeed, args.commonSeed1, ip, port, args.height, args.width, args.logHeight, sender_size,
                                    args.bucket1, args.hash1LengthInBytes, send_set, sender_base_ot_result.ot_messages,
                                    sender_base_ot_result.choices);

    oprf_psi_offline::ServerComputeHashOutputs(args.width, sender_size, args.bucket2, trans_hash_inputs, args.hashLengthInBytes, ip, port);
}

ServerOfflineOutput ServerOne_PreDealPSIIds(ServerOfflineInput input) {
    auto sender_set = oprf_psi_offline::PreDealPSIIds(input.ids);
    uint64_t sender_size = sender_set.size();

    ServerOfflineOutput output;
    output.dump_data.sender_set = sender_set;
    output.dump_data.sender_size = sender_size;

    uint8_t* send_data = (uint8_t*)(&sender_size);
    for (int i = 0; i < sizeof(uint64_t); ++i) {
        output.send_data.push_back(send_data[i]);
    }
    return output;
}
ClientOfflineOutput ClientOne_PreDealPSIIds(ClientOfflineInput input) {
    auto receiver_set = oprf_psi_offline::PreDealPSIIds(input.ids);
    uint64_t receiver_size = receiver_set.size();

    uint8_t* recv_data = input.recv_data.data();
    uint64_t sender_size = *((uint64_t*)recv_data);

    // 组织dump和send_data
    ClientOfflineOutput output;
    output.dump_data.receiver_set = receiver_set;
    output.dump_data.receiver_size = receiver_size;
    output.dump_data.sender_size = sender_size;

    return output;
}

ClientOfflineOutput ClientTwo_ReceiverBaseOTs(ClientOfflineInput input, const std::string& ip, uint32_t port) {
    auto ot_messages = oprf_psi_offline::ReceiverBaseOTs(input.args.width, input.args.inertalSeed, input.args.inertalSeed1, ip, port);

    ClientOfflineOutput output;
    output.dump_data.receiver_set = std::move(input.dump_data.receiver_set);
    output.dump_data.receiver_size = input.dump_data.receiver_size;
    output.dump_data.sender_size = input.dump_data.sender_size;
    output.dump_data.ot_messages = std::move(ot_messages);

    return output;
}

ServerOfflineOutput ServerTwo_SenderBaseOTs(ServerOfflineInput input, const std::string& ip, uint32_t port) {
    auto ot_rlt = oprf_psi_offline::SenderBaseOTs(input.args.width, input.args.inertalSeed, input.args.inertalSeed1, ip, port);

    ServerOfflineOutput output;
    output.dump_data.sender_set = input.dump_data.sender_set;
    output.dump_data.sender_size = input.dump_data.sender_size;
    output.dump_data.ot_messages = std::move(ot_rlt.ot_messages);
    output.dump_data.choices = std::move(ot_rlt.choices);

    return output;
}

ClientOfflineOutput ClientThree_Matrix(ClientOfflineInput input) {
    auto recv_set = oprf_psi_offline::ComputeRecvSet(input.dump_data.receiver_size, input.args.hash1LengthInBytes,
                            input.args.commonSeed, input.args.commonSeed1, input.dump_data.receiver_set);
    oprf_psi_offline::offline::SetOfflineChannelFlag(true);
    auto trans_hash_inputs = oprf_psi_offline::ClientExchangeMetrics(input.args.commonSeed, input.args.commonSeed1, "", 0,
                            input.args.height, input.args.width, input.args.logHeight, input.dump_data.receiver_size,
                            input.args.bucket1, recv_set, input.dump_data.ot_messages);

    ClientOfflineOutput output;
    output.dump_data.sender_size = input.dump_data.sender_size;
    output.dump_data.receiver_size = input.dump_data.receiver_size;
    output.dump_data.trans_hash_inputs = std::move(trans_hash_inputs);

    output.send_data = oprf_psi_offline::offline::GetSendData();

    return output;
}

ServerOfflineOutput ServerThree_Matrix(ServerOfflineInput input) {
    auto send_set = oprf_psi_offline::ComputeSendSet(input.args.hash1LengthInBytes, input.dump_data.sender_size,
                            input.args.commonSeed, input.args.commonSeed1, input.dump_data.sender_set);
    oprf_psi_offline::offline::SetOfflineChannelFlag(true);
    oprf_psi_offline::offline::SetRecvData(input.recv_data);
    auto trans_hash_inputs = oprf_psi_offline::ServerExchangeMetrics(input.args.commonSeed, input.args.commonSeed1, "", 0,
                            input.args.height, input.args.width, input.args.logHeight, input.dump_data.sender_size,
                            input.args.bucket1, input.args.hash1LengthInBytes, send_set, 
                            input.dump_data.ot_messages, input.dump_data.choices);

    ServerOfflineOutput output;
    output.dump_data.sender_size = input.dump_data.sender_size;
    output.dump_data.trans_hash_inputs = std::move(trans_hash_inputs);

    return output;
}

ServerOfflineOutput ServerFour_HashOutput(ServerOfflineInput input) {
    oprf_psi_offline::offline::SetOfflineChannelFlag(true);
    oprf_psi_offline::ServerComputeHashOutputs(input.args.width, input.dump_data.sender_size, input.args.bucket2,
                            input.dump_data.trans_hash_inputs, input.args.hashLengthInBytes, "", 0);

    ServerOfflineOutput output;
    output.send_data = oprf_psi_offline::offline::GetSendData();

    return output;
}

ClientOfflineOutput ClientFour_HashOutput(ClientOfflineInput input) {
    oprf_psi_offline::offline::SetOfflineChannelFlag(true);
    oprf_psi_offline::offline::SetRecvData(input.recv_data);
    auto pair_result = oprf_psi_offline::ReceiveHashOutputsAndComputePSI(input.args.width, input.dump_data.receiver_size,
                            input.dump_data.trans_hash_inputs, input.args.bucket2,
                            input.args.hashLengthInBytes, input.dump_data.sender_size,
                            "", 0);
    ClientOfflineOutput output;
    // client--server两端result
    output.dump_data.pair_result = pair_result;
    return output;
}

ClientOfflineOutput ClientFive_SendResult(ClientOfflineInput input) {
    auto pair_result = input.dump_data.pair_result;
    ClientOfflineOutput output;
    // 保存client端result
    std::vector<uint64_t> cli_result;
    for_each(pair_result.begin(), pair_result.end(), [&cli_result](auto iter){
        cli_result.push_back(iter.second);
    });
    output.dump_data.result = cli_result;
    
    // 发送给server端的result
    std::vector<uint64_t> srv_result;
    for_each(pair_result.begin(), pair_result.end(), [&srv_result](auto iter){
        srv_result.push_back(iter.first);
    });

    // std::vector<uint64_t>到std::vector<uint8_t>转换
    uint8_t* send_data = (uint8_t*)srv_result.data();
    uint64_t size = srv_result.size() * sizeof(uint64_t);
    output.send_data = std::vector<uint8_t>(send_data, send_data + size);
    return output;
}

ServerOfflineOutput ServerFive_RecvResult(ServerOfflineInput input) {
    // std::vector<uint8_t>到std::vector<uint64_t>转换
    uint64_t* result = (uint64_t*)input.recv_data.data();
    uint64_t size = input.recv_data.size() / sizeof(uint64_t);
    ServerOfflineOutput output;
    output.dump_data.result = std::vector<uint64_t>(result, result + size);

    return output;
}
} // namespace offline_cpp