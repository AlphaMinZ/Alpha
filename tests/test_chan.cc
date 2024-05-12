#include "../alpha/alpha.h"
#include "../alpha/channel.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_NAME("system");

void sender(alphaMin::Chan<int>& ch) {
    for (int i = 0; i < 3; ++i) {
        std::string str;
        std::cin >> str;
        int tmp = std::stoi(str);
        ch.send(tmp);
        ALPHA_LOG_INFO(g_logger) << "Sent: " << tmp;
    }
    ch.close();
    ALPHA_LOG_INFO(g_logger) << "block chan is closed";
}

void receiver1(alphaMin::Chan<int>& ch) {
    while (true) {
        auto result = ch.receive();
        int value = result.first;
        bool closed = result.second;
        if (closed) {
            ALPHA_LOG_INFO(g_logger) << "Channel is closed";
            break;
        }
        ALPHA_LOG_INFO(g_logger) << "Receiver " << 1 << " received: " << value;
    }
}

void receiver2(alphaMin::Chan<int>& ch) {
    while (true) {
        auto result = ch.receive();
        int value = result.first;
        bool closed = result.second;
        if (closed) {
            ALPHA_LOG_INFO(g_logger) << "Channel is closed";
            break;
        }
        ALPHA_LOG_INFO(g_logger) << "Receiver " << 2 << " received: " << value;
    }
}

void sender_unblock(alphaMin::Chan<int>& ch) {
    for (int i = 0; i < 3; ++i) {
        std::string str;
        std::cin >> str;
        int tmp = std::stoi(str);
        bool success = ch.send(tmp);
        if(success) {
            ALPHA_LOG_INFO(g_logger) << "Sent: " << tmp;
        } else {
            ALPHA_LOG_INFO(g_logger) << "Channel is full";
        }
    }
    ch.close();
    ALPHA_LOG_INFO(g_logger) << "block chan is closed";
}

void receiver_unblock(alphaMin::Chan<int>& ch) {
    while (true) {
        auto result = ch.receive();
        int value = result.first;
        bool closed = result.second;
        if (closed) {
            ALPHA_LOG_INFO(g_logger) << "Channel is closed";
            break;
        }
        if(value != 0) {
            ALPHA_LOG_INFO(g_logger) << "receiver: " << value;
        }
    }
}

int main(int argc, char** argv) {
    alphaMin::Chan<int> ch(2, false);
    alphaMin::Scheduler sc(2);
    sc.start();
    // sc.schedule([&ch]() {
    //     sender_unblock(ch);
    // });
    sc.schedule([&ch]() {
        receiver_unblock(ch);
    });
    // sc.schedule([&ch]() {
    //     receiver2(ch);
    // });
    sc.stop(); 
    return 0;
}
