#include "fiber.h"
#include "log.h"
#include "scheduler.h"

namespace alphaMin {

static Logger::ptr g_logger = ALPHA_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

// 线程局部变量， 保存当前正在运行的协程指针，必须时刻指向当前正在运行的协程对象。
// 协程模块初始化时，t_fiber指向线程主协程对象。
static thread_local Fiber* t_fiber = nullptr;

// 保存线程主协程指针，智能指针形式。协程模块初始化时，t_thread_fiber指向线程主协程对象。
// 当子协程resume时，通过swapcontext将主协程的上下文保存到t_thread_fiber的ucontext_t成员中，
// 同时激活子协程的ucontext_t上下文。当子协程yield时，
// 从t_thread_fiber中取得主协程的上下文并恢复运行。
static thread_local Fiber::ptr t_threadFiber = nullptr;

class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;    // 取个别名

// 每个线程的第一个协程
Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        // TODO(AlphaMinZ)
    }

    ++s_fiber_count;

    ALPHA_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

/**
* @brief 构造函数
* @param[in] cb 协程执行的函数
* @param[in] stacksize 协程栈大小
* @param[in] use_caller 是否在MainFiber上调度
*/
Fiber::Fiber(std::function<void()> cb, size_t stackSize, bool use_caller)
    :m_id(++s_fiber_id)
    ,m_cb(cb) {
    ++s_fiber_count;
    m_stackSize = stackSize ? stackSize : 128 * 1024;

    m_stack = StackAllocator::Alloc(m_stackSize);
    if(getcontext(&m_ctx)) {

    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stackSize;

    if(!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    ALPHA_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {

        StackAllocator::Dealloc(m_stack, m_stackSize);
    } else {

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    ALPHA_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id
                                << " total = " << s_fiber_count;
}

// 重置协程执行函数为 INIT
void Fiber::reset(std::function<void()> cb) {

    m_cb = cb;
    if(getcontext(&m_ctx)) {

    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stackSize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

// 将当前协程切换到运行状态
void Fiber::swapIn() {
    SetThis(this);

    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {

    }
}

// 将当前协程切换到后台
void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {

    }
}

// 线程主协程调用 将当前协程切换到运行状态
void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {

    }
}

// 返回到主协程
void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {

    }
}

// 设置当前线程的运行协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

// 返回当前所在的协程
Fiber::ptr Fiber::GetThis() {
    if(t_fiber) {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);

    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

// 将当前协程切换到后台,并设置为READY状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();

    cur->m_state = READY;
    cur->swapOut();
}

// 将当前协程切换到后台,并设置为HOLD状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();

    // cur->m_state = HOLD;
    cur->swapOut();
}

// 返回当前协程的总数量
uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

// 协程执行函数 执行完成返回到线程主协程
void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();

    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        ALPHA_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id = " << cur->getId();
    } catch (...) {
        cur->m_state = EXCEPT;
        ALPHA_LOG_ERROR(g_logger) << "Fiber Except: "
            << " fiber_id = " << cur->getId();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();


}

// 协程执行函数 执行完成返回到线程调度协程
void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();

    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        ALPHA_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id = " << cur->getId();
    } catch (...) {
        cur->m_state = EXCEPT;
        ALPHA_LOG_ERROR(g_logger) << "Fiber Except: "
            << " fiber_id = " << cur->getId();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();


}

// 获取当前协程的 id 
uint64_t Fiber::GetFiberId() {
    if(t_fiber) {
        return t_fiber->getId();
    }

    return 0;
}

}