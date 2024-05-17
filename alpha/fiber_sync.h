#ifndef __ALPHA_FIBER_SYNC_H__
#define __ALPHA_FIBER_SYNC_H__

#include <atomic>
#include <deque>
#include <list>
#include "alpha.h"

namespace alphaMin {

// 互斥量
class FiberMutex {
public:
    typedef std::shared_ptr<FiberMutex> ptr;

    FiberMutex() = default;

    void lock() {
        while(m_locked.exchange(true)) {
            // 当前协程无法获取锁，将其添加到等待队列，并让出 CPU 控制权
            m_waiting.push_back(Fiber::GetThis());
            Fiber::YieldToHold();
        }
    }

    void unlock() {
        m_locked.store(false);
        // 释放锁后，唤醒等待队列中的第一个协程（如果有）
        if (!m_waiting.empty()) {
            auto next = m_waiting.front();
            m_waiting.pop_front();
            // Scheduler::GetThis()->schedule(next);
            IOManager::GetThis()->schedule(next);
        }
    }

private:
    std::atomic<bool> m_locked{false};
    std::list<Fiber::ptr> m_waiting; // 等待队列
};

class FiberCondition {
public:
    typedef std::shared_ptr<FiberCondition> ptr;

    FiberCondition() = default;

    void wait(FiberMutex& mutex) {
        m_waiting.push_back(Fiber::GetThis()); // 将当前协程添加到等待队列中
        mutex.unlock();
        Fiber::YieldToHold();
        mutex.lock();
    }

    void notify_one() {
        if(!m_waiting.empty()) {
            auto next = m_waiting.front();
            m_waiting.pop_front();
            // Scheduler::GetThis()->schedule(next);
            IOManager::GetThis()->schedule(next);
        }
    }

    void notify_all() {
        while(!m_waiting.empty()) {
            auto next = m_waiting.front();
            m_waiting.pop_front();
            // Scheduler::GetThis()->schedule(next);
            IOManager::GetThis()->schedule(next);
        }
    }

private:
    std::list<Fiber::ptr> m_waiting; // 等待队列
};

}

#endif