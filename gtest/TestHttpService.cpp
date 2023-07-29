#include <sese/service/HttpService.h>
#include <sese/service/BalanceLoader.h>
#include <sese/security/SSLContextBuilder.h>
#include <sese/net/http/HttpUtil.h>
#include <sese/record/Marco.h>

#include <random>

#include <gtest/gtest.h>

sese::net::IPv4Address::Ptr createAddress() {
    std::random_device device;
    auto engine = std::default_random_engine(device());
    std::uniform_int_distribution<uint16_t> dis(1025, 65535);
    auto port = dis(engine);
    printf("select port %d\n", (int) port);
    return sese::net::IPv4Address::create("0.0.0.0", 8090);
}


TEST(TestHttpService, _0) {
    auto servCtx = sese::security::SSLContextBuilder::SSL4Server();
    servCtx->importCertFile(PROJECT_PATH "/gtest/Data/test-ca.crt");
    servCtx->importPrivateKeyFile(PROJECT_PATH "/gtest/Data/test-key.pem");
    ASSERT_TRUE(servCtx->authPrivateKey());

    auto addr = createAddress();

    sese::service::HttpConfig config;
    config.servName = "Server for Test";
    config.servCtx = servCtx;

    config.controller1Map["/index.html"] =
            [](sese::net::http::RequestHeader &req,
               sese::net::http::ResponseHeader &resp) -> bool {
        SESE_INFO("req GET: %s", req.getUrl().c_str());
        resp.setCode(404);
        return true;
    };

    config.controller2Map["/post"] =
            [](sese::net::http::RequestHeader &req,
               sese::net::http::ResponseHeader &resp,
               sese::ByteBuilder *buffer) -> bool {
        auto contentLength = req.get("Content-Length", "0");
        if (contentLength != "0") {
            char *end;
            auto len = std::strtol(contentLength.c_str(), &end, 0);
            char b[1024]{};
            buffer->read(b, len);
            SESE_INFO("req POST: %s\n%s", req.getUrl().c_str(), b);
        } else {
            SESE_INFO("req POST: %s", req.getUrl().c_str());
        }

        resp.setCode(200);
        sese::net::http::HttpUtil::sendResponse(buffer, &resp);
        return true;
    };

    sese::service::BalanceLoader service;
    service.setThreads(2);
    service.setAddress(addr);
    ASSERT_TRUE(service.init<sese::service::HttpService>([&config]() -> auto {
        return new sese::service::HttpService(config);
    }));

    service.start();
    ASSERT_TRUE(service.isStarted());

    getchar();

    service.stop();
}