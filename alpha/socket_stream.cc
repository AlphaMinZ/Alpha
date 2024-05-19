#include "socket_stream.h"

namespace alphaMin {

SocketStream::SocketStream(Socket::ptr sock, bool owner)
    :m_socket(sock)
    ,m_owner(owner) {
}

SocketStream::~SocketStream() {
    if(m_owner && m_socket) {
        m_socket->close();
    }
}

int SocketStream::read(void* buffer, size_t length) {
    if(!isConnection()) {
        return -1;
    }
    if(!m_remainingBuffer.empty()) {
        int toRead = 0;
        if(length <= m_remainingBuffer.size()) {
            toRead = length;
            memcpy(buffer, m_remainingBuffer.data(), toRead);
            m_remainingBuffer.erase(m_remainingBuffer.begin(), m_remainingBuffer.begin() + toRead);
            return toRead;
        } else {
            toRead = length - m_remainingBuffer.size();
            memcpy(buffer, m_remainingBuffer.data(), m_remainingBuffer.size());
            m_remainingBuffer.clear();
            read(buffer + (length - toRead), toRead);
        }
    }
    return m_socket->recv(buffer, length);
} 

int SocketStream::read(ByteArray::ptr ba, size_t length) {
    if(!isConnection()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, length);
    int rt = m_socket->recv(&iovs[0], iovs.size());
    if(rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

int SocketStream::readUntil(std::string& output, char delimiter) {
    char buffer[1024];
    output.clear();
    int totalBytes = 0;

    while(true) {
        int64_t len = read(buffer, sizeof(buffer));
        if(len < 0) {
            // 发生错误
            return -1;
        } else if(len == 0) {
            // 连接关闭
            return totalBytes;
        }

        for(int i = 0; i < len; ++i) {
            if(buffer[i] == delimiter) {
                output.append(buffer, i + 1);
                if(i + 1 < len) {
                    m_remainingBuffer.assign(buffer + i + 1, buffer + len);
                }
                return totalBytes + i + 1;
            }
        }

        output.append(buffer, len);
        totalBytes += len;
        if(len != sizeof(buffer)) {
            break;
        }
    }

    return totalBytes;
}

int SocketStream::write(const void* buffer, size_t length) {
    if(!isConnection()) {
        return -1;
    }
    return m_socket->send(buffer, length);
}

int SocketStream::write(ByteArray::ptr ba, size_t length)  {
    if(!isConnection()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getReadBuffers(iovs, length);
    int rt = m_socket->send(&iovs, iovs.size());
    if(rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

void SocketStream::close() {
    if(m_socket) {
        m_socket->close();
    }
}

bool SocketStream::isConnection() const {
    return m_socket && m_socket->isConnected();
}

}
