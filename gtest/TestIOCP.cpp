#include <gtest/gtest.h>

#include <sese/service/iocp/IOCPServer.h>
#include <sese/record/Marco.h>
#include <sese/io/OutputUtil.h>
#include <sese/util/Util.h>

class MyIOCPServer : public sese::iocp::IOCPServer {
public:
    MyIOCPServer() {
        setDeleteContextCallback(myDeleter);
    }

    void onAcceptCompleted(Context *ctx) override {
        SESE_INFO("onAcceptCompleted %d", ctx->getFd());
        postRead(ctx);
    }

    void onPreRead(Context *ctx) override {
        SESE_INFO("onRreRead %d", ctx->getFd());
    }

    void onReadCompleted(Context *ctx) override {
        SESE_INFO("onReadCompleted %d", ctx->getFd());
        sese::streamMove(ctx, ctx, IOCP_WSABUF_SIZE);
        postWrite(ctx);
    }

    void onWriteCompleted(Context *ctx) override {
        SESE_INFO("onWriteCompleted %d", ctx->getFd());
    }

    static void myDeleter(Context *ctx) {
        SESE_INFO("onDeleteCallback %d", ctx->getFd());
    }
};

TEST(TestIOCP, Server) {
    auto address = sese::net::IPv4Address::localhost(sese::net::createRandomPort());

    MyIOCPServer server;
    server.setAddress(address);
    server.setThreads(2);
    server.setServProtos("\x8http/1.1");
    ASSERT_TRUE(server.init());
    SESE_INFO("server listening on %d", address->getPort());

    sese::net::Socket socket(sese::net::Socket::Family::IPv4, sese::net::Socket::Type::TCP);
    ASSERT_EQ(socket.connect(address), 0);
    socket << "Hello World";

    char buffer[32]{};
    socket.read(buffer, sizeof(buffer));
    SESE_INFO("echo from server, %s", buffer);
    socket.close();

    server.shutdown();
}