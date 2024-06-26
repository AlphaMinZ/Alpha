#ifndef __ALPHA_RPC_RPC_CONTROLLERIMPL_H__
#define __ALPHA_RPC_RPC_CONTROLLERIMPL_H__

#include <google/protobuf/service.h>
#include <memory>

namespace alphaMin {

namespace rpc {

/**
 * @brief RPC控制器
 * 
 */
class RpcControllerImpl : public google::protobuf::RpcController {
public:
    typedef std::shared_ptr<RpcControllerImpl> ptr;

    RpcControllerImpl();
    ~RpcControllerImpl();

    /**
     * @brief 重置控制器状态
     * 
     */
    void Reset() override;

    /**
     * @brief 是否调用失败
     * 
     */
    bool Failed() const override;

    /**
     * @brief 返回错误文本 
     * 
     * @return std::string 
     */
    std::string ErrorText() const override;

    /**
     * @brief 设置错误原因
     * 
     * @param reason 
     */
    void SetFailed(const std::string& reason) override;

    /**
     * @brief 取消RPC调用 
     * 
     */
    void StartCancel() override;

    /**
     * @brief RPC调用是否取消
     * 
     */
    bool IsCanceled() const override;

    /**
     * @brief 取消RPC调用时进行通知 
     * 
     * @param callbak 
     */
    void NotifyOnCancel(google::protobuf::Closure* callbak) override;

private:
    bool        m_failed;  // RPC方法执行过程中的状态
    std::string m_errText; // RPC方法执行过程中的错误信息
};

}

}

#endif