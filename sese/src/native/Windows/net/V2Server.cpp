#include "sese/net/V2Server.h"
#include "openssl/ssl.h"

using namespace sese::net::v2;

WindowsServiceIOContext::WindowsServiceIOContext(socket_t socket, HANDLE handle, void *ssl) noexcept
    : socket(socket),
      event(handle),
      ssl(ssl) {
}

int64_t WindowsServiceIOContext::peek(void *buf, size_t len) noexcept {
    if (ssl) {
        return ::SSL_peek((ssl_st *) ssl, (char *) buf, (int) len);
    } else {
        return ::recv(socket, (char *) buf, (int) len, MSG_PEEK);
    }
}

int64_t WindowsServiceIOContext::read(void *buf, size_t len) noexcept {
    if (ssl) {
        return ::SSL_read((ssl_st *) ssl, (char *) buf, (int) len);
    } else {
        return ::recv(socket, (char *) buf, (int) len, 0);
    }
}

int64_t WindowsServiceIOContext::write(const void *buf, size_t len) noexcept {
    if (ssl) {
        return ::SSL_write((ssl_st *) ssl, (const char *) buf, (int) len);
    } else {
        return ::send(socket, (const char *) buf, (int) len, 0);
    }
}

void WindowsServiceIOContext::close() noexcept {
    if (ssl) {
        ::SSL_shutdown((ssl_st *) ssl);
    } else {
        ::shutdown(socket, SD_BOTH);
    }
    isClosing = true;
}

//WindowsService::Ptr sese::net::v2::WindowsService::create(ServerOption *opt) noexcept {
//    if (opt == nullptr) {
//        return nullptr;
//    }
//
//    if (opt->isSSL && opt->sslContext) {
//        if (!opt->sslContext->authPrivateKey()) {
//            return nullptr;
//        }
//    }
//
//    auto family = opt->address->getRawAddress()->sa_family;
//    socket_t listenSocket = ::socket(family, SOCK_STREAM, 0);
//    if (SOCKET_ERROR == listenSocket) {
//        return nullptr;
//    }
//
//    unsigned long ul = 1;
//    if (SOCKET_ERROR == ioctlsocket(listenSocket, FIONBIO, &ul)) {
//        ::closesocket(listenSocket);
//        return nullptr;
//    }
//
//    if (SOCKET_ERROR == ::bind(listenSocket, opt->address->getRawAddress(), opt->address->getRawAddressLength())) {
//        ::closesocket(listenSocket);
//        return nullptr;
//    }
//
//    if (SOCKET_ERROR == ::listen(listenSocket, SERVER_MAX_CONNECTION)) {
//        ::closesocket(listenSocket);
//        return nullptr;
//    }
//
//    WSAEVENT event = ::WSACreateEvent();
//    if (WSA_INVALID_EVENT == event) {
//        ::WSACloseEvent(event);
//        return nullptr;
//    }
//
//    if (::WSAEventSelect(listenSocket, event, FD_ACCEPT)) {
//        ::WSACloseEvent(event);
//        ::closesocket(listenSocket);
//        return nullptr;
//    }
//
//    auto serv = new WindowsService;
//    serv->option = opt;
//    serv->eventNum = 1;
//    serv->socketSet[0] = listenSocket;
//    serv->hEventSet[0] = event;
//
//    return std::unique_ptr<WindowsService>(serv);
//}

WindowsService::~WindowsService() noexcept {
    if (initStatus && !exitStatus && mainThread) {
        shutdown();
    }
}

bool WindowsService::init() noexcept {
    if (sslContext && !sslContext->authPrivateKey()) {
        return false;
    }

    if (!address) {
        address = IPv4Address::localhost();
        address->setPort(8080);
    }

    auto family = address->getRawAddress()->sa_family;
    socket_t listenSocket = ::socket(family, SOCK_STREAM, 0);
    if (SOCKET_ERROR == listenSocket) {
        return false;
    }
    unsigned long ul = 1;
    if (SOCKET_ERROR == ioctlsocket(listenSocket, FIONBIO, &ul)) {
        ::closesocket(listenSocket);
        return false;
    }
    if (SOCKET_ERROR == ::bind(listenSocket, address->getRawAddress(), address->getRawAddressLength())) {
        ::closesocket(listenSocket);
        return false;
    }
    if (SOCKET_ERROR == ::listen(listenSocket, SERVER_MAX_CONNECTION)) {
        ::closesocket(listenSocket);
        return false;
    }
    WSAEVENT event = ::WSACreateEvent();
    if (WSA_INVALID_EVENT == event) {
        ::WSACloseEvent(event);
        return false;
    }
    if (::WSAEventSelect(listenSocket, event, FD_ACCEPT)) {
        ::WSACloseEvent(event);
        ::closesocket(listenSocket);
        return false;
    }

    this->eventNum = 1;
    this->socketSet[0] = listenSocket;
    this->hEventSet[0] = event;

    initStatus = true;
    return true;
}

void WindowsService::loop() noexcept {
    while (true) {
        if (exitStatus) break;

        DWORD nIndex = ::WSAWaitForMultipleEvents(eventNum, hEventSet, FALSE, 1000, FALSE);
        if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT) {
            continue;
        }

        nIndex -= WSA_WAIT_EVENT_0;
        for (DWORD i = nIndex; i < eventNum; ++i) {
            nIndex = ::WSAWaitForMultipleEvents(1, &hEventSet[i], TRUE, 1000, FALSE);
            if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT) {
                continue;
            }

            WSANETWORKEVENTS enumEvent;
            ::WSAEnumNetworkEvents(socketSet[i], hEventSet[i], &enumEvent);
            if (enumEvent.lNetworkEvents & FD_ACCEPT) {
                if (enumEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
                    continue;
                }
                if (eventNum > MaxEventSize) {
                    continue;
                }
                SOCKET clientSocket = ::accept(socketSet[i], NULL, NULL);//NOLINT

                unsigned long ul = 1;
                if (SOCKET_ERROR == ioctlsocket(clientSocket, FIONBIO, &ul)) {
                    ::closesocket(clientSocket);
                    continue;
                }

                if (sslContext) {
                    auto *clientSSL = (ssl_st *) handshake(clientSocket);
                    if (!clientSSL) {
                        ::closesocket(clientSocket);
                        continue;
                    }
                    sslSet[eventNum] = clientSSL;
                    connect({clientSocket, INVALID_HANDLE_VALUE, clientSSL});
                } else {
                    connect({clientSocket, INVALID_HANDLE_VALUE, nullptr});
                }


                WSAEVENT clientEvent = ::WSACreateEvent();
                if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_CLOSE)) {
                    ::WSACloseEvent(clientEvent);
                    ::closesocket(clientSocket);
                }
                socketSet[eventNum] = clientSocket;
                hEventSet[eventNum] = clientEvent;
                eventNum += 1;
            } else if (enumEvent.lNetworkEvents & FD_READ) {
                if (enumEvent.iErrorCode[FD_READ_BIT] != 0) {
                    if (sslSet[i]) {
                        SSL_shutdown((ssl_st *) sslSet[i]);
                    } else {
                        ::shutdown(socketSet[i], SD_BOTH);
                    }
                    continue;
                }

                /// WSAEventSelect 在对端调用 SSL_shutdown 之后依然会收到一次可读事件
                if (sslContext) {
                    char buf;
                    auto rt = SSL_peek((ssl_st *) sslSet[i], &buf, 1);
                    if (rt <= 0) {
                        continue;
                    }
                }
                WSAEventSelect(socketSet[i], hEventSet[i], FD_CLOSE);
                handle({socketSet[i], hEventSet[i], sslSet[i]});
            } else if (enumEvent.lNetworkEvents & FD_CLOSE) {
                closing({socketSet[i], nullptr, nullptr});
                // 关闭套接字，并将其从 socket数组 和 事件数组 中移除
                if (sslContext) {
                    SSL_free((SSL *) sslSet[i]);
                    memmove(&sslSet[i], &sslSet[i + 1], (eventNum - i - 1) * sizeof(PVOID));
                }
                closesocket(socketSet[i]);
                WSACloseEvent(hEventSet[i]);
                memmove(&socketSet[i], &socketSet[i + 1], (eventNum - i - 1) * sizeof(SOCKET));
                memmove(&hEventSet[i], &hEventSet[i + 1], (eventNum - i - 1) * sizeof(HANDLE));
                eventNum -= 1;
            }
        }
    }
}

void *WindowsService::handshake(SOCKET client) noexcept {
    ssl_st *clientSSL = nullptr;
    clientSSL = SSL_new((SSL_CTX *) sslContext->getContext());
    SSL_set_fd(clientSSL, (int) client);
    SSL_set_accept_state(clientSSL);

    while (true) {
        auto rt = SSL_do_handshake(clientSSL);
        if (rt <= 0) {
            // err is SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE
            auto err = SSL_get_error(clientSSL, rt);
            if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE) {
                SSL_free(clientSSL);
                closesocket(client);
                return nullptr;
            }
        } else {
            break;
        }
    }

    return clientSSL;
}

void WindowsService::connect(IOContext ctx) noexcept {
    threadPool->postTask([ctx, this]() {
        auto myCtx = ctx;
        onConnect(myCtx);
    });
}

void WindowsService::handle(IOContext ctx) noexcept {
    threadPool->postTask([ctx, this]() {
        auto myCtx = ctx;
        onHandle(myCtx);
        if (!myCtx.isClosing) {
            // WSAResetEvent(myCtx.event);
            WSAEventSelect(myCtx.socket, myCtx.event, FD_READ | FD_CLOSE);
        }
    });
}

void WindowsService::closing(IOContext ctx) noexcept {
    threadPool->postTask([ctx, this]() {
        auto myCtx = ctx;
        onClosing(myCtx);
    });
}

void WindowsService::start() noexcept {
    mainThread = std::make_unique<Thread>([this]() { loop(); }, "WIN_MAIN");
    threadPool = std::make_unique<ThreadPool>("WIN_SERV", threads);
    mainThread->start();
}

void WindowsService::shutdown() noexcept {
    if (mainThread != nullptr && mainThread->joinable()) {
        exitStatus = true;
        mainThread->join();
        threadPool->shutdown();
    }
}