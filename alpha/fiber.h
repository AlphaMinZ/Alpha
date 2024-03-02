#ifndef __ALPHA_FIBER_H__
#define __ALPHA_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>

#include "log.h"

namespace alphaMin {

class Scheduler;

// 协程类
class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    // 协程状态
    enum State {
        /// 初始化状态
        INIT,
        /// 暂停状态
        HOLD,
        /// 执行中状态
        EXEC,
        /// 结束状态
        TERM,
        /// 可执行状态
        READY,
        /// 异常状态
        EXCEPT
    };

private:
    // 每个线程的第一个协程
    Fiber();

public:
    /**
     * @brief 构造函数
     * @param[in] cb 协程执行的函数
     * @param[in] stacksize 协程栈大小
     * @param[in] use_caller 是否在MainFiber上调度
     */
    Fiber(std::function<void()> cb, size_t stackSize = 0, bool use_caller = false);

    ~Fiber();

    // 重置协程执行函数为 INIT
    void reset(std::function<void()> cb);

    // 将当前协程切换到运行状态
    void swapIn();

    // 将当前协程切换到后台
    void swapOut();

    // 线程主协程调用 将当前协程切换到运行状态
    void call();

    // 返回到主协程
    void back();

    // 返回协程 id
    uint64_t getId() const { return m_id;}

    // 返回协程状态
    State getState() const { return m_state;}

    // 设置当前线程的运行协程
    static void SetThis(Fiber* f);

    // 返回当前所在的协程
    static Fiber::ptr GetThis();

    // 将当前协程切换到后台,并设置为READY状态
    static void YieldToReady();

    // 将当前协程切换到后台,并设置为HOLD状态
    static void YieldToHold();

    // 返回当前协程的总数量
    static uint64_t TotalFibers();

    // 协程执行函数 执行完成返回到线程主协程
    static void MainFunc();

    // 协程执行函数 执行完成返回到线程调度协程
    static void CallerMainFunc();

    // 获取当前协程的 id 
    static uint64_t GetFiberId();
private:
    /// 协程id
    uint64_t m_id = 0;
    /// 协程运行栈大小
    uint32_t m_stackSize = 0;
    /// 协程状态
    State m_state = INIT;
    /// 协程上下文
    ucontext_t m_ctx;
    /// 协程运行栈指针
    void* m_stack = nullptr;
    /// 协程运行函数
    std::function<void()> m_cb;
};

}

#endif