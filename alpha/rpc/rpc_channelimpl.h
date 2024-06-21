#ifndef __ALPHA_RPC_RPC_CHANNELIMPL_H__
#define __ALPHA_RPC_RPC_CHANNELIMPL_H__

#include <google/protobuf/service.h>
#include <memory>
#include <string>

#include "../alpha.h"

namespace alphaMin {

namespace rpc {

/**
 * @brief RPC消息通道
 * 
 */
class RpcChannelImpl : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannelImpl> {
public:
    RpcChannelImpl();
    RpcChannelImpl(const RpcChannelImpl&) = delete;
    RpcChannelImpl& operator=(const RpcChannelImpl&) = delete;
    ~RpcChannelImpl();

    void CallMethod(const google::protobuf::MethodDescriptor* method,
        google::protobuf::RpcController*                      controller,
        const google::protobuf::Message*                      request,
        google::protobuf::Message*                            response,
        google::protobuf::Closure*                            done) override;

private:
    /**
     * @brief 连接回调函数
     * 
     */
    void onConnection(const Socket::ptr& conn);

    /**
     * @brief 读写消息回调函数
     * 
     * @param conn 连接对象
     * @param buffer 缓冲区
     * @param reveiveTime 消息接收时间
     * @param client 客户端对象
     */
    void onMessage(const Socket::ptr& conn);

private:
    std::string m_package; // RPC请求体
    std::string m_result;  // RPC应答体
};

}

}

#endif
