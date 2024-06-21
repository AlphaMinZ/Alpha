#include "rpc_channelimpl.h"
#include "../alpha.h"
#include "rpcheader.pb.h"
#include "rpc_client.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <unistd.h>

namespace alphaMin {

static alphaMin::Logger::ptr g_rpclogger = ALPHA_LOG_NAME("rpc");
using namespace google::protobuf;

namespace rpc {

RpcChannelImpl::RpcChannelImpl() {}

RpcChannelImpl::~RpcChannelImpl() {}

void RpcChannelImpl::CallMethod(
        const MethodDescriptor* method,
        RpcController*          controller,
        const Message*          request,
        Message*                response,
        Closure*                done) {

    const ServiceDescriptor* serviceDesc = method->service();

    std::string serviceName = serviceDesc->name();
    std::string methodName = method->name();

    // 获取参数的序列化字符串长度
    std::string argsStr;
    if(!request->SerializeToString(&argsStr)) {
        controller->SetFailed("failed to serialze request");
        return;
    }

    int argsSize = argsStr.size();
    RpcHeader rpcHeader;
    rpcHeader.set_service_name(serviceName);
    rpcHeader.set_method_name(methodName);
    rpcHeader.set_args_size(argsSize);

    // 序列化消息
    std::string rpcHeaderStr;
    if(!rpcHeader.SerializeToString(&rpcHeaderStr)) {
        controller->SetFailed("failed to serialize RPC Header");
        return;
    }

    uint32_t headerSize = rpcHeaderStr.size();

    // 组织待发送的 RPC 请求的字符串
    m_package.clear();
    m_package.insert(0, std::string(reinterpret_cast<char*>(&headerSize), 4));
    m_package += rpcHeaderStr;
    m_package += argsStr;

    ALPHA_LOG_DEBUG(g_rpclogger) << "receive rpc header: ["
                    << headerSize << "] ["
                    << rpcHeaderStr.c_str() << "] ["
                    << serviceName.c_str() << "] ["
                    << methodName.c_str() << "] ["
                    << argsSize << "] ["
                    << argsStr.c_str() << "]";

    // 在 zookeeper 上查询所需服务的主机 IP 和端口号
    ZkClientRPC zkCli;
    zkCli.start();
     ALPHA_LOG_DEBUG(g_rpclogger) << "创建 zk client";
    std::string method_path = "/" + serviceName + "/" + methodName;
    std::string hostData = zkCli.getData(method_path);
    if(hostData == "") {
        controller->SetFailed(method_path + " is not exist");
        return;
    }

    int idx = hostData.find(':');
    if(idx == -1) {
        controller->SetFailed(method_path + " address is invalid");
        return;
    }

    std::string ip = hostData.substr(0, idx);
    uint16_t port = atoi(hostData.substr(idx + 1).c_str());

    IPAddress::ptr addr = alphaMin::IPAddress::Create(ip.c_str(), port);
    // IPAddress::ptr addr = alphaMin::IPAddress::Create("127.0.0.1", 8000);

    ALPHA_LOG_INFO(g_rpclogger) << "1";

    RpcClient::ptr client = std::make_shared<RpcClient>(addr, "RpcChannelImpl");
    ALPHA_LOG_INFO(g_rpclogger) << "2";
    client->setConnectionCallback(std::bind(&RpcChannelImpl::onConnection, this, std::placeholders::_1));
    ALPHA_LOG_INFO(g_rpclogger) << "3";
    client->setMessageCallback(std::bind(&RpcChannelImpl::onMessage, this, client->getMySock()));

    client->getMutex().lock();
    ALPHA_LOG_INFO(g_rpclogger) << "++++++++++==";
    client->connect();
    ALPHA_LOG_INFO(g_rpclogger) << "============";
    client->getCond().wait(client->getMutex());
    // sleep(3);

    ALPHA_LOG_INFO(g_rpclogger) << "close connection by RpcProvider";

    // 对接收到的RPC应答进行反序列化
    
    if(!response->ParseFromString(m_result)) {
        controller->SetFailed("failed to parse response: " + m_result);
    }
    ALPHA_LOG_DEBUG(g_rpclogger) << "接受结束";
}

void RpcChannelImpl::onConnection(const Socket::ptr& conn) {
    if(conn->isConnected()) {
        ALPHA_LOG_INFO(g_rpclogger) << "Connection Up";
        // 发送RPC请求
        conn->send(m_package.c_str(), m_package.size());
        ALPHA_LOG_INFO(g_rpclogger) << "send package to RpcProvider: " << m_package;
    } else {
        ALPHA_LOG_INFO(g_rpclogger) << "Connection Down";
        conn->close();
    }
}

void RpcChannelImpl::onMessage(const Socket::ptr& conn) {
    // 接受RPC应答
    ALPHA_LOG_INFO(g_rpclogger) << "jieshou";
    SocketStream ss(conn);
    ss.read(&m_result, m_result.size());
    ALPHA_LOG_INFO(g_rpclogger) << "receive response from RpcProvider: " << m_result;
    // conn->cancelAll();
    conn->close();
}

}

}