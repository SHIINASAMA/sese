#pragma once

#include "sese/event/Event.h"
#include "sese/net/Socket.h"
#include "sese/thread/Thread.h"

#include <queue>

namespace sese::service {
    class UserBalanceLoader;
    class MasterEventLoop;
}// namespace sese::service

class API sese::service::UserBalanceLoader {
public:
    ~UserBalanceLoader() noexcept;

    void setThreads(size_t th) noexcept;

    void setAddress(const net::IPAddress::Ptr &addr) { UserBalanceLoader::address = addr; }

    bool isStarted() const { return _isStart; }

    template<class Service>
    bool init() noexcept;

    void start() noexcept;

    void stop() noexcept;

protected:
    void master() noexcept;

    void slave(
            sese::event::EventLoop *eventLoop,
            std::queue<socket_t> *masterQueue,
            std::queue<socket_t> *slaveQueue,
            std::mutex *mutex
    ) noexcept;

protected:
    std::atomic_bool _isStart{false};
    std::atomic_bool _isStop{false};

    size_t threads{2};
    std::vector<sese::event::EventLoop *> eventLoopVector;
    std::vector<sese::Thread::Ptr> threadVector;
    sese::net::IPAddress::Ptr address = sese::net::IPv4Address::localhost(8080);

    sese::net::Socket *socket{nullptr};
    sese::service::MasterEventLoop *masterEventLoop{nullptr};
    sese::Thread::Ptr masterThread{nullptr};
    /// socket_t 交换队列
    std::queue<socket_t> *masterSocketQueueArray{nullptr};
    std::queue<socket_t> *slaveSocketQueueArray{nullptr};
    std::mutex *mutexArray{nullptr};
};

class sese::service::MasterEventLoop final : public sese::event::EventLoop {
public:
    void onAccept(int fd) override;

    std::queue<socket_t> socketQueue;
};

template<class Service>
bool sese::service::UserBalanceLoader::init() noexcept {
    if (address == nullptr) return false;

    socket = new net::Socket(
            address->getRawAddress()->sa_family == AF_INET ? net::Socket::Family::IPv4 : net::Socket::Family::IPv6,
            net::Socket::Type::TCP
    );
    if (-1 == socket->getRawSocket()) {
        return false;
    }

    if (!socket->setNonblocking(true)) {
        goto free_socket;
    }

    if (0 != socket->bind(address)) {
        goto free_socket;
    }

    if (0 != socket->listen(32)) {
        goto free_socket;
    }

    masterEventLoop = new MasterEventLoop;
    masterEventLoop->setListenFd((int) socket->getRawSocket());
    if (!masterEventLoop->init()) {
        goto free_master;
    }

    for (size_t i = 0; i < threads; ++i) {
        auto event = new Service;
        if (!event->init()) {
            delete event;
            goto free_event;
        } else {
            eventLoopVector.emplace_back(event);
        }
    }

    // 初始化交换队列
    masterSocketQueueArray = new std::queue<socket_t>[threads];
    slaveSocketQueueArray = new std::queue<socket_t>[threads];
    mutexArray = new std::mutex[threads];

    return true;

free_event:
    for (decltype(auto) eventLoop: eventLoopVector) {
        delete eventLoop;
    }
    eventLoopVector.clear();

free_master:
    delete masterEventLoop;
    masterEventLoop = nullptr;

free_socket:
    socket->close();
    delete socket;
    socket = nullptr;
    return false;
}