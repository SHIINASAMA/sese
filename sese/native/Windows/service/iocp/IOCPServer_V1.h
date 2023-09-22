#pragma once

#include <sese/net/Address.h>
#include <sese/security/SSLContext.h>
#include <sese/service/iocp/IOBuf.h>
#include <sese/thread/Thread.h>
#include <sese/io/ByteBuilder.h>

#include <atomic>

namespace sese::_windows::iocp {

    class IOCPServer_V1;

    struct Context_V1 {
        using IOBuf = sese::iocp::IOBuf;
        using Node = sese::iocp::IOBufNode;

        enum class Type {
            Read,
            Write
        };

        Context_V1();
        ~Context_V1();

        OVERLAPPED overlapped{};
        WSABUF wsabufRead{};
        WSABUF wsabufWrite{};
        Type type{Type::Read};
        SOCKET fd{INVALID_SOCKET};
        IOCPServer_V1 *self{};
        UINT timer{};
        void *ssl{nullptr};
        Node *readNode{};
        IOBuf recv{};
        io::ByteBuilder send{IOCP_WSABUF_SIZE, IOCP_WSABUF_SIZE};
        void *data{nullptr};
    };

    class IOCPServer_V1 {
    public:
        using Context = Context_V1;
        using DeleteContextCallback = std::function<void(Context *data)>;

        bool init();
        void shutdown();
        static void postRead(Context *ctx);
        static void postWrite(Context *ctx);
        static void setTimeout(Context *ctx, int64_t seconds);
        static void cancelTimeout(Context *ctx);
        static void onDeleteContext(Context *) {}
        virtual void onAcceptCompleted(Context *ctx){};
        virtual void onPreRead(Context *ctx) {}
        virtual void onReadCompleted(Context *ctx){};
        virtual void onWriteCompleted(Context *ctx){};
        virtual void onTimeout(Context *ctx){};
        virtual void onAlpnGet(Context *ctx, const uint8_t *in, uint32_t inLength){};
        int onAlpnSelect(
                const uint8_t **out, uint8_t *outLength,
                const uint8_t *in, uint32_t inLength
        );

    public:
        void setAddress(const net::IPAddress::Ptr &addr) { IOCPServer_V1::address = addr; }
        void setThreads(size_t numberOfThreads) { IOCPServer_V1::threads = numberOfThreads; }
        void setServCtx(const security::SSLContext::Ptr &ctx) { IOCPServer_V1::sslCtx = ctx; }
        void setServProtos(const std::string &protos) { IOCPServer_V1::servProtos = protos; }
        void setDeleteContextCallback(const DeleteContextCallback &callback) { IOCPServer_V1::deleteContextCallback = callback; }

        [[nodiscard]] const security::SSLContext::Ptr &getServCtx() const { return IOCPServer_V1::sslCtx; }
        [[nodiscard]] const DeleteContextCallback &getDeleteContextCallback() const { return IOCPServer_V1::deleteContextCallback; };

    protected:
        void acceptThreadProc();
        void eventThreadProc();
        static int alpnCallbackFunction(
                void *ssl,
                const uint8_t **out, uint8_t *outLength,
                const uint8_t *in, uint32_t inLength,
                IOCPServer_V1 *server
        );

        std::atomic_bool isShutdown{false};
        HANDLE iocpFd{INVALID_HANDLE_VALUE};
        SOCKET listenFd{INVALID_SOCKET};
        net::IPAddress::Ptr address{};
        Thread::Ptr acceptThread{};

        size_t threads{};
        std::vector<Thread::Ptr> eventThreadGroup{};
        DeleteContextCallback deleteContextCallback = onDeleteContext;
        security::SSLContext::Ptr sslCtx{};
        std::string servProtos{};
    };

}// namespace sese::_windows::iocp