#ifndef __ALPHA_SOCKET_STREAM_H__
#define __ALPHA_SOCKET_STREAM_H__

#include "stream.h"
#include "socket.h"

namespace alphaMin {

class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;
    
    SocketStream(Socket::ptr sock, bool owner = true);

    ~SocketStream();

    int read(void* buffer, size_t length) override;

    int read(ByteArray::ptr ba, size_t length) override;

    int write(const void* buffer, size_t length) override;

    int write(ByteArray::ptr ba, size_t length) override;

    void close() override;

    Socket::ptr getSocket() const { return m_socket;}
    bool isConnection() const;
protected:
    Socket::ptr m_socket;
    bool m_owner;
};

}

#endif