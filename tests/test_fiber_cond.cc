#include "../alpha/fiber_sync.h"
#include "../alpha/alpha.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

alphaMin::FiberMutex fm;
alphaMin::FiberCondition fc;

// 在协程中等待条件变量的通知
void worker() {
    std::cout << "Worker: acquiring lock" << std::endl;
    fm.lock();  // 获取互斥锁
    std::cout << "Worker: acquired lock, waiting..." << std::endl;
    fc.wait(fm);  // 等待条件变量
    std::cout << "Worker: received notification, releasing lock" << std::endl;
    fm.unlock();  // 释放互斥锁
}

int main(int argc, char** argv) {
    alphaMin::Scheduler scheduler(2, true);
    scheduler.start();

    // 创建一个协程执行 worker 函数
    scheduler.schedule(worker);

    // 主线程等待一段时间后发送通知给等待的协程
    // sleep(2);
    std::string str;
    std::cin >> str;
    std::cout << "Main: notifying worker" << std::endl;
    fm.lock();  // 获取互斥锁
    fc.notify_one();  // 发送通知
    fm.unlock();  // 释放互斥锁

    scheduler.stop();
    return 0;
}