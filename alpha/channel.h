#ifndef __ALPHA_CHANNEL_H__
#define __ALPHA_CHANNEL_H__

#include "alpha.h"
#include "fiber_sync.h"
#include <queue>
#include <stdexcept>
#include <utility>

namespace alphaMin {

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_NAME("system");

template<class T>
class Chan {
public:
    typedef std::shared_ptr<Chan> ptr;

    Chan(size_t capacity = 0, bool block = true)
        :m_capacity(capacity)
        ,m_block(block)
        ,m_closed(false) {}
    
    // 发送数据到通道
    bool send(const T& data) {
        if(m_block) {
            m_mutex.lock();
            if(m_closed) {
                m_mutex.unlock();
                throw std::runtime_error("send on closed channel");
            }
            while(m_capacity > 0 && m_queue.size() >= m_capacity) {
                m_notFull.wait(m_mutex);
            }
            m_queue.push(data);
            m_notEmpty.notify_one();
            m_mutex.unlock();
        } else {
            return trySend(data);
        }
        return true;
    }

    // 非阻塞发送数据到通道，返回是否成功
    bool trySend(const T& data) {
        m_mutex.lock();
        if(m_closed) {
            m_mutex.unlock();
            return false;
        }
        if(m_capacity > 0 && m_queue.size() >= m_capacity) {
            m_mutex.unlock();
            return false;
        }
        m_queue.push(data);
        m_notEmpty.notify_one();
        m_mutex.unlock();
        return true;
    }

    std::pair<std::unique_ptr<T>, bool> receive() {
        ALPHA_LOG_INFO(g_logger) << "start receive";
        std::unique_ptr<T> data;
        bool closed = false;
        if(m_block) {
            ALPHA_LOG_INFO(g_logger) << "need lock";
            m_mutex.lock();
            while(m_queue.empty() && !m_closed) {
                ALPHA_LOG_INFO(g_logger) << "block read, to wait";
                m_notEmpty.wait(m_mutex);
                ALPHA_LOG_INFO(g_logger) << "block read, from wait";
            }
            if(m_queue.empty() && m_closed) {
                ALPHA_LOG_INFO(g_logger) << "chan is empty, and closed";
                closed = true;
                data = nullptr; // 返回一个空指针
            } else if(!m_queue.empty()) {
                ALPHA_LOG_INFO(g_logger) << "=================";
                data.reset(new T(std::move(m_queue.front()))); // 使用 std::unique_ptr 的构造函数
                ALPHA_LOG_INFO(g_logger) << "=================";
                m_queue.pop();
                if(!m_closed) {
                    m_notFull.notify_one();
                }
            }
            m_mutex.unlock();
            ALPHA_LOG_INFO(g_logger) << "have unlock";
            if(data == nullptr) {
                ALPHA_LOG_INFO(g_logger) << "data is nullptr";
            }
        } else {
            std::pair<std::unique_ptr<T>, bool> result = tryReceive();
            data = std::move(result.first);
            closed = result.second;
        }
        ALPHA_LOG_INFO(g_logger) << "need to return";
        return std::make_pair(std::move(data), closed);
    }

    std::pair<std::unique_ptr<T>, bool> tryReceive() {
        std::unique_ptr<T> data;
        bool closed = false;
        m_mutex.lock();
        if(m_queue.empty() && !m_closed) {
            m_mutex.unlock();
            return std::make_pair(std::move(data), closed);
        } else if(m_queue.empty() && m_closed) {
            closed = true;
            data = nullptr; // 返回一个空指针
        } else {
            data.reset(new T(std::move(m_queue.front()))); // 使用 std::unique_ptr 的构造函数
            m_queue.pop();
        }
        m_mutex.unlock();
        return std::make_pair(std::move(data), closed);
    }

    // close chan
    void close() {
        m_mutex.lock();

        m_closed = true;
        m_notEmpty.notify_all();

        m_mutex.unlock();
    }

    bool isClosed() const {
        return m_closed;
    }

    bool isEmpty() const {
        return m_queue.empty();
    }

    size_t size() const {
        return m_queue.size();
    }

private:
    size_t m_capacity; // 缓冲区容量，0 表示无限制
    bool m_block; // 是否阻塞
    std::queue<T> m_queue; // 数据队列
    FiberMutex m_mutex; // 协程互斥锁
    FiberCondition m_notEmpty; // 不为空的条件变量
    FiberCondition m_notFull; // 不为满的条件变量
    bool m_closed; // 通道是否关闭
};

}

#endif