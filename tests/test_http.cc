#include "../alpha/http/http.h"
#include "../alpha/log.h"

void test_request() {
    alphaMin::http::HttpRequest::ptr req(new alphaMin::http::HttpRequest);
    req->setHeader("host", "www.sylar.top");
    req->setBody("hello alpha");

    req->dump(std::cout) << std::endl;
}

void test_response() {
    alphaMin::http::HttpResponse::ptr rsp(new alphaMin::http::HttpResponse);
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello alpha");
    rsp->setStatus((alphaMin::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}