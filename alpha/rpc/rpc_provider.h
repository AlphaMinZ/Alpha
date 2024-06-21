#ifndef __ALPHA_RPC_RPC_PROVIDER_H__
#define __ALPHA_RPC_RPC_PROVIDER_H__

#include <google/protobuf/service.h>
#include <memory>
#include <unordered_map>

#include "../alpha.h"
#include "zkclient.h"

namespace alphaMin {

namespace rpc {

class RpcProvider : public Noncopyable {
public:
    typedef std::shared_ptr<RpcProvider> ptr;
    RpcProvider();
    ~RpcProvider();

    /**
     * @brief 发布RPC方法
     * 
     * @param service RPC服务对象
     */
    void notifyService(google::protobuf::Service* service);

    /**
     * @brief 启动RPC服务，开始提供RPC远程网络调用服务
     * 
     */
    void run();

private:
    /**
     * @brief 连接创建与销毁的回调函数
     * 
     */
    void onConnection(const Socket::ptr& conn);

    /**
     * @brief 读写消息回调函数
     * 
     * @param conn 连接对象
     */
    void onMessage(const Socket::ptr& conn);

    /**
     * @brief 序列化RPC的响应和网络发送
     * 
     */
    void sendRpcResponse(const Socket::ptr& conn, google::protobuf::Message*);

private:
    using MethodMap = std::unordered_map<std::string, const google::protobuf::MethodDescriptor*>;

    /**
     * @brief 服务类型信息
     */
    struct ServiceInfo {
        google::protobuf::Service* service;   // 服务对象
        MethodMap                  methodMap; // 服务方法
    };

    using ServiceMap = std::unordered_map<std::string, ServiceInfo>;
    ServiceMap m_serviceMap; // 注册成功的服务对象和服务方法
};

}

}

#endif