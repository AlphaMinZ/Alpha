#ifndef __ALPHA_HTTP_HTTP_SESSION_H__
#define __ALPHA_HTTP_HTTP_SESSION_H__

#include "../socket_stream.h"
#include "http.h"

namespace alphaMin {

namespace http {

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock, bool owner = true);

    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};

}

}

#endif