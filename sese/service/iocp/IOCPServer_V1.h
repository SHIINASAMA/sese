#pragma once

#include <sese/security/SSLContext.h>
#include <sese/service/TimerableService_V2.h>
#include <sese/service/BalanceLoader.h>
#include <sese/io/ByteBuilder.h>

#include <set>

namespace sese::iocp {

    class Context_V1;
    class IOCPServer_V1;
    class IOCPService_V1;

    class IOCPServer_V1 {
    public:
        using Context = Context_V1;
        using DeleteContextCallback = std::function<void(Context *data)>;

        bool init();
        void shutdown();
        void postRead(Context *ctx);
        void postWrite(Context *ctx);
        void setTimeout(Context *ctx, int64_t seconds);
        void cancelTimeout(Context *ctx);
        static void onDeleteContext(Context *) {}
        virtual void onAcceptCompleted(Context *ctx){};
        virtual void onPreRead(Context *ctx) {}
        virtual void onReadCompleted(Context *ctx){};
        virtual void onWriteCompleted(Context *ctx){};
        virtual void onAlpnGet(Context *ctx, const uint8_t *in, uint32_t inLength){};
        int onAlpnSelect(
                const uint8_t **out, uint8_t *outLength,
                const uint8_t *in, uint32_t inLength
        );

    public:
        void setAddress(const net::IPAddress::Ptr &addr) { balanceLoader.setAddress(addr); }
        void setThreads(size_t threads) { balanceLoader.setThreads(threads); }
        void setServCtx(const security::SSLContext::Ptr &ctx) { IOCPServer_V1::sslCtx = ctx; }
        void setServProtos(const std::string &protos) { IOCPServer_V1::servProtos = protos; }
        void setDeleteContextCallback(const DeleteContextCallback &callback) { IOCPServer_V1::deleteContextCallback = callback; }

        [[nodiscard]] const security::SSLContext::Ptr &getServCtx() const { return IOCPServer_V1::sslCtx; }
        [[nodiscard]] const DeleteContextCallback &getDeleteContextCallback() const { return IOCPServer_V1::deleteContextCallback; };

    public:
        void setAcceptTimeout(uint32_t seconds) { balanceLoader.setAcceptTimeout(seconds); }
        void setDispatchTimeout(uint32_t seconds) { balanceLoader.setDispatchTimeout(seconds); }

    protected:
        DeleteContextCallback deleteContextCallback = onDeleteContext;
        security::SSLContext::Ptr sslCtx{};
        std::string servProtos{};
        service::BalanceLoader balanceLoader;
    };

    class Context_V1 final : public io::InputStream, public  io::OutputStream {
        friend class IOCPServer_V1;
        friend class IOCPService_V1;
        IOCPServer_V1 *self{};
        IOCPService_V1 *client{};
        socket_t fd{0};
        void *ssl{};
        event::BaseEvent *event{};
        service::TimeoutEvent_V2 *timeoutEvent{};
        io::ByteBuilder send{8192, 8192};
        io::ByteBuilder recv{8192, 8192};
        void *data{};

    public:
        int64_t read(void *buffer, size_t length) override;
        int64_t write(const void *buffer, size_t length) override;

        [[nodiscard]] int32_t getFd() const { return (int32_t) Context_V1::fd; }
        [[nodiscard]] void *getData() const { return Context_V1::data; }
        void setData(void *pData) { Context_V1::data = pData; }
    };

    class IOCPService_V1 final : public service::TimerableService_V2 {
    public:
        using Context = Context_V1;

        explicit IOCPService_V1(IOCPServer_V1 *master);
        ~IOCPService_V1() override;

        void postRead(Context *ctx);
        void postWrite(Context *ctx);
        static void onAcceptCompleted(Context *ctx);
        static void onPreRead(Context *ctx);
        static void onReadCompleted(Context *ctx);
        static void onWriteCompleted(Context *ctx);
        static void onTimeout(Context *ctx);
        static void onAlpnGet(Context *ctx, const uint8_t *in, uint32_t inLength);
        static int alpnCallbackFunction(
                void *ssl,
                const uint8_t **out, uint8_t *outLength,
                const uint8_t *in, uint32_t inLength,
                IOCPService_V1 *service
        );

        void onAccept(int fd) override;
        void onRead(event::BaseEvent *event) override;
        void onWrite(event::BaseEvent *event) override;
        void onTimeout(service::TimeoutEvent_V2 *event) override;

        event::BaseEvent *createEventEx(int fd, unsigned int events, void *data);
        void freeEventEx(sese::event::BaseEvent *event);

    private:
        static int64_t read(int fd, void *buffer, size_t len, void *ssl);
        static int64_t write(int fd, const void *buffer, size_t len, void *ssl);

        IOCPServer_V1 *master{};
        std::set<event::BaseEvent *> eventSet;
    };

}// namespace sese::iocp