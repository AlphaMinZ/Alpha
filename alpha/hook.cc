#include "hook.h"
#include <dlfcn.h>
#include <stdarg.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#include "log.h"
#include "fiber.h"
#include "iomanager.h"
#include "fd_manager.h"

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_NAME("system");
namespace alphaMin {

// static alphaMin::ConfigVar<int>::ptr g_tcp_connect_timeout =
//     alphaMin::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
    _HookIniter() {
        hook_init();
        // s_connect_timeout = g_tcp_connect_timeout->getValue();
        s_connect_timeout = 5000;

        // g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
        //         alphaMin_LOG_INFO(g_logger) << "tcp connect timeout changed from "
        //                                  << old_value << " to " << new_value;
        //         s_connect_timeout = new_value;
        // });
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

}

struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {
    if(!alphaMin::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    alphaMin::FdCtx::ptr ctx = alphaMin::FdMgr::GetInstance()->get(fd);
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);

    /*
        这里如果 n 为正数则说明执行成功，则直接返回
        否则则是有问题，进入下面逻辑
    */

    while(n == -1 && errno == EINTR) {
        // 如果是中断的话，则一直重试
        n = fun(fd, std::forward<Args>(args)...);
    }
    // 此时就需要做异步操作了
    if(n == -1 && errno == EAGAIN) {
        alphaMin::IOManager* iom = alphaMin::IOManager::GetThis();
        alphaMin::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(to != (uint64_t)-1) {
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                // 这里定时器执行函数里面条件没了，或者取消了，就退出这个条件定时器的回调
                if(!t || t->cancelled) {
                    return;
                }
                // 如果不是的话，那就是时间超过了设置的超时时间
                t->cancelled = ETIMEDOUT;
                // 把这个 io 事件取消掉，因为已经超时了
                iom->cancelEvent(fd, (alphaMin::IOManager::Event)(event));
            }, winfo);
        }

        // 没有带回调函数，就是默认将本协程作为回调函数
        // 这里是先添加一个条件定时器来判断是否会超时，然后再将本协程作为事件添加进入
        // 定时器判断超时会将此事件删除，若在定时器超市之前此事件执行了，定时器就之间返回
        int rt = iom->addEvent(fd, (alphaMin::IOManager::Event)(event));
        if(rt) {
            // 添加事件失败，取消定时器
            ALPHA_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            // 添加事件成功，让出执行权
            alphaMin::Fiber::YieldToHold();
            // 事件返回之后再将定时器取消
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                // 这里是说明此事件是由定时器唤醒的，说明已经超时了
                errno = tinfo->cancelled;
                return -1;
            }
            // 这里就是有 io 事件回来了，就去重新读
            goto retry;
        }
    }
    
    return n;
}

extern "C" {

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if(!alphaMin::t_hook_enable) {
        return sleep_f(seconds);
    }

    alphaMin::Fiber::ptr fiber = alphaMin::Fiber::GetThis();
    alphaMin::IOManager* iom = alphaMin::IOManager::GetThis();
    /*
        void: 这表示成员函数的返回类型。在这个例子中，成员函数的返回类型是 void，即没有返回值。
        (alphaMin::Scheduler::*): 这表示这是一个指向 alphaMin::Scheduler 类中成员函数的指针。
        (alphaMin::Fiber::ptr, int thread): 这是成员函数的参数列表。
    */
    iom->addTimer(seconds * 1000, std::bind((void(alphaMin::Scheduler::*)
            (alphaMin::Fiber::ptr, int thread))&alphaMin::IOManager::schedule
            ,iom, fiber, -1));
    alphaMin::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if(!alphaMin::t_hook_enable) {
        return usleep_f(usec);
    }
    alphaMin::Fiber::ptr fiber = alphaMin::Fiber::GetThis();
    alphaMin::IOManager* iom = alphaMin::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(alphaMin::Scheduler::*)
            (alphaMin::Fiber::ptr, int thread))&alphaMin::IOManager::schedule
            ,iom, fiber, -1));
    alphaMin::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!alphaMin::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    alphaMin::Fiber::ptr fiber = alphaMin::Fiber::GetThis();
    alphaMin::IOManager* iom = alphaMin::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(alphaMin::Scheduler::*)
            (alphaMin::Fiber::ptr, int thread))&alphaMin::IOManager::schedule
            ,iom, fiber, -1));
    alphaMin::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if(!alphaMin::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    alphaMin::FdMgr::GetInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
    if(!alphaMin::t_hook_enable) {
        return connect_f(fd, addr, addrlen);
    }
    alphaMin::FdCtx::ptr ctx = alphaMin::FdMgr::GetInstance()->get(fd);
    if(!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }
    
    if(ctx->getUserNonblock()) {
        return connect_f(fd, addr, addrlen);
    }

    int n = connect_f(fd, addr, addrlen);
    if(n == 0) {
        return 0;
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    alphaMin::IOManager* iom = alphaMin::IOManager::GetThis();
    alphaMin::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, alphaMin::IOManager::WRITE);
        }, winfo);
    }

    int rt = iom->addEvent(fd, alphaMin::IOManager::WRITE);
    if(rt == 0) {
        alphaMin::Fiber::YieldToHold();
        if(timer) {
            timer->cancel();
        }
        if(tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        ALPHA_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, alphaMin::s_connect_timeout);
}

int accept(int s, struct sockaddr* addr, socklen_t* addrlen) {
    int fd = do_io(s, accept_f, "accept", alphaMin::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        alphaMin::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void* buf, size_t count) {
    return do_io(fd, read_f, "read", alphaMin::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec* iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", alphaMin::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", alphaMin::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", alphaMin::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", alphaMin::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", alphaMin::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", alphaMin::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", alphaMin::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", alphaMin::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", alphaMin::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if(!alphaMin::t_hook_enable) {
        return close_f(fd);
    }

    alphaMin::FdCtx::ptr ctx = alphaMin::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        auto iom = alphaMin::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        alphaMin::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                alphaMin::FdCtx::ptr ctx = alphaMin::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                alphaMin::FdCtx::ptr ctx = alphaMin::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        alphaMin::FdCtx::ptr ctx = alphaMin::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!alphaMin::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            alphaMin::FdCtx::ptr ctx = alphaMin::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}