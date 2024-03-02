#include "../alpha/log.h"
#include "../alpha/iomanager.h"
#include "../alpha/thread.h"
#include "../alpha/fiber.h"
#include "../alpha/scheduler.h"
#include "../alpha/timer.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

int sock = 0;

void test_fiber() {
    ALPHA_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //alphaMin::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        ALPHA_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        alphaMin::IOManager::GetThis()->addEvent(sock, alphaMin::IOManager::READ, [](){
            ALPHA_LOG_INFO(g_logger) << "read callback";
        });
        alphaMin::IOManager::GetThis()->addEvent(sock, alphaMin::IOManager::WRITE, [](){
            ALPHA_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            alphaMin::IOManager::GetThis()->cancelEvent(sock, alphaMin::IOManager::READ);
            close(sock);
        });
    } else {
        ALPHA_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    alphaMin::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

alphaMin::Timer::ptr s_timer;
void test_timer() {
    alphaMin::IOManager iom(1);
    // ALPHA_LOG_INFO(g_logger) << "+++++++++++++++++++++++";
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        ALPHA_LOG_INFO(g_logger) << "hello timer i = " << i;
        if(++i == 3) {
            s_timer->reset(2000, true);
        }
        if(i == 6) {
            s_timer->cancel();
        }
    }, true);
}

int main(int argc, char** argv) {
    // test1();
    test_timer();
    return 0;
}