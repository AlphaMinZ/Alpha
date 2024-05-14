#ifndef __ALPHA_OBJECT_POOL_H__
#define __ALPHA_OBJECT_POOL_H__

#include "fiber_sync.h"
#include "log.h"
#include <list>
#include <functional>
#include <memory>
#include <unordered_map>

namespace alphaMin {

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_NAME("system");

// 只能在使用 iomanage 的情况下使用此对象池
template<class T>
class ObjectPool {
public:
    typedef std::shared_ptr<ObjectPool> ptr;

    ObjectPool(size_t maxSize = 100,
                std::function<std::shared_ptr<T>()> creator = [](){ return std::make_shared<T>();},
                std::function<void(std::shared_ptr<T>)> destroyer = [](std::shared_ptr<T> obj){},
                std::function<void(std::shared_ptr<T>)> resetter = [](std::shared_ptr<T> obj){})
        :m_maxSize(maxSize)
        ,m_creator(creator)
        ,m_destroyer(destroyer)
        ,m_resetter(resetter) {}

    ~ObjectPool() {}

    std::shared_ptr<T> get() {
        m_mutex.lock();

        std::shared_ptr<T> obj;
        if(!m_pool.empty()) {
            obj = m_pool.front();
            m_pool.pop_front();
            m_inUse[obj.get()] = true;
        } else if(m_pool.size() < m_maxSize) {
            obj = m_creator();
            m_inUse[obj.get()] = true;
        }
        m_resetter(obj);

        m_mutex.unlock();
        return obj;
    }

    void put(std::shared_ptr<T> obj) {
        m_mutex.lock();

        auto it = m_inUse.find(obj.get());
        if(it != m_inUse.end() && it->second) {
            m_inUse[obj.get()] = false;
            if(m_pool.size() < m_maxSize) {
                m_pool.push_back(obj);
            } else {
                ALPHA_LOG_WARN(g_logger) << "Warning: Pool is full. Discarding object.";
            }
        } else {
            ALPHA_LOG_WARN(g_logger) << "Warning: Object not released properly.";
        }

        m_mutex.unlock();
    }

    size_t get_size() {
        m_mutex.lock();
        size_t size = m_pool.size();
        m_mutex.unlock();
        return size;
    }

    size_t get_maxSize() const {
        return m_maxSize;
    }

    void setMaxSize(size_t maxSize) {
        m_mutex.lock();
        m_maxSize = maxSize;
        m_mutex.unlock();
    }

    void expandPool(size_t n) {
        m_mutex.lock();
        m_maxSize += n;
        m_mutex.unlock();
    }

    void ShrinkPool(size_t n) {
        m_mutex.lock();
        if(n < m_maxSize) {
            m_maxSize -= n;
            while(m_pool.size() > m_maxSize) {
                m_pool.pop_back();
            }
        }

        m_mutex.unlock();
    }

    void close() {
        m_mutex.lock();
        m_pool.clear();
        m_inUse.clear();
        m_mutex.unlock();
    }

private:
    std::list<std::shared_ptr<T> > m_pool; // 存储对象的池子
    FiberMutex m_mutex; // 协程锁
    std::unordered_map<T*, bool> m_inUse; // 对象是否正在被使用
    std::function<std::shared_ptr<T>()> m_creator; // 创建对象
    std::function<void(std::shared_ptr<T>)> m_destroyer; // 销毁对象
    std::function<void(std::shared_ptr<T>)> m_resetter; // 对象重置函数
    size_t m_maxSize;
};

}

#endif