#include "multi_plexer.h"
#include "log.h"
#include <unistd.h>
#include <cstring>
#include <algorithm>

namespace alphaMin {

static alphaMin::Logger::ptr g_logger = ALPHA_LOG_NAME("system");

SelectMultiplexer::SelectMultiplexer()
    :m_maxfd(0) {
    FD_ZERO(&m_readfds);
    FD_ZERO(&m_writefds);
    FD_ZERO(&m_errorfds);
}

SelectMultiplexer::~SelectMultiplexer() {}

void SelectMultiplexer::addFileDescriptor(int fd) {
    FD_SET(fd, &m_readfds);
    if(fd > m_maxfd) {
        m_maxfd = fd;
    }
}

void SelectMultiplexer::removeFileDescriptor(int fd) {
    FD_CLR(fd, &m_readfds);
    FD_CLR(fd, &m_writefds);
    FD_CLR(fd, &m_errorfds);
    // 修改 m_maxfd
}

std::vector<int> SelectMultiplexer::waitForEvents() {
    fd_set tmp_readfds = m_readfds;
    fd_set tmp_writefds = m_writefds;
    fd_set tmp_errorfds = m_errorfds;

    int ready_fds = select(m_maxfd + 1, &tmp_readfds, &tmp_writefds, &tmp_errorfds, nullptr);

    std::vector<int> ready_events;
    for(int fd = 0; fd <= m_maxfd && ready_fds > 0; ++fd) {
        if(FD_ISSET(fd, &tmp_readfds) || FD_ISSET(fd, &tmp_writefds) || FD_ISSET(fd, &tmp_errorfds)) {
            ready_events.push_back(fd);
            --ready_fds;
        }
    }

    return ready_events;
}

PollMultiplexer::PollMultiplexer() {}

PollMultiplexer::~PollMultiplexer() {}

void PollMultiplexer::addFileDescriptor(int fd) {
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN | POLLOUT;
    pfd.revents = 0;
    m_pollfds.push_back(pfd);
}

void PollMultiplexer::removeFileDescriptor(int fd) {
    auto it = std::remove_if(m_pollfds.begin(), m_pollfds.end(),
                            [fd](const pollfd &pfd) { return pfd.fd == fd;});
    m_pollfds.erase(it, m_pollfds.end());
}

std::vector<int> PollMultiplexer::waitForEvents() {
    int ready_fds = poll(m_pollfds.data(), m_pollfds.size(), -1);

    std::vector<int> ready_events;
    if(ready_fds > 0) {
        for(const auto &pfd : m_pollfds) {
            if(pfd.revents & (POLLIN | POLLOUT | POLLERR | POLLHUP)) {
                ready_events.push_back(pfd.fd);
            }
        }
    }

    return ready_events;
}

EpollMultiplexer::EpollMultiplexer() {
    m_epollfd = epoll_create(5000);
    if(-1 == m_epollfd) {
        ALPHA_LOG_ERROR(g_logger) << "Failed to create epoll instance";
    }
}

EpollMultiplexer::~EpollMultiplexer() {
    close(m_epollfd);
}

void EpollMultiplexer::addFileDescriptor(int fd) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLOUT | EPOLLET; // Edge-triggered mode
    if(epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event) == -1) {
        ALPHA_LOG_ERROR(g_logger) << "Failed to add file descriptor to epoll instance";
    }
}

void EpollMultiplexer::removeFileDescriptor(int fd) {
    if(epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        ALPHA_LOG_ERROR(g_logger) << "Failed to remove file descriptor from epoll instance";
    }
}

std::vector<int> EpollMultiplexer::waitForEvents() {
    const int max_events = 256;
    epoll_event events[max_events];

    int ready_fds = epoll_wait(m_epollfd, events, max_events, -1);
    if(ready_fds == -1) {
        ALPHA_LOG_ERROR(g_logger) << "Failed to wait for events using epoll instance";
    }

    std::vector<int> ready_events;
    for(int i = 0; i < ready_fds; ++i) {
        ready_events.push_back(events[i].data.fd);
    }

    return ready_events;
}

}