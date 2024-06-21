#include "zkclient.h"
#include "../alpha.h"

#include <semaphore.h>

namespace alphaMin {

static alphaMin::Logger::ptr g_rpclogger = ALPHA_LOG_NAME("rpc");

namespace rpc {

void g_watcher(zhandle_t* handler, int type, int state,
    const char* path, void* wathcerCtx) {
    ALPHA_LOG_FATAL(g_rpclogger) << "触发 g_watch 连接回调";
    if (type == ZOO_SESSION_EVENT) {
        ALPHA_LOG_FATAL(g_rpclogger) << "会话事件";
        if (state == ZOO_CONNECTED_STATE) {
            ALPHA_LOG_FATAL(g_rpclogger) << "链接事件";
            // FiberCondition* cond = (FiberCondition*)zoo_get_context(handler);
            // if(cond != nullptr) {
            //     ALPHA_LOG_FATAL(g_rpclogger) << "唤醒";
            //     cond->notify_one();
            //     ALPHA_LOG_FATAL(g_rpclogger) << "成功唤醒";
            // }
            sem_t* sem = (sem_t*)zoo_get_context(handler);
            sem_post(sem);
        }
    }
}

ZkClientRPC::ZkClientRPC()
    : m_zkHandler(nullptr) {
}

ZkClientRPC::~ZkClientRPC() {
    if (m_zkHandler != nullptr) {
        zookeeper_close(m_zkHandler);
    }
}

void ZkClientRPC::start() {
    std::string connstr  = "127.0.0.1:2181";

    /**
     * zookeeprer_mt: 多线程版本 zookeeper_st: 单线程版本
     * 有如下三个线程：
     * 1. API 调用线程
     * 2. 网络 I/O 线程(poll)
     * 3. watcher回调函数
     */
    // 初始化zk资源 这是一个异步连接函数 函数返回并不表示与zkServer连接成功
    m_zkHandler = zookeeper_init(connstr.c_str(), g_watcher, 30000, nullptr, nullptr, 0);
    // 如果zkServer返回消息 则会调用该回调函数以改变信号量唤醒当前线程
    if (nullptr == m_zkHandler) {
        ALPHA_LOG_FATAL(g_rpclogger) << "failed to initialize zookeeper";
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);

    ALPHA_LOG_FATAL(g_rpclogger) << "zk句柄初始化成功";

    FiberMutex::ptr mutex = std::make_shared<FiberMutex>();
    FiberCondition* cond = new FiberCondition;
    // mutex->lock();
    // ALPHA_LOG_FATAL(g_rpclogger) << "lock成功";
    zoo_set_context(m_zkHandler, &sem);
    ALPHA_LOG_FATAL(g_rpclogger) << "传递上下文成功";
    // 阻塞直到zkServer响应链接创建成功
    // cond->wait(*mutex);
    // sleep(5);
    sem_wait(&sem);
    // ALPHA_LOG_FATAL(g_rpclogger) << "链接创建成功";
    // mutex->unlock();
    // delete(cond);
    ALPHA_LOG_INFO(g_logger) << "initialize zookeeper successfully";
}

void ZkClientRPC::create(const std::string& path, const std::string& data, int flags) {
    char path_buffer[128];
    int  bufferlen = sizeof(path_buffer);

    // 判断该节点是否存在
    int flag = zoo_exists(m_zkHandler, path.c_str(), 0, nullptr);
    if (flag == ZNONODE) {
        // 创建指定path的znode节点
        flag = zoo_create(m_zkHandler, path.c_str(), data.c_str(), data.size(),
            &ZOO_OPEN_ACL_UNSAFE, flags, path_buffer, bufferlen);
        if (flag == ZOK) {
            ALPHA_LOG_INFO(g_rpclogger) << "create znode: " << path.c_str();
        } else {
            ALPHA_LOG_FATAL(g_rpclogger) << "failed to create znode: " << path.c_str();
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZkClientRPC::getData(const std::string& path) {
    char buffer[64];
    int  bufferlen = sizeof(buffer);
    int  flag      = zoo_get(m_zkHandler, path.c_str(), 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK) {
        ALPHA_LOG_ERROR(g_rpclogger) << "failed to get data of znode: " << path.c_str();
        return "";
    }
    return buffer;
}

int32_t ZkClientRPC::close() {
    int32_t rt = ZOK;
    if(m_zkHandler) {
        rt = zookeeper_close(m_zkHandler);
        m_zkHandler = nullptr;
    }
    return rt;
}

}

}

