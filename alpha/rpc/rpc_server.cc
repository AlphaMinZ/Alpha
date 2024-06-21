#include "rpc_server.h"
#include "../alpha.h"

namespace alphaMin {

static alphaMin::Logger::ptr g_rpclogger = ALPHA_LOG_NAME("system");

namespace rpc {

void RpcServer::handleClient(Socket::ptr client) {

    ALPHA_LOG_INFO(g_rpclogger) << "RPC handleClient: " << *client;

    auto self = std::static_pointer_cast<RpcServer>(shared_from_this());

    m_ioWorker->addEvent(client->getSocket(), IOManager::READ, [&self, client]() {
        self->getMessageCallback()(client);
    });

    // m_ioWorker->addEvent(client->getSocket(), IOManager::WRITE, [&self, client]() {
    //     client->send("hello", 5);
    // });
}

}

}