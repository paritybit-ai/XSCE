#include <iostream>
#include "PSI/offline/offline_interface.h"
#include "PSI/offline/c_offline_interface.h"

#include <vector>
#include <string>

void PlaintextIntersection(std::vector<std::string> ids, std::vector<std::string> ids2) {
    std::sort(ids.begin(), ids.end());
    std::sort(ids2.begin(), ids2.end());
    std::vector<std::string> rlt;

    for (int i1 = 0, i2 = 0; i1 < ids.size() && i2 < ids2.size();) {
        if (ids[i1] == ids2[i2]) {
            rlt.push_back(ids[i1]);
            ++i1;
            ++i2;
        } else if (ids[i1] < ids2[i2]) {
            ++i1;
        } else {
            ++i2;
        }
    }

    std::for_each(rlt.begin(), rlt.end(), [](const std::string& str){
        std::cout << "result:" << str << std::endl;
    });
}

extern offline_cpp::ClientOfflineOutput ClientOutputToCpp(const ClientOfflineOutput& output);
extern ClientOfflineInput ClientInputFromCpp(const offline_cpp::ClientOfflineInput& input);
extern offline_cpp::ServerOfflineOutput ServerOutputToCpp(const ServerOfflineOutput& output);
extern ServerOfflineInput ServerInputFromCpp(const offline_cpp::ServerOfflineInput& input);
void CInterfaceOfflineTest(offline_cpp::OprfArgs client_args, offline_cpp::OprfArgs server_args,
    std::vector<std::string> client_ids, std::vector<std::string> server_ids) {
    offline_cpp::ClientOfflineInput client_input_cpp;
    offline_cpp::ServerOfflineInput server_input_cpp;
    client_input_cpp.args = client_args;
    client_input_cpp.ids = client_ids;
    server_input_cpp.args = server_args;
    server_input_cpp.ids = server_ids;

    ClientOfflineInput client_input = ClientInputFromCpp(client_input_cpp);
    ServerOfflineInput server_input = ServerInputFromCpp(server_input_cpp);

    // 2. 阶段1, hash输入数据；
    auto server_one_output_fut = std::async(ServerOne_PreDealPSIIds, server_input);
    ServerOfflineOutput server_one_output = server_one_output_fut.get();

    client_input.recv_data = server_one_output.send_data;
    auto client_one_output_fut = std::async(ClientOne_PreDealPSIIds, client_input);
    ClientOfflineOutput client_one_output = client_one_output_fut.get();

    // 阶段2, 生成ot数据;
    client_input.dump_data = client_one_output.dump_data;
    auto client_two_output_fut = std::async(ClientTwo_ReceiverBaseOTs, client_input, "127.0.0.1", 12345);

    server_input.dump_data = server_one_output.dump_data;
    auto server_two_output_fut = std::async(ServerTwo_SenderBaseOTs, server_input, "127.0.0.1", 12345);

    ClientOfflineOutput client_two_output = client_two_output_fut.get();
    ServerOfflineOutput server_two_output = server_two_output_fut.get();

    std::cout << ">>>>>>>>>>>>>>C interface to compute matrix<<<<<<<<<<<<<<<<<<" << std::endl;
    // 阶段3，计算矩阵
    client_input.dump_data = client_two_output.dump_data;
    auto client_three_output_fut = std::async(ClientThree_Matrix, client_input);
    ClientOfflineOutput client_three_output = client_three_output_fut.get();

    server_input.dump_data = server_two_output.dump_data;
    server_input.recv_data = client_three_output.send_data;
    auto server_three_output_fut = std::async(ServerThree_Matrix, server_input);
    ServerOfflineOutput server_three_output = server_three_output_fut.get();

    // 阶段4，计算psi结果
    server_input.dump_data = server_three_output.dump_data;
    auto server_four_output_fut = std::async(ServerFour_HashOutput, server_input);
    ServerOfflineOutput server_four_output = server_four_output_fut.get();

    client_input.dump_data = client_three_output.dump_data;
    client_input.recv_data = server_four_output.send_data;
    auto client_four_output_fut = std::async(ClientFour_HashOutput, client_input);
    ClientOfflineOutput client_four_output = client_four_output_fut.get();

    // 阶段5，client发送psi结果给server
    client_input.dump_data = client_four_output.dump_data;
    auto client_five_output_fut = std::async(ClientFive_SendResult, client_input);
    ClientOfflineOutput client_five_output = client_five_output_fut.get();

    server_input.dump_data = server_four_output.dump_data;
    server_input.recv_data = client_five_output.send_data;
    auto server_five_output_fut = std::async(ServerFive_RecvResult, server_input);
    ServerOfflineOutput server_five_output = server_five_output_fut.get();


    auto client_five_output_cpp = ClientOutputToCpp(client_five_output);
    auto server_five_output_cpp = ServerOutputToCpp(server_five_output);
    // 打印结果;
    std::vector<uint64_t> cli_rlt = client_five_output_cpp.dump_data.result;
    std::vector<uint64_t> srv_rlt = server_five_output_cpp.dump_data.result;
    std::cout << ">>>>>>>>>>>>>>>PSI result num: " << cli_rlt.size() << "<<<<<<<<<<<<<<<<<<<" << std::endl;
    for (int i = 0; i < cli_rlt.size(); ++i) {
        std::cout << "rlt[" << i << "]: client index=" << cli_rlt[i] << ", server index:" << srv_rlt[i] << std::endl;  
    }
    
    OfflineStruct ostr = {client_input, client_one_output, server_input, server_one_output};
    ReleaseMemory(&ostr);
}

