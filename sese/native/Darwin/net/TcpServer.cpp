// #include <sese/net/TcpServer.h>
// #include "sese/util/Util.h"
// #include <unistd.h>
// #include <fcntl.h>

// using Server = sese::TcpServer;
// using sese::IOContext;

// int64_t sese::IOContext::read(void *buffer, size_t length) {
//     return ::read(socket, buffer, length);
// }

// int64_t sese::IOContext::write(const void *buffer, size_t length) {
//     return ::write(socket, buffer, length);
// }

// void sese::IOContext::close() {
//     isClosed = true;
//     shutdown(socket, SHUT_RDWR);
//     ::close(socket);
// }

// #define CLEAR      \
//     close(sockFd); \
//     return nullptr;

// Server::Ptr Server::create(const IPAddress::Ptr &ipAddress, size_t threads, size_t keepAlive) noexcept {
//     auto family = ipAddress->getRawAddress()->sa_family;
//     socket_t sockFd = socket(family, SOCK_STREAM, IPPROTO_TCP);
//     if (-1 == sockFd) {
//         return nullptr;
//     }

//     int32_t opt = fcntl(sockFd, F_GETFL);
//     if (-1 == opt) {
//         CLEAR
//     }

//     if (-1 == fcntl(sockFd, F_SETFL, opt | O_NONBLOCK)) {
//         CLEAR
//     }

//     if (-1 == bind(sockFd, ipAddress->getRawAddress(), ipAddress->getRawAddressLength())) {
//         CLEAR
//     }

//     if (-1 == listen(sockFd, SERVER_MAX_CONNECTION)) {
//         CLEAR
//     }

//     int32_t kqueueFd = kqueue();
//     if (-1 == kqueueFd) {
//         CLEAR
//     }

//     KEvent event{};
//     EV_SET(&event, sockFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
//     if (-1 == kevent(kqueueFd, &event, 1, nullptr, 0, nullptr)) {
//         close(sockFd);
//         close(kqueueFd);
//         return nullptr;
//     }

//     auto server = new Server;
//     server->sockFd = sockFd;
//     server->kqueueFd = kqueueFd;
//     server->threadPool = std::make_unique<ThreadPool>("TcpServer", threads);
//     if (keepAlive > 0) {
//         server->timer = Timer::create();
//     }
//     server->keepAlive = keepAlive;
//     return std::unique_ptr<Server>(server);
// }

// #undef CLEAR

// Server::Ptr Server::create(const Socket::Ptr &listenSocket, size_t threads, size_t keepAlive) noexcept {
//     int32_t kqueueFd = kqueue();
//     if (-1 == kqueueFd) {
//         return nullptr;
//     }

//     KEvent event{};
//     EV_SET(&event, listenSocket->getRawSocket(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
//     if (-1 == kevent(kqueueFd, &event, 1, nullptr, 0, nullptr)) {
//         close(kqueueFd);
//         return nullptr;
//     }

//     auto server = new Server;
//     server->sockFd = listenSocket->getRawSocket();
//     server->kqueueFd = kqueueFd;
//     server->threadPool = std::make_unique<ThreadPool>("TcpServer", threads);
//     if (keepAlive > 0) {
//         server->timer = Timer::create();
//     }
//     server->keepAlive = keepAlive;
//     return std::unique_ptr<Server>(server);
// }

// void Server::loopWith(const std::function<void(IOContext *)> &handler) noexcept {
//     struct timespec timeout {
//         1, 0
//     };

//     while (true) {
//         if (isShutdown) break;

//         int32_t nev = kevent(kqueueFd, nullptr, 0, events, MaxEvents, &timeout);
//         if (-1 == nev) continue;
//         for (int32_t n = 0; n < nev; n++) {
//             if (events[n].ident == sockFd) {
//                 // 新连接接入
//                 socket_t client = accept(sockFd, nullptr, nullptr);
//                 if (-1 == client) {
//                     close(client);
//                     continue;
//                 }

//                 int32_t opt = fcntl(client, F_GETFL);
//                 if (-1 == opt) {
//                     close(client);
//                     continue;
//                 }

//                 if (-1 == fcntl(client, F_SETFL, opt | O_NONBLOCK)) {
//                     close(client);
//                     continue;
//                 }

//                 KEvent event{};
//                 EV_SET(&event, client, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
//                 if (-1 == kevent(kqueueFd, &event, 1, nullptr, 0, nullptr)) {
//                     close(client);
//                     continue;
//                 }

//                 // 首次接入，开始计时
//                 if (0 != keepAlive) {
//                     mutex.lock();
//                     taskMap[client] = timer->delay(std::bind(&Server::closeCallback, this, client), (int64_t) keepAlive, false);
//                     mutex.unlock();
//                 }
//             } else if (events[n].filter == EVFILT_READ) {
//                 if (events[n].flags & EV_EOF) {
//                     continue;
//                 }

//                 socket_t client = events[n].ident;
//                 if (0 != keepAlive) {
//                     mutex.lock();
//                     auto iterator = taskMap.find(client);
//                     // iterator != taskMap.end()
//                     taskMap.erase(client);
//                     mutex.unlock();
//                     iterator->second->cancel();
//                 }

//                 threadPool->postTask([handler, client, this]() {
//                     IOContext ioContext;
//                     ioContext.socket = client;
//                     handler(&ioContext);

//                     if (ioContext.isClosed) {
//                         // 不需要保留连接，已主动关闭
//                     } else {
//                         if (0 == keepAlive) {
//                             // 此处不应默认关闭链接，应由开发人员决定
//                             // ::shutdown(client, SHUT_RDWR);
//                             // ::close(client);
//                         } else {
//                             // 需要保留连接，但需要做超时管理
//                             KEvent ev{};
//                             EV_SET(&ev, client, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
//                             kevent(kqueueFd, &ev, 1, nullptr, 0, nullptr);

//                             // 继续计时
//                             mutex.lock();
//                             taskMap[client] = timer->delay(std::bind(&Server::closeCallback, this, client), (int64_t) keepAlive, false);
//                             mutex.unlock();
//                         }
//                     }
//                 });
//             }
//         }
//     }
// }

// void Server::shutdown() noexcept {
//     isShutdown = true;
//     threadPool->shutdown();
//     if (keepAlive > 0) {
//         timer->shutdown();
//     }
//     for (auto &pair: taskMap) {
//         ::shutdown(pair.first, SHUT_RDWR);
//         close(pair.first);
//     }
// }

// void Server::closeCallback(socket_t socket) noexcept {
//     mutex.lock();
//     taskMap.erase(socket);
//     mutex.unlock();
//     ::shutdown(socket, SHUT_RDWR);
//     close(socket);
// }