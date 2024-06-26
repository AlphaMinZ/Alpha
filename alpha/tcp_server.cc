#include "tcp_server.h"
#include "config.h"
#include "log.h"
#include "socket_stream.h"

namespace alphaMin {

static alphaMin::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
    alphaMin::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
            "tcp server read timeout");

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_NAME("system");

TcpServer::TcpServer(alphaMin::IOManager* worker,
                    alphaMin::IOManager* io_worker,
                    alphaMin::IOManager* accept_worker)
    :m_worker(worker)
    ,m_ioWorker(io_worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("alpha/1.0.0")
    ,m_isStop(true) {
}

TcpServer::~TcpServer() {
    for(auto& i : m_socks) {
        i->close();
    }
    m_socks.clear();
}

void TcpServer::setConf(const TcpServerConf& v) {
    m_conf.reset(new TcpServerConf(v));
}

bool TcpServer::bind(alphaMin::Address::ptr addr, bool ssl) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails, ssl);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs
                        ,std::vector<Address::ptr>& fails
                        ,bool ssl) {
    m_ssl = ssl;
    for(auto& addr : addrs) {
        Socket::ptr sock = ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
        if(!sock->bind(addr)) {
            ALPHA_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            ALPHA_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }

    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for(auto& i : m_socks) {
        ALPHA_LOG_INFO(g_logger) << "type=" << m_type
            << " name=" << m_name
            << " ssl=" << m_ssl
            << " server bind success: " << *i;
    }
    return true;
}

void TcpServer::startAccept(Socket::ptr sock) {
    ALPHA_LOG_DEBUG(g_logger) << "TcpServer::startAccept start";
    while(!m_isStop) {
        Socket::ptr client = sock->accept();
        if(client) {
            client->setRecvTimeout(m_recvTimeout);
            ALPHA_LOG_DEBUG(g_logger) << "TcpServer::startAccept handleClient";
            m_ioWorker->schedule(std::bind(&TcpServer::handleClient,
                        shared_from_this(), client));
        } else {
            ALPHA_LOG_ERROR(g_logger) << "accept errno=" << errno
                << " errstr=" << strerror(errno);
        }
    }
}

bool TcpServer::start() {
    ALPHA_LOG_DEBUG(g_logger) << "TcpServer::start start";
    if(!m_isStop) {
        return true;
    }
    m_isStop = false;
    for(auto& sock : m_socks) {
        ALPHA_LOG_DEBUG(g_logger) << "TcpServer::start startAccept_1";
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                    shared_from_this(), sock));
        ALPHA_LOG_DEBUG(g_logger) << "TcpServer::start startAccept_2";
    }
    ALPHA_LOG_DEBUG(g_logger) << "TcpServer::start stop";
    return true;
}

void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([this, self]() {
        for(auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}



// static void Message(const alphaMin::Socket::ptr& conn) {
//     SocketStream ss(conn);
//     std::string message;
//     message.resize(128);
//     ss.read(&message[0], 128);

//     ALPHA_LOG_INFO(g_logger) << message;

//     sleep(5);
//     conn->send("hello client", 12);
//     bool isConnection = conn->isConnected();
//     if(isConnection) {
//         std::cout << "connectioned\n";
//     }
// }

void TcpServer::handleClient(Socket::ptr client) {
    ALPHA_LOG_INFO(g_logger) << "handleClient: " << *client;

    auto self = shared_from_this();

    // m_ioWorker->addEvent(client->getSocket(), IOManager::READ, [&self, client]() {
    //     // self->getMessageCallback()(client);
    //     Message(client);
    // });
}

bool TcpServer::loadCertificates(const std::string& cert_file, const std::string& key_file) {
    for(auto& i : m_socks) {
        auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(i);
        if(ssl_socket) {
            if(!ssl_socket->loadCertificates(cert_file, key_file)) {
                return false;
            }
        }
    }
    return true;
}

std::string TcpServer::toString(const std::string& prefix) {
    std::stringstream ss;
    ss << prefix << "[type=" << m_type
       << " name=" << m_name << " ssl=" << m_ssl
       << " worker=" << (m_worker ? m_worker->getName() : "")
       << " accept=" << (m_acceptWorker ? m_acceptWorker->getName() : "")
       << " recv_timeout=" << m_recvTimeout << "]" << std::endl;
    std::string pfx = prefix.empty() ? "    " : prefix;
    for(auto& i : m_socks) {
        ss << pfx << pfx << *i << std::endl;
    }
    return ss.str();
}

}