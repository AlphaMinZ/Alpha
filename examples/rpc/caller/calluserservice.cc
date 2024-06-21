#include "../user.pb.h"
#include "../../../alpha/rpc/rpc_channelimpl.h"
#include "../../../alpha/rpc/rpc_controllerimpl.h"
#include "../../../alpha/alpha.h"

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void run() {
    fixbug::UserServiceRpc_Stub stub(new alphaMin::rpc::RpcChannelImpl);
    fixbug::LoginRequest request;
    request.set_name("alpha");
    request.set_pwd("883721");
    fixbug::LoginResponse response;
    // RPC 同步调用
    alphaMin::rpc::RpcControllerImpl controller;
    stub.Login(&controller, &request, &response, nullptr);

    if(controller.Failed()) {
        ALPHA_LOG_ERROR(g_logger) << controller.ErrorText();
    } else {
        // RPC 调用完成 读取结果
        if(response.result().errcode() == 0) {
            ALPHA_LOG_INFO(g_logger) << "rpc login response: " << response.success();
            std::string test_name_4 = response.test_name_4();
            std::string test_name_1 = response.test_name_1();
            ALPHA_LOG_INFO(g_logger) << "rpc test_name: " << test_name_1 << " " << test_name_1.size(); 
            ALPHA_LOG_INFO(g_logger) << "rpc test_name: " << test_name_4 << " " << test_name_4.size(); 
        } else {
            ALPHA_LOG_ERROR(g_logger) << "failed to rpc login: " << response.result().errmsg();
        }
    }
}

int main() {
    alphaMin::IOManager iom(3);
    iom.schedule(run);
    // iom.stop();
    // run();
    return 0;
}