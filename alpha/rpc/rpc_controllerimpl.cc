#include "rpc_controllerimpl.h"

namespace alphaMin {

namespace rpc {

RpcControllerImpl::RpcControllerImpl()
    :m_failed(false)
    ,m_errText("") {}

RpcControllerImpl::~RpcControllerImpl() {}

void RpcControllerImpl::Reset() {
    m_failed = false;
    m_errText = "";
}

bool RpcControllerImpl::Failed() const {
    return m_failed;
}

std::string RpcControllerImpl::ErrorText() const {
    return m_errText;
}

void RpcControllerImpl::SetFailed(const std::string& reason) {
    m_failed = true;
    m_errText = reason;
}

void RpcControllerImpl::StartCancel() {}

bool RpcControllerImpl::IsCanceled() const {
    return false;
}

void RpcControllerImpl::NotifyOnCancel(google::protobuf::Closure* callback) {}

}

}