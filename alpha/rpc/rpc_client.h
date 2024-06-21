#ifndef __ALPHA_RPC_RPC_CLIENT_H__
#define __ALPHA_RPC_RPC_CLIENT_H__

#include "../alpha.h"

namespace alphaMin {

namespace rpc {

class RpcClient : public std::enable_shared_from_this<RpcClient> {
public:
    typedef std::shared_ptr<RpcClient> ptr;

    RpcClient(const IPAddress::ptr& serverAddr, const std::string& name);
    RpcClient(const RpcClient&) = delete;
    RpcClient& operator=(const RpcClient&) = delete;
    ~RpcClient();

    const std::string& getName() const { return m_name;}

    bool getRetry() const { return m_retry;}

    void enableRetry() { m_retry = true;}

    Socket::ptr& getMySock() { return m_sock;}

    FiberMutex& getMutex() { return m_mutex;}
    FiberCondition& getCond() { return m_cond;}
    bool isConnected() { return m_connect;}
    void connect();

    void close();

    void setConnectionCallback(std::function<void(const Socket::ptr&)> cb) { m_connectionCallback = std::move(cb);}
    void setMessageCallback(std::function<void(const Socket::ptr&)> cb) { m_messageCallback = std::move(cb);}
    void setWriteCompleteCallback(std::function<void(const Socket::ptr&)> cb) { m_writeCopmleteCallback = std::move(cb);}

    std::function<void(const Socket::ptr&)>& getMessageCallback() { return m_messageCallback;}

private:
    Socket::ptr m_sock;

    std::function<void(const Socket::ptr&)> m_connectionCallback;
    std::function<void(const Socket::ptr&)> m_messageCallback;
    std::function<void(const Socket::ptr&)> m_writeCopmleteCallback;

    const std::string m_name;
    std::atomic_bool m_connect;
    std::atomic_bool m_retry;
    Address::ptr m_serverAddr;

    FiberMutex m_mutex;
    FiberCondition m_cond;
};

}

}

#endif