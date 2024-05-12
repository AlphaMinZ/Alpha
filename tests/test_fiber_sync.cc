#include "../alpha/alpha.h"
#include "../alpha/fiber_sync.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

alphaMin::FiberMutex fm;

void test1() {
    fm.lock();
    ALPHA_LOG_INFO(g_logger) << "test1 is lock";
    sleep(2);
    // alphaMin::Fiber::YieldToHold();
    fm.unlock();
}

void test2() {
    fm.lock();
    ALPHA_LOG_INFO(g_logger) << "test2 is lock";
}

int main(int argc, char** argv) {
    alphaMin::Scheduler scheduler(1, true, "test");

    scheduler.start();
    scheduler.schedule(&test1);
    scheduler.schedule(&test2);

    scheduler.stop();

    return 0;
}