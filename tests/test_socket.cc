#include "../alpha/socket.h"
#include "../alpha/iomanager.h"

static alphaMin::Logger::ptr g_looger = ALPHA_LOG_ROOT();

void test_socket() {
    //std::vector<alphaMin::Address::ptr> addrs;
    //alphaMin::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //alphaMin::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    ALPHA_LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<alphaMin::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    alphaMin::IPAddress::ptr addr = alphaMin::Address::LookupAnyIPAddress("www.bilibili.com");
    // alphaMin::IPAddress::ptr addr = alphaMin::Address::LookupAnyIPAddress("www.mihoyo.com");
    if(addr) {
        ALPHA_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        ALPHA_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    alphaMin::Socket::ptr sock = alphaMin::Socket::CreateTCP(addr);
    addr->setPort(80);
    ALPHA_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        ALPHA_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        ALPHA_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        ALPHA_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        ALPHA_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    ALPHA_LOG_INFO(g_looger) <<  "ret msg is " << buffs;
}

void test2() {
    alphaMin::IPAddress::ptr addr = alphaMin::Address::LookupAnyIPAddress("www.baidu.com:80");
    if(addr) {
        ALPHA_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        ALPHA_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    alphaMin::Socket::ptr sock = alphaMin::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        ALPHA_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        ALPHA_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = alphaMin::GetCurrentUS();
    for(size_t i = 0; i < 10000000000ul; ++i) {
        if(int err = sock->getError()) {
            ALPHA_LOG_INFO(g_looger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        //struct tcp_info tcp_info;
        //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //    ALPHA_LOG_INFO(g_looger) << "err";
        //    break;
        //}
        //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //    ALPHA_LOG_INFO(g_looger)
        //            << " state=" << (int)tcp_info.tcpi_state;
        //    break;
        //}
        static int batch = 10000000;
        if(i && (i % batch) == 0) {
            uint64_t ts2 = alphaMin::GetCurrentUS();
            ALPHA_LOG_INFO(g_looger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}

void test_server() {
    ALPHA_LOG_INFO(g_looger) << "server start";
    alphaMin::IPAddress::ptr addr = alphaMin::IPAddress::Create("127.0.0.1", 9527);
    if(addr) {
        ALPHA_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        ALPHA_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    alphaMin::Socket::ptr server_sock = alphaMin::Socket::CreateTCP(addr);

    server_sock->bind(addr);

    if(!server_sock->isValid()) {
        ALPHA_LOG_INFO(g_looger) << "server sock creat fail";
        return;
    }

    server_sock->listen(10);

    ALPHA_LOG_INFO(g_looger) << "listen succ";

    alphaMin::Socket::ptr client_sock = server_sock->accept();

    if(client_sock->isValid()) {
        ALPHA_LOG_INFO(g_looger) << "accept succ";
    }

    alphaMin::IOManager::GetThis()->addEvent(client_sock->getSocket(), alphaMin::IOManager::READ, [client_sock](){
        ALPHA_LOG_INFO(g_looger) << "read callback";
        sleep(5);
        std::string buffer;
        buffer.resize(1024);
        client_sock->recv(&buffer[0], buffer.size());
        ALPHA_LOG_INFO(g_looger) << "from client: " << buffer.size() << buffer;
    });

    // sleep(5);
    // std::string buffer;
    // buffer.resize(1024);
    // client_sock->recv(&buffer[0], buffer.size());
    // ALPHA_LOG_INFO(g_looger) << "from client: " << buffer.size() << buffer;
    
    
}

void test_client() {
    ALPHA_LOG_INFO(g_looger) << "client start";
    alphaMin::IPAddress::ptr addr = alphaMin::IPAddress::Create("127.0.0.1", 9527);
    if(addr) {
        ALPHA_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        ALPHA_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    alphaMin::Socket::ptr client_sock = alphaMin::Socket::CreateTCPSocket();
    if(!client_sock->connect(addr)) {
        ALPHA_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        ALPHA_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "hello server";
    int rt = client_sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        ALPHA_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    sleep(2);

    rt = client_sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        ALPHA_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    sleep(2);

    rt = client_sock->send(buff, sizeof(buff));

    if(rt <= 0) {
        ALPHA_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }
}

int main(int argc, char** argv) {
    alphaMin::IOManager iom(2);
    // iom.schedule(&test_socket);
    // iom.schedule(&test2);
    iom.schedule(&test_server);
    // sleep(2);
    iom.schedule(&test_client);
    // test_server();
    return 0;
}