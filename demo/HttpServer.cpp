#include <sese/service/http/HttpServer_V3.h>
#include <sese/security/SSLContextBuilder.h>
#include <sese/util/Initializer.h>
#include <sese/record/Marco.h>
#include <sese/thread/Locker.h>
#include <sese/Util.h>

using sese::net::http::Controller;
using sese::net::http::Request;
using sese::net::http::Response;

SESE_CTRL(MyController, std::mutex mutex; std::map<std::string, std::string> map; int timers = 0) {
    SESE_INFO("LOADING " __FUNCTION__);
    SESE_URL(set, RequestType::GET, "/set?name&id") {
        sese::Locker locker(mutex);
        auto name = req.getQueryArg("name");
        auto id = req.getQueryArg("id");
        map[id] = name;
    };
    SESE_URL(get, RequestType::GET, "/get?{id}") {
        sese::Locker locker(mutex);
        auto id = req.getQueryArg("id");
        auto iterator = map.find(id);
        std::string message;
        if (iterator == map.end()) {
            message = "cannot found name by id '" + id + "'\n";
        } else {
            message = "name = '" + iterator->second + "'\n";
        }
        resp.getBody().write(message.data(), message.length());
    };
    SESE_URL(timers, RequestType::GET, "/timers") {
        sese::Locker locker(mutex);
        auto message = "timers = '" + std::to_string(this->timers) + "'\n";
        resp.getBody().write(message.data(), message.length());
    };
}

int main(int argc, char **argv) {
    using sese::service::http::v3::HttpServer;

    sese::initCore(argc, argv);
    auto ssl = sese::security::SSLContextBuilder::SSL4Server();
    ssl->importCertFile(PROJECT_PATH "/test/Data/test-ca.crt");
    ssl->importPrivateKeyFile(PROJECT_PATH "/test/Data/test-key.pem");

    auto server = HttpServer();
    server.setKeepalive(60);
    server.setName("HttpServiceImpl_V3");
    server.regMountPoint("/www", PROJECT_PATH "/build/html");
    server.regController<MyController>();
    server.regService(sese::net::IPv4Address::localhost(443), ssl);

    server.startup();
    getchar();
    server.shutdown();
}