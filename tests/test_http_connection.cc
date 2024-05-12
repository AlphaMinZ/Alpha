#include "../alpha/http/http_connection.h"
#include <iostream>
#include <fstream>
#include "../alpha/log.h"
#include "../alpha/address.h"
#include "../alpha/alpha.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void run() {
    alphaMin::Address::ptr addr = alphaMin::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!addr) {
        ALPHA_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    alphaMin::Socket::ptr sock = alphaMin::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        ALPHA_LOG_INFO(g_logger) << "connect " << *addr << " failed";
    }

    alphaMin::http::HttpConnection::ptr conn(new alphaMin::http::HttpConnection(sock));

    alphaMin::http::HttpRequest::ptr req(new alphaMin::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    ALPHA_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;

    conn->sendRequest(req);
    ALPHA_LOG_INFO(g_logger) << "1";
    auto rsp = conn->recvResponse();
    ALPHA_LOG_INFO(g_logger) << "2";
    if(!rsp) {
        ALPHA_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    ALPHA_LOG_INFO(g_logger) << "3";
    ALPHA_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;
    ALPHA_LOG_INFO(g_logger) << "4";

    std::ofstream ofs("rsp.dat");
    ofs << *rsp;
}

int main(int argc, char** argv) {
    alphaMin::IOManager iom(2);
    iom.schedule(run);
    return 0;
}