void OfflineTest(offline_cpp::OprfArgs client_args, offline_cpp::OprfArgs server_args,
    std::vector<std::string> client_ids, std::vector<std::string> server_ids) {
    offline_cpp::ClientOfflineInput client_input;
    offline_cpp::ServerOfflineInput server_input;
    client_input.args = client_args;
    client_input.ids = client_ids;
    server_input.args = server_args;
    server_input.ids = server_ids;
    // 2. 阶段1, hash输入数据；
    auto server_one_output_fut = std::async(offline_cpp::ServerOne_PreDealPSIIds, server_input);
    offline_cpp::ServerOfflineOutput server_one_output = server_one_output_fut.get();

    client_input.recv_data = server_one_output.send_data;
    auto client_one_output_fut = std::async(offline_cpp::ClientOne_PreDealPSIIds, client_input);
    offline_cpp::ClientOfflineOutput client_one_output = client_one_output_fut.get();

    // 阶段2, 生成ot数据;
    client_input.dump_data = client_one_output.dump_data;
    auto client_two_output_fut = std::async(offline_cpp::ClientTwo_ReceiverBaseOTs, client_input, "127.0.0.1", 12345);

    server_input.dump_data = server_one_output.dump_data;
    auto server_two_output_fut = std::async(offline_cpp::ServerTwo_SenderBaseOTs, server_input, "127.0.0.1", 12345);

    offline_cpp::ClientOfflineOutput client_two_output = client_two_output_fut.get();
    offline_cpp::ServerOfflineOutput server_two_output = server_two_output_fut.get();

    // 阶段3，计算矩阵
    client_input.dump_data = client_two_output.dump_data;
    auto client_three_output_fut = std::async(offline_cpp::ClientThree_Matrix, client_input);
    offline_cpp::ClientOfflineOutput client_three_output = client_three_output_fut.get();

    server_input.dump_data = server_two_output.dump_data;
    server_input.recv_data = client_three_output.send_data;
    auto server_three_output_fut = std::async(offline_cpp::ServerThree_Matrix, server_input);
    offline_cpp::ServerOfflineOutput server_three_output = server_three_output_fut.get();

    // 阶段4，计算psi结果
    server_input.dump_data = server_three_output.dump_data;
    auto server_four_output_fut = std::async(offline_cpp::ServerFour_HashOutput, server_input);
    offline_cpp::ServerOfflineOutput server_four_output = server_four_output_fut.get();

    client_input.dump_data = client_three_output.dump_data;
    client_input.recv_data = server_four_output.send_data;
    auto client_four_output_fut = std::async(offline_cpp::ClientFour_HashOutput, client_input);
    offline_cpp::ClientOfflineOutput client_four_output = client_four_output_fut.get();

    // 阶段5，client发送psi结果给server
    client_input.dump_data = client_four_output.dump_data;
    auto client_five_output_fut = std::async(offline_cpp::ClientFive_SendResult, client_input);
    offline_cpp::ClientOfflineOutput client_five_output = client_five_output_fut.get();

    server_input.dump_data = server_four_output.dump_data;
    server_input.recv_data = client_five_output.send_data;
    auto server_five_output_fut = std::async(offline_cpp::ServerFive_RecvResult, server_input);
    offline_cpp::ServerOfflineOutput server_five_output = server_five_output_fut.get();

    // 打印结果;
    std::vector<uint64_t> cli_rlt = client_five_output.dump_data.result;
    std::vector<uint64_t> srv_rlt = server_five_output.dump_data.result;
    std::cout << ">>>>>>>>>>>>>>>PSI result num: " << cli_rlt.size() << "<<<<<<<<<<<<<<<<<<<" << std::endl;
    for (int i = 0; i < cli_rlt.size(); ++i) {
        std::cout << "rlt[" << i << "]: client index=" << cli_rlt[i] << ", server index:" << srv_rlt[i] << std::endl;  
    }
}

void OnlineTest(offline_cpp::OprfArgs client_args, offline_cpp::OprfArgs server_args,
    std::vector<std::string> client_ids, std::vector<std::string> server_ids) {
    auto client_fut = std::async(offline_cpp::ClientOnlinePsi, client_args, client_ids, "127.0.0.1", 12345);
    auto server_fut = std::async(offline_cpp::ServerOnlinePsi, server_args, server_ids, "127.0.0.1", 12345);

    auto client_rlt = client_fut.get();
    server_fut.get();

    std::cout << ">>>>>>>>>>>>>>>PSI result num: " << client_rlt.size() << "<<<<<<<<<<<<<<<<<<<" << std::endl;
    for (int i = 0; i < client_rlt.size(); ++i) {
        std::cout << "rlt[" << i << "]: client index=" << client_rlt[i].second << ", server index:" << client_rlt[i].first << std::endl;
    }
}

void TestChannelAndOffline();
int main() {
    // 1. 准备数据
    offline_cpp::OprfArgs client_args;
    offline_cpp::OprfArgs server_args;
    server_args.inertalSeed = 0xaabbccddee;
    server_args.inertalSeed1 = 0xaabbccddee;

    client_args.commonSeed = 15097466436413582908;
    client_args.commonSeed1 = 1792421738015176190;
    server_args.commonSeed = 15097466436413582908;
    server_args.commonSeed1 = 1792421738015176190;

    std::vector<std::string> client_ids;
    std::vector<std::string> server_ids;
    for (int i = 0; i < 100; ++i) {
        client_ids.push_back(std::to_string(i)); // 0到99
    }
    for (int i = 0; i < 100; ++i) {
        server_ids.push_back(std::to_string(i * 2)); // 0到200之间的偶数
    }

    // PlaintextIntersection(client_ids, server_ids);
    // OfflineTest(client_args, server_args, client_ids, server_ids);
    CInterfaceOfflineTest(client_args, server_args, client_ids, server_ids);
    // OnlineTest(client_args, server_args, client_ids, server_ids);
    
    return 0;
}

