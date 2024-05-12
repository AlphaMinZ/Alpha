#include "../alpha/tcp_server.h"
#include "../alpha/log.h"
#include "../alpha/iomanager.h"
#include "../alpha/bytearray.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

class EchoServer : public alphaMin::TcpServer {
public:
    EchoServer(int type);
    void handleClient(alphaMin::Socket::ptr client);
private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type) {

}

void EchoServer::handleClient(alphaMin::Socket::ptr client) {
    ALPHA_LOG_INFO(g_logger) << "handleClient " << *client;
    alphaMin::ByteArray::ptr ba(new alphaMin::ByteArray);
    while(true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0) {
            ALPHA_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if(rt < 0) {
            ALPHA_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        if(m_type == 1) { // text
            std::cout << ba->toString();
        } else {
            std::cout << ba->toHexString();
        }
    }
}

int type = 1;

void run() {
    EchoServer::ptr es(new EchoServer(type));
    auto addr = alphaMin::Address::LookupAny("0.0.0.0:8022");
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}

int main(int argc, char** argv) {
    if(argc < 2) {
        ALPHA_LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if(!strcmp(argv[1], "-b")) {
        type = 2;
    }

    alphaMin::IOManager iom(2);
    ALPHA_LOG_INFO(g_logger) << "type = " << type;
    iom.schedule(run);
    return 0;
}