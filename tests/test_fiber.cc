#include "../alpha/fiber.h"
#include "../alpha/thread.h"

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void run_in_fiber() {
    ALPHA_LOG_INFO(g_logger) << "run_in_fiber begin";
    alphaMin::Fiber::YieldToHold();
    ALPHA_LOG_INFO(g_logger) << "run_in_fiber begin";
    alphaMin::Fiber::YieldToHold();
}

void test_fiber() {
    ALPHA_LOG_INFO(g_logger) << "main begin";
    {
        alphaMin::Fiber::GetThis();
        ALPHA_LOG_INFO(g_logger) << "main begin";
        alphaMin::Fiber::ptr fiber(new alphaMin::Fiber(run_in_fiber));
        fiber->swapIn();
        ALPHA_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        ALPHA_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    ALPHA_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    alphaMin::Thread::SetName("main");

    std::vector<alphaMin::Thread::ptr> thrs;

    for(int i = 0; i < 5; i++) {
        thrs.push_back(alphaMin::Thread::ptr(
                    new alphaMin::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    // test_fiber();

    return 0;
}