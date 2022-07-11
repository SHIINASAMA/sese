#include <sese/net/http/HttpServer.h>
#include <sese/record/LogHelper.h>
#include <sese/Test.h>
#include <sese/Util.h>

using sese::Thread;
using sese::IPv4Address;
using sese::LogHelper;
using sese::http::HttpServer;
using sese::http::HttpServiceContext;

LogHelper helper("HTTPD");//NOLINT

int main() {
    auto address = IPv4Address::create("0.0.0.0", 8080);
    auto server = HttpServer::create(address, 2);
    assert(helper, server != nullptr, -1);

    Thread serverThread([&server](){
        server->loopWith([](const HttpServiceContext::Ptr& context) {
            decltype(auto) request = context->getRequest();
            auto url = request->getUrl();
            helper.info("request url: %s", url.c_str());

            decltype(auto) response = context->getResponse();
            response->setCode(200);
            response->set("Content-length", "17");
            context->flush();
            context->write("Hello Sese Httpd!", 17);
            return;
        });
    });

    serverThread.start();
    sese::sleep(10);
    server->shutdown();
    serverThread.join();

    return 0;
}