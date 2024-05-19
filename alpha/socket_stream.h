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

    /**
    * @brief 读取数据直到遇到指定的字符
    * @param[out] output 存储读取到的数据的字符串
    * @param[in] delimiter 指定的分隔字符
    * @return
    *      @retval >0 返回读取到的数据的实际大小
    *      @retval =0 被关闭
    *      @retval <0 出现流错误
    */
    int readUntil(std::string& output, char delimiter);

    Socket::ptr getSocket() const { return m_socket;}
    bool isConnection() const;
protected:
    Socket::ptr m_socket;
    bool m_owner;
    std::vector<char> m_remainingBuffer; // 缓存剩余数据
};

}

#endif