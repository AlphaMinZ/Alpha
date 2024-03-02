#include "../alpha/tcp_server.h"
#include "../alpha/iomanager.h"
#include "../alpha/log.h"

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void run() {
    auto addr = alphaMin::Address::LookupAny("0.0.0.0:8022");
    //auto addr2 = alphaMin::UnixAddress::ptr(new alphaMin::UnixAddress("/tmp/unix_addr"));
    std::vector<alphaMin::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    alphaMin::TcpServer::ptr tcp_server(new alphaMin::TcpServer);
    std::vector<alphaMin::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    
}
int main(int argc, char** argv) {
    alphaMin::IOManager iom(2);
    iom.schedule(run);
    return 0;
}