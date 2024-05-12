#include "../alpha/http/http_server.h"
#include "../alpha/log.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

int run() {
    alphaMin::http::HttpServer::ptr server(new alphaMin::http::HttpServer);
    alphaMin::Address::ptr addr = alphaMin::Address::LookupAnyIPAddress("0.0.0.0:8022");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/alpha/xx", [](alphaMin::http::HttpRequest::ptr req
                ,alphaMin::http::HttpResponse::ptr rsp
                ,alphaMin::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/alpha/*", [](alphaMin::http::HttpRequest::ptr req
                ,alphaMin::http::HttpResponse::ptr rsp
                ,alphaMin::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });
    server->start();
}

int main(int argc, char** argv) {
    alphaMin::IOManager iom(2);
    iom.schedule(run);
    return 0;
}