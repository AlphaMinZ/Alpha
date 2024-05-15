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

// void sender_unblock(alphaMin::Chan<int>& ch) {
//     for (int i = 0; i < 3; ++i) {
//         std::string str;
//         std::cin >> str;
//         int tmp = std::stoi(str);
//         bool success = ch.send(tmp);
//         if(success) {
//             ALPHA_LOG_INFO(g_logger) << "Sent: " << tmp;
//         } else {
//             ALPHA_LOG_INFO(g_logger) << "Channel is full";
//         }
//     }
//     ch.close();
//     ALPHA_LOG_INFO(g_logger) << "block chan is closed";
// }

// void receiver_unblock(alphaMin::Chan<int>& ch) {
//     while (true) {
//         auto result = ch.receive();
//         auto data = std::move(result.first);
//         bool closed = result.second;
//         if (closed) {
//             ALPHA_LOG_INFO(g_logger) << "Channel is closed";
//             break;
//         }
//         if (data) {
//             ALPHA_LOG_INFO(g_logger) << *data;
//         } else {
//             ALPHA_LOG_INFO(g_logger) << "Received nullptr";
//         }
//         sleep(1);
//     } 
// }

void send_3(alphaMin::Chan<int>& ch_int, alphaMin::Chan<std::string>& ch_str) {
    // std::string str;
    // std::cin >> str;
    // int tmp = std::stoi(str);
    // bool success = ch_int.send(tmp);

    // std::string str_2;
    // std::cin >> str_2;
    // success = ch_str.send(str_2);

    // std::string str_3;
    // std::cin >> str_3;
    // tmp = std::stoi(str_3);
    // success = ch_int.send(tmp);

    // ch_int.send(999);
    // ch_int.send(888);
    // ch_int.send(777);
    // ch_int.send(666);
    // ch_int.send(555);
    // ch_int.send(444);
    ch_str.send("hello");

    // ch_int.close();
    // ch_str.close();
}

void read_3(alphaMin::Chan<int>& ch_int, alphaMin::Chan<std::string>& ch_str) {

    // alphaMin::FiberCondition cond;
    auto cond = std::make_shared<alphaMin::FiberCondition>();
    // alphaMin::FiberMutex mutex;
    auto mutex = std::make_shared<alphaMin::FiberMutex>();
    auto tmp_int = 0;
    std::string tmp_str;
    // std::pair<std::unique_ptr<int>, bool> result_int;
    auto result_int = std::make_shared<std::pair<std::unique_ptr<int>, bool> >();
    result_int->second = false;
    ALPHA_LOG_INFO(g_logger) << "result_int's count is " << result_int.use_count();
    // std::pair<std::unique_ptr<std::string>, bool> result_str;
    auto result_str = std::make_shared<std::pair<std::unique_ptr<std::string>, bool> >();
    result_str->second = false;

    while(true) {
        if(!result_int->second) {
            alphaMin::IOManager::GetThis()->schedule([&ch_int, &tmp_int, cond, result_int, mutex]() {
                ALPHA_LOG_INFO(g_logger) << "start read ch_int====================================";
                ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
                ALPHA_LOG_INFO(g_logger) << "result_int's count is " << result_int.use_count();
                auto result = ch_int.receive();
                ALPHA_LOG_INFO(g_logger) << "next to lock";
                ALPHA_LOG_INFO(g_logger) << "mutex's count is " << mutex.use_count();
                mutex->lock();
                *result_int = std::move(result);
                ALPHA_LOG_INFO(g_logger) << "result_int's count is " << result_int.use_count();
                mutex->unlock();
                ALPHA_LOG_INFO(g_logger) << "have read from ch_int";
                auto tmp = std::move(result_int->first);
                if(tmp == nullptr) {
                    ALPHA_LOG_INFO(g_logger) << "read a nullptr int";
                }
                if(tmp != nullptr) {
                    tmp_int = *tmp;
                    ALPHA_LOG_INFO(g_logger) << "tmp_int = " << tmp_int;
                }
                ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
                cond->notify_all();
                ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
            });
        }

        if(!result_str->second) {
            alphaMin::IOManager::GetThis()->schedule([&ch_str, &tmp_str, cond, result_str, mutex]() {
                ALPHA_LOG_INFO(g_logger) << "start read ch_str====================================";
                ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
                ALPHA_LOG_INFO(g_logger) << "result_str's count is " << result_str.use_count();
                auto result = ch_str.receive();
                ALPHA_LOG_INFO(g_logger) << "next to lock";
                ALPHA_LOG_INFO(g_logger) << "mutex's count is " << mutex.use_count();
                mutex->lock();
                *result_str = std::move(result);
                ALPHA_LOG_INFO(g_logger) << "result_str's count is " << result_str.use_count();
                mutex->unlock();
                ALPHA_LOG_INFO(g_logger) << "have read from ch_str";
                auto tmp = std::move(result_str->first);
                if(tmp == nullptr) {
                    ALPHA_LOG_INFO(g_logger) << "read a nullptr str";
                }
                if(tmp != nullptr) {
                    tmp_str = *tmp;
                    ALPHA_LOG_INFO(g_logger) << "tmp_str = " << tmp_str;
                }
                ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
                cond->notify_all();
                ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
            });
        }

        ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
        cond->wait(*mutex);
        ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
        ALPHA_LOG_INFO(g_logger) << "已经从wait中出来了";
        if(result_int->second && result_str->second) {
            mutex->unlock();
            ALPHA_LOG_INFO(g_logger) << "result_str's count is " << result_str.use_count();
            ALPHA_LOG_INFO(g_logger) << "result_int's count is " << result_int.use_count();
            ALPHA_LOG_INFO(g_logger) << "mutex's count is " << mutex.use_count();
            ALPHA_LOG_INFO(g_logger) << "cond's count is " << cond.use_count();
            break;
        }
    }
    ALPHA_LOG_INFO(g_logger) << "read stop";
}

int main(int argc, char** argv) {
    alphaMin::Chan<int> ch_int(2);
    alphaMin::Chan<std::string> ch_str(2);
    alphaMin::IOManager iom(3);
    
    ch_int.close();
    ch_str.close();

    iom.schedule([&ch_int, &ch_str]() {
        send_3(ch_int, ch_str);
    });

    // sleep(2);

    iom.schedule([&ch_int, &ch_str]() {
        read_3(ch_int, ch_str);
    });

    // sleep(2);

    // ch_int.close();
    // ch_str.close();

    return 0;
}
