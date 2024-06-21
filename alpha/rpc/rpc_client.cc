#include "rpc_client.h"

namespace alphaMin {

static alphaMin::Logger::ptr g_rpclogger = ALPHA_LOG_NAME("system");

namespace rpc {

RpcClient::RpcClient(const IPAddress::ptr& serverAddr, const std::string& name)
    :m_name(name)
    ,m_serverAddr(serverAddr)
    ,m_connect(true)
    ,m_retry(false)
    ,m_connectionCallback()
    ,m_messageCallback() {

    m_sock = Socket::CreateTCPSocket();
}

RpcClient::~RpcClient() {
    if(m_sock) {
        // m_sock->cancelAll();
        m_sock->close();
    }
}

void RpcClient::connect() {
    ALPHA_LOG_INFO(g_rpclogger) << "RpcClient[" << m_name.c_str() << "] - connecting to " << m_serverAddr->getAddr()->sa_data;
    m_connect = true;
    if(m_sock->connect(m_serverAddr)) {
        ALPHA_LOG_INFO(g_rpclogger) << "RpcClient connect succ";
    } else {
        ALPHA_LOG_ERROR(g_rpclogger) << "RpcClient connect failed";
        return;
    }

    auto self = shared_from_this();
    // auto sock = getMySock();

    IOManager::GetThis()->addEvent(m_sock->getSocket(), IOManager::READ, [self]() {
        std::cout << "hahahahahahahahahaha";
        self->getMessageCallback()(self->getMySock());
        std::cout << "hahahahahahahahahaha";
        self->getCond().notify_one();
    });

    m_connectionCallback(m_sock);
}

void RpcClient::close() {
    // m_sock->cancelAll();
    m_sock->close();
}

}

}