#include "../alpha/alpha.h"
alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();


void run() {
    alphaMin::rpc::RpcProvider rpc;
    rpc.run();
    // alphaMin::IPAddress::ptr addr = alphaMin::IPAddress::Create("127.0.0.1", 8000);
    
    // alphaMin::TcpServer::ptr server(new alphaMin::TcpServer);
    // server->bind(addr);

    // sleep(2);

    // server->start();
}

int main(int argc, char** argv) {

    alphaMin::IOManager iom(2);
    iom.schedule(run);
    // iom.stop();
    // run();
    return 0;
}