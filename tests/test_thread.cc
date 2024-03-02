#include "thread.h"
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>
#include <vector>


int count = 0;

void fun1() {
    std::cout << "[name] " << alphaMin::Thread::GetName()
                << ", [this.name] " << alphaMin::Thread::GetThis()->GetName()
                << ", [id] " << syscall(SYS_gettid)
                << ", [this.id] " << alphaMin::Thread::GetThis()->getId()
                << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "[thread test begin]" << std::endl;

    std::vector<alphaMin::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        alphaMin::Thread::ptr thr(new alphaMin::Thread(&fun1, "name_" + std::to_string(i * 1)));

        thrs.push_back(thr);


    }

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    std::cout << "[thread test end]" << std::endl;
    return 0;
}