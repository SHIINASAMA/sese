// Copyright 2024 libsese
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define SESE_C_LIKE_FORMAT

#include <sese/service/iocp/IOCPServer.h>
#include <sese/record/Marco.h>
#include <sese/util/Initializer.h>
#include <sese/util/Util.h>

class MyIOCPServer : public sese::iocp::IOCPServer {
public:
    MyIOCPServer() {
        setDeleteContextCallback(myDeleter);
    }

    void onAcceptCompleted(sese::iocp::Context *ctx) override {
        SESE_INFO("onAcceptCompleted %d", ctx->getFd());
        postRead(ctx);
        setTimeout(ctx, 10);
    }

    void onPreRead(sese::iocp::Context *ctx) override {
        cancelTimeout(ctx);
        SESE_INFO("onPreRead %d", ctx->getFd());
    }

    void onReadCompleted(sese::iocp::Context *ctx) override {
        SESE_INFO("onReadCompleted %d", ctx->getFd());
        sese::streamMove(ctx, ctx, IOCP_WSABUF_SIZE);
        postWrite(ctx);
    }

    void onWriteCompleted(sese::iocp::Context *ctx) override {
        SESE_INFO("onWriteCompleted %d", ctx->getFd());
        postRead(ctx);
        setTimeout(ctx, 10);
    }

    void onTimeout(sese::iocp::Context *ctx) override {
        SESE_INFO("onTimeout %d", ctx->getFd());
        postClose(ctx);
    }

    static void myDeleter(sese::iocp::Context *ctx) {
        SESE_INFO("onDeleteCallback %d", ctx->getFd());
    }
};

MyIOCPServer server;

int main(int argc, char **argv) {
    sese::initCore(argc, argv);
    auto address = sese::net::IPv4Address::any(8080);
    server.setThreads(2);
    server.setAddress(address);
    auto rt = server.init();
    if (!rt) {
        SESE_ERROR("server init failed!");
        return 0;
    }
    SESE_INFO("server listening on %d", address->getPort());
    getchar();
    server.shutdown();
    return 0;
}