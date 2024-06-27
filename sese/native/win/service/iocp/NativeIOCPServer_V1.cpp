#define SESE_C_LIKE_FORMAT

#include <sese/native/win/service/iocp/NativeIOCPServer_V1.h>
#include <sese/record/Marco.h>

#include <openssl/ssl.h>

#include <mswsock.h>

using namespace sese::net;
using namespace sese::iocp;
using namespace sese::_windows::iocp::v1;

NativeContext::NativeContext(OverlappedWrapper *p_wrapper) : pWrapper(p_wrapper) {
    wsabufWrite.buf = static_cast<CHAR *>(malloc(IOCP_WSABUF_SIZE));
}

NativeContext::~NativeContext() {
    free(wsabufWrite.buf);
}

int64_t NativeContext::read(void *buffer, size_t length) {
    if (ssl) {
        return SSL_read(static_cast<SSL *>(ssl), buffer, static_cast<int>(length));
    } else {
        return recv.read(buffer, length);
    }
}

int64_t NativeContext::write(const void *buffer, size_t length) {
    if (ssl) {
        return SSL_write(static_cast<SSL *>(ssl), buffer, static_cast<int>(length));
    } else {
        return send.write(buffer, length);
    }
}

int64_t NativeContext::peek(void *buffer, size_t length) {
    if (ssl) {
        return SSL_peek(static_cast<SSL *>(ssl), buffer, static_cast<int>(length));
    } else {
        return recv.peek(buffer, length);
    }
}

int64_t NativeContext::trunc(size_t length) {
    if (ssl) {
        char buffer[1024]{};
        int64_t real = 0;
        while (true) {
            const auto NEED = std::min<int>(static_cast<int>(length - real), sizeof(buffer));
            if (const int L = SSL_read(static_cast<SSL *>(ssl), buffer, NEED); L > 0) {
                real += L;
            } else {
                break;
            }
            if (real == length) {
                break;
            }
        }
        return real;
    } else {
        return recv.trunc(length);
    }
}

OverlappedWrapper::OverlappedWrapper() : ctx(this) {
}

bool NativeIOCPServer::init() {
    if (!initConnectEx()) {
        return false;
    }

    if (address) {
        listenFd = WSASocketW(
                address->getFamily(),
                SOCK_STREAM,
                0,
                nullptr,
                0,
                WSA_FLAG_OVERLAPPED
        );
        if (listenFd == INVALID_SOCKET) {
            return false;
        }
        if (SOCKET_ERROR == Socket::setNonblocking(listenFd)) {
            return false;
        }
        if (SOCKET_ERROR == Socket::bind(listenFd, address->getRawAddress(), address->getRawAddressLength())) {
            return false;
        }
        if (SOCKET_ERROR == Socket::listen(listenFd, SOMAXCONN)) {
            return false;
        }
    }

    iocpFd = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, static_cast<DWORD>(threads));
    if (iocpFd == INVALID_HANDLE_VALUE) {
        if (address) {
            Socket::close(listenFd);
            listenFd = INVALID_SOCKET;
        }
        return false;
    }

    for (int i = 0; i < threads; ++i) {
        auto th = std::make_unique<Thread>([this] { eventThreadProc(); }, "IOCP_" + std::to_string(i + 1));
        th->start();
        eventThreadGroup.emplace_back(std::move(th));
    }

    acceptThread = std::make_unique<Thread>([this] { acceptThreadProc(); }, "IOCP_0");
    acceptThread->start();

    if (sslCtx) {
        const auto SERVER_SSL = static_cast<SSL_CTX *>(sslCtx->getContext());
        SSL_CTX_set_alpn_select_cb(
                SERVER_SSL,
                reinterpret_cast<SSL_CTX_alpn_select_cb_func>(&alpnCallbackFunction),
                this
        );
    }

    const auto METHOD = BIO_meth_new(BIO_get_new_index() | BIO_TYPE_SOURCE_SINK, "bioIOCP");
    BIO_meth_set_ctrl(METHOD, reinterpret_cast<long (*)(BIO *, int, long, void *)>(&NativeIOCPServer::bioCtrl));
    BIO_meth_set_read(METHOD, reinterpret_cast<int (*)(BIO *, char *, int)>(&NativeIOCPServer::bioRead));
    BIO_meth_set_write(METHOD, reinterpret_cast<int (*)(BIO *, const char *, int)>(&NativeIOCPServer::bioWrite));
    this->bioMethod = METHOD;

    return true;
}

void NativeIOCPServer::shutdown() {
    void *lp_completion_key = nullptr;
    isShutdown = true;
    for (int i = 0; i < threads; ++i) {
        PostQueuedCompletionStatus(iocpFd, -1, reinterpret_cast<ULONG_PTR>(lp_completion_key), nullptr);
    }
    for (auto &&thread: eventThreadGroup) {
        thread->join();
    }
    eventThreadGroup.clear();
    acceptThread->join();
    acceptThread = nullptr;

    for (auto &&item: wrapperSet) {
        deleteContextCallback(&item->ctx);
        Socket::close(item->ctx.fd);
        delete item;
    }
    wrapperSet.clear();

    if (bioMethod) {
        BIO_meth_free(static_cast<BIO_METHOD *>(bioMethod));
        bioMethod = nullptr;
    }
}

void NativeIOCPServer::postRead(NativeIOCPServer::Context *ctx) {
    ctx->type = NativeContext::Type::READ;
    ctx->readNode = std::make_unique<IOBufNode>(IOCP_WSABUF_SIZE);
    ctx->wsabufRead.buf = static_cast<CHAR *>(ctx->readNode->buffer);
    ctx->wsabufRead.len = static_cast<ULONG>(ctx->readNode->CAPACITY);
    DWORD n_bytes, dw_flags = 0;
    int n_rt = WSARecv(
            ctx->fd,
            &ctx->wsabufRead,
            1,
            &n_bytes,
            &dw_flags,
            &(ctx->pWrapper->overlapped),
            nullptr
    );
    auto e = getNetworkError();
    if (n_rt == SOCKET_ERROR && e != ERROR_IO_PENDING) {
        releaseContext(ctx);
    }
}

void NativeIOCPServer::postWrite(NativeIOCPServer::Context *ctx) {
    auto len = ctx->send.peek(ctx->wsabufWrite.buf, IOCP_WSABUF_SIZE);
    if (len == 0) {
        return;
    }
    ctx->type = NativeContext::Type::WRITE;
    ctx->wsabufWrite.len = static_cast<ULONG>(len);
    DWORD n_bytes, dw_flags = 0;
    int n_rt = WSASend(
            ctx->fd,
            &ctx->wsabufWrite,
            1,
            &n_bytes,
            dw_flags,
            &(ctx->pWrapper->overlapped),
            nullptr
    );
    auto e = getNetworkError();
    if (n_rt == SOCKET_ERROR && e != ERROR_IO_PENDING) {
        releaseContext(ctx);
    }
}

void NativeIOCPServer::postClose(NativeIOCPServer::Context *ctx) {
    if (activeReleaseMode) {
        void *lp_completion_key = nullptr;
        ctx->type = Context::Type::CLOSE;
        PostQueuedCompletionStatus(iocpFd, 0, reinterpret_cast<ULONG_PTR>(lp_completion_key), reinterpret_cast<LPOVERLAPPED>(ctx->pWrapper));
    } else {
        releaseContext(ctx);
    }
}

#define ConnectEx ((LPFN_CONNECTEX) connectEx)

void NativeIOCPServer::postConnect(const net::IPAddress::Ptr &to, const security::SSLContext::Ptr &cli_ctx, void *data) {
    auto sock = sese::net::Socket::socket(to->getFamily(), SOCK_STREAM, IPPROTO_IP);
    if (to->getFamily() == AF_INET) {
        const auto FROM = sese::net::IPv4Address::any();
        sese::net::Socket::bind(sock, FROM->getRawAddress(), FROM->getRawAddressLength());
    } else {
        const auto FROM = sese::net::IPv6Address::any();
        sese::net::Socket::bind(sock, FROM->getRawAddress(), FROM->getRawAddressLength());
    }

    sese::net::Socket::setNonblocking(sock);

    auto p_wrapper = new OverlappedWrapper();
    p_wrapper->ctx.fd = sock;
    p_wrapper->ctx.self = this;
    p_wrapper->ctx.type = NativeContext::Type::CONNECT;
    p_wrapper->ctx.data = data;
    if (cli_ctx) {
        p_wrapper->ctx.ssl = SSL_new(static_cast<SSL_CTX *>(cli_ctx->getContext()));
        SSL_set_fd(static_cast<SSL *>(p_wrapper->ctx.ssl), static_cast<int>(sock));
        SSL_set_alpn_protos(static_cast<SSL *>(p_wrapper->ctx.ssl), reinterpret_cast<const unsigned char *>(clientProtos.c_str()), static_cast<unsigned>(clientProtos.length()));
    }
    auto addr = to->getRawAddress();
    auto len = to->getRawAddressLength();

    CreateIoCompletionPort(reinterpret_cast<HANDLE>(p_wrapper->ctx.fd), iocpFd, 0, 0);
    BOOL n_rt = ConnectEx(sock, addr, len, nullptr, 0, nullptr, reinterpret_cast<LPOVERLAPPED>(p_wrapper));
    auto e = getNetworkError();
    if (n_rt == FALSE && e != ERROR_IO_PENDING) {
        releaseContext(&p_wrapper->ctx);
    } else {
        wrapperSetMutex.lock();
        wrapperSet.emplace(p_wrapper);
        wrapperSetMutex.unlock();
        onPreConnect(&p_wrapper->ctx);
    }
}

#undef ConnectEx

void NativeIOCPServer::acceptThreadProc() {
    using namespace std::chrono_literals;

    while (!isShutdown) {

        if (listenFd != INVALID_SOCKET) {
            auto client_socket = accept(listenFd, nullptr, nullptr);
            if (client_socket == INVALID_SOCKET) {
                // std::this_thread::sleep_for(500ms);
                wrapperSetMutex.lock();
                wheel.check();
                wrapperSetMutex.unlock();
                continue;
            }

            if (SOCKET_ERROR == Socket::setNonblocking(client_socket)) {
                Socket::close(client_socket);
                // std::this_thread::sleep_for(500ms);
                wrapperSetMutex.lock();
                wheel.check();
                wrapperSetMutex.unlock();
                continue;
            }

            auto p_wrapper = new OverlappedWrapper;
            p_wrapper->ctx.fd = client_socket;
            p_wrapper->ctx.self = this;

            if (sslCtx) {
                auto server_ssl = static_cast<SSL_CTX *>(sslCtx->getContext());
                auto client_ssl = SSL_new(server_ssl);
                SSL_set_fd(client_ssl, static_cast<int>(client_socket));
                SSL_set_accept_state(client_ssl);

                while (true) {
                    auto rt = SSL_do_handshake(client_ssl);
                    if (rt <= 0) {
                        auto err = SSL_get_error(client_ssl, rt);
                        if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE) {
                            SSL_free(client_ssl);
                            Socket::close(client_socket);
                            delete p_wrapper;
                            wrapperSetMutex.lock();
                            wheel.check();
                            wrapperSetMutex.unlock();
                            return;
                        }
                    } else {
                        p_wrapper->ctx.ssl = client_ssl;
                        SSL_set_mode(client_ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
                        const uint8_t *data = nullptr;
                        uint32_t data_length;
                        SSL_get0_alpn_selected(client_ssl, &data, &data_length);
                        onAlpnGet(&p_wrapper->ctx, data, data_length);
                        p_wrapper->ctx.bio = BIO_new(static_cast<BIO_METHOD *>(bioMethod));
                        BIO_set_data(static_cast<BIO *>(p_wrapper->ctx.bio), &p_wrapper->ctx);
                        BIO_set_init(static_cast<BIO *>(p_wrapper->ctx.bio), 1);
                        BIO_set_shutdown(static_cast<BIO *>(p_wrapper->ctx.bio), 0);
                        SSL_set_bio(static_cast<SSL *>(p_wrapper->ctx.ssl), static_cast<BIO *>(p_wrapper->ctx.bio), static_cast<BIO *>(p_wrapper->ctx.bio));
                        break;
                    }
                }
            }

            CreateIoCompletionPort(reinterpret_cast<HANDLE>(p_wrapper->ctx.fd), iocpFd, 0, 0);

            wrapperSetMutex.lock();
            wrapperSet.emplace(p_wrapper);
            wheel.check();
            wrapperSetMutex.unlock();

            onAcceptCompleted(&p_wrapper->ctx);
        } else {
            std::this_thread::sleep_for(500ms);
            wrapperSetMutex.lock();
            wheel.check();
            wrapperSetMutex.unlock();
        }
    }
}

void NativeIOCPServer::eventThreadProc() {
    OverlappedWrapper *p_wrapper{};
    DWORD lp_number_of_bytes_transferred = 0;
    void *lp_completion_key = nullptr;

    while (!isShutdown) {
        GetQueuedCompletionStatus(
                iocpFd,
                &lp_number_of_bytes_transferred,
                reinterpret_cast<PULONG_PTR>(&lp_completion_key),
                reinterpret_cast<LPOVERLAPPED *>(&p_wrapper),
                INFINITE
        );
        if (p_wrapper == nullptr) {
            continue;
        } else if (lp_number_of_bytes_transferred == 0 && p_wrapper->ctx.type != NativeContext::Type::CONNECT) {
            // 主动释放模式对端关闭
            // 任何模式下的非主动关闭
            if (activeReleaseMode || p_wrapper->ctx.type != NativeContext::Type::CLOSE) {
                releaseContext(&p_wrapper->ctx);
            }
            continue;
        } else if (lp_number_of_bytes_transferred == -1) {
            break;
        }

        if (p_wrapper->ctx.type == NativeContext::Type::READ) {
            onPreRead(&p_wrapper->ctx);
            p_wrapper->ctx.readNode->size = lp_number_of_bytes_transferred;
            p_wrapper->ctx.recv.push(std::move(p_wrapper->ctx.readNode));
            p_wrapper->ctx.readNode = nullptr;
            p_wrapper->ctx.wsabufRead.buf = nullptr;
            p_wrapper->ctx.wsabufRead.len = 0;
            if (lp_number_of_bytes_transferred == IOCP_WSABUF_SIZE) {
                postRead(&p_wrapper->ctx);
            } else {
                onReadCompleted(&p_wrapper->ctx);
            }
        } else if (p_wrapper->ctx.type == NativeContext::Type::WRITE) {
            p_wrapper->ctx.send.trunc(lp_number_of_bytes_transferred);
            auto len = p_wrapper->ctx.send.peek(p_wrapper->ctx.wsabufWrite.buf, IOCP_WSABUF_SIZE);
            if (len == 0) {
                onWriteCompleted(&p_wrapper->ctx);
            } else {
                p_wrapper->ctx.type = NativeContext::Type::WRITE;
                p_wrapper->ctx.wsabufWrite.len = static_cast<ULONG>(len);
                DWORD n_bytes, dw_flags = 0;
                int n_rt = WSASend(
                        p_wrapper->ctx.fd,
                        &p_wrapper->ctx.wsabufWrite,
                        1,
                        &n_bytes,
                        dw_flags,
                        &(p_wrapper->overlapped),
                        nullptr
                );
                auto e = getNetworkError();
                if (n_rt == SOCKET_ERROR && e != ERROR_IO_PENDING) {
                    releaseContext(&p_wrapper->ctx);
                }
            }
        } else {
            auto connect_status = GetOverlappedResult(reinterpret_cast<HANDLE>(p_wrapper->ctx.fd), reinterpret_cast<LPOVERLAPPED>(p_wrapper), &lp_number_of_bytes_transferred, TRUE);
            if (connect_status == FALSE) {
                releaseContext(&p_wrapper->ctx);
                continue;
            }

            auto ssl = (SSL *) p_wrapper->ctx.ssl;
            if (ssl) {
                SSL_set_connect_state(ssl);
                // GCOVR_EXCL_START
                while (true) {
                    auto rt = SSL_do_handshake((SSL *) ssl);
                    if (rt <= 0) {
                        // err is SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE
                        auto err = SSL_get_error((SSL *) ssl, rt);
                        if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE) {
                            SSL_free((SSL *) ssl);
                            p_wrapper->ctx.ssl = nullptr;
                            ssl = nullptr;
                            break;
                        }
                    } else {
                        break;
                    }
                }
                // GCOVR_EXCL_STOP
                if (ssl == nullptr) {
                    releaseContext(&p_wrapper->ctx);
                    continue;
                } else {
                    SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
                    const uint8_t *data = nullptr;
                    uint32_t data_length;
                    SSL_get0_alpn_selected(ssl, &data, &data_length);
                    onAlpnGet(&p_wrapper->ctx, data, data_length);
                    p_wrapper->ctx.bio = BIO_new(static_cast<BIO_METHOD *>(bioMethod));
                    BIO_set_data(static_cast<BIO *>(p_wrapper->ctx.bio), &p_wrapper->ctx);
                    BIO_set_init(static_cast<BIO *>(p_wrapper->ctx.bio), 1);
                    BIO_set_shutdown(static_cast<BIO *>(p_wrapper->ctx.bio), 0);
                    SSL_set_bio(static_cast<SSL *>(p_wrapper->ctx.ssl), static_cast<BIO *>(p_wrapper->ctx.bio), static_cast<BIO *>(p_wrapper->ctx.bio));
                }
            }
            p_wrapper->ctx.type = Context::Type::READY;
            p_wrapper->ctx.self->onConnected(&p_wrapper->ctx);
        }
    }
}

int NativeIOCPServer::onAlpnSelect(const uint8_t **out, uint8_t *out_length, const uint8_t *in, uint32_t in_length) {
    if (SSL_select_next_proto(const_cast<unsigned char **>(out), out_length, reinterpret_cast<const uint8_t *>(servProtos.c_str()), static_cast<int>(servProtos.length()), in, in_length) != OPENSSL_NPN_NEGOTIATED) {
        return SSL_TLSEXT_ERR_NOACK;
    }
    return SSL_TLSEXT_ERR_OK;
}

int NativeIOCPServer::alpnCallbackFunction([[maybe_unused]] void *ssl, const uint8_t **out, uint8_t *out_length, const uint8_t *in, uint32_t in_length, NativeIOCPServer *server) {
    return server->onAlpnSelect(out, out_length, in, in_length);
}

bool NativeIOCPServer::initConnectEx() {
    auto sock = ::socket(AF_INET, SOCK_STREAM, 0);
    DWORD dw_bytes;
    GUID guid = WSAID_CONNECTEX;
    auto rc = WSAIoctl(
            sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid, sizeof(guid),
            &connectEx, sizeof(connectEx),
            &dw_bytes, nullptr, nullptr
    );
    Socket::close(sock);
    return rc == 0;
}

long NativeIOCPServer::bioCtrl([[maybe_unused]] void *bio, int cmd, [[maybe_unused]] long num, [[maybe_unused]] void *ptr) {
    int ret = 0;
    if (cmd == BIO_CTRL_FLUSH) {
        ret = 1;
    }
    return ret;
}

int NativeIOCPServer::bioWrite(void *bio, const char *in, int length) {
    auto ctx = static_cast<NativeContext *>(BIO_get_data(static_cast<BIO *>(bio)));
    return static_cast<int>(ctx->send.write(in, length));
}

int NativeIOCPServer::bioRead(void *bio, char *out, int length) {
    auto ctx = static_cast<NativeContext *>(BIO_get_data(static_cast<BIO *>(bio)));
    return static_cast<int>(ctx->recv.read(out, length));
}

void NativeIOCPServer::setTimeout(NativeIOCPServer::Context *ctx, int64_t seconds) {
    wrapperSetMutex.lock();
    ctx->timeoutEvent = wheel.delay(
            [this, ctx]() {
                // auto pWrapper = ctx->pWrapper;
                // Socket::close(pWrapper->ctx.fd);
                // pWrapper->ctx.self->getDeleteContextCallback()(&pWrapper->ctx);
                // wrapperSet.erase(pWrapper);
                // delete ctx->pWrapper;
                ctx->timeoutEvent = nullptr;
                this->onTimeout(ctx);
            },
            seconds, false
    );
    wrapperSetMutex.unlock();
}

void NativeIOCPServer::cancelTimeout(NativeIOCPServer::Context *ctx) {
    if (ctx->timeoutEvent) {
        wrapperSetMutex.lock();
        wheel.cancel(ctx->timeoutEvent);
        ctx->timeoutEvent = nullptr;
        wrapperSetMutex.unlock();
    }
}

void NativeIOCPServer::releaseContext(Context *ctx) {
    wrapperSetMutex.lock();
    auto p_wrapper = ctx->pWrapper;
    wrapperSet.erase(p_wrapper);
    if (p_wrapper->ctx.timeoutEvent) {
        wheel.cancel(p_wrapper->ctx.timeoutEvent);
        p_wrapper->ctx.timeoutEvent = nullptr;
    }
    wrapperSetMutex.unlock();
    Socket::close(p_wrapper->ctx.fd);
    if (p_wrapper->ctx.ssl) {
        SSL_free(static_cast<SSL *>(p_wrapper->ctx.ssl));
        p_wrapper->ctx.ssl = nullptr;
    }
    p_wrapper->ctx.self->getDeleteContextCallback()(&p_wrapper->ctx);
    delete p_wrapper;
}
