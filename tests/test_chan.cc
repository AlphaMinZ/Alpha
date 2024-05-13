#include "../alpha/alpha.h"
#include "../alpha/channel.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_NAME("system");

// void sender(alphaMin::Chan<int>& ch) {
//     for (int i = 0; i < 3; ++i) {
//         std::string str;
//         std::cin >> str;
//         int tmp = std::stoi(str);
//         ch.send(tmp);
//         ALPHA_LOG_INFO(g_logger) << "Sent: " << tmp;
//     }
//     ch.close();
//     ALPHA_LOG_INFO(g_logger) << "block chan is closed";
// }

// void receiver1(alphaMin::Chan<int>& ch) {
//     while (true) {
//         auto result = ch.receive();
//         int value = result.first;
//         bool closed = result.second;
//         if (closed) {
//             ALPHA_LOG_INFO(g_logger) << "Channel is closed";
//             break;
//         }
//         ALPHA_LOG_INFO(g_logger) << "Receiver " << 1 << " received: " << value;
//     }
// }

// void receiver2(alphaMin::Chan<int>& ch) {
//     while (true) {
//         auto result = ch.receive();
//         int value = result.first;
//         bool closed = result.second;
//         if (closed) {
//             ALPHA_LOG_INFO(g_logger) << "Channel is closed";
//             break;
//         }
//         ALPHA_LOG_INFO(g_logger) << "Receiver " << 2 << " received: " << value;
//     }
// }

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
        auto data = std::move(result.first);
        bool closed = result.second;
        if (closed) {
            ALPHA_LOG_INFO(g_logger) << "Channel is closed";
            break;
        }
        if (data) {
            ALPHA_LOG_INFO(g_logger) << *data;
        } else {
            ALPHA_LOG_INFO(g_logger) << "Received nullptr";
        }
        sleep(1);
    } 
}

int main(int argc, char** argv) {
    alphaMin::Chan<int> ch(2, false);
    alphaMin::IOManager iom(2);
    
    iom.schedule([&ch]() {
        receiver_unblock(ch);
    });
    
    iom.schedule([&ch]() {
        sender_unblock(ch);
    });

    return 0;
}
