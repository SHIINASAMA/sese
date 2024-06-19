#include <sese/internal/service/http/HttpServiceImpl_V3.h>
#include <sese/service/http/HttpServer_V3.h>

#include <utility>

using namespace sese::service::http::v3;

void HttpServer::regMountPoint(const std::string &uri_prefix, const std::string &local) {
    mount_points[uri_prefix] = local;
}

void HttpServer::regServlet(const net::http::Servlet &servlet) {
    this->servlets.emplace(servlet.getUri(), servlet);
}

void HttpServer::regFilter(const std::string &uri_prefix, const net::http::Servlet::Callback &callback) {
    this->filters[uri_prefix] = callback;
}

void HttpServer::setKeepalive(uint32_t seconds) {
    keepalive = std::max<uint32_t>(seconds, 5);
}

void HttpServer::regService(const net::IPAddress::Ptr &address, std::unique_ptr<security::SSLContext> context) {
    auto service = internal::service::http::v3::HttpServiceImpl::create(
            address, std::move(context), keepalive, name, mount_points, servlets, filters
    );
    this->services.push_back(service);
}

void HttpServer::setName(const std::string &name) {
    this->name = name;
}

bool HttpServer::startup() {
    for (auto &&item: services) {
        item->startup();
    }
    return true;
}

bool HttpServer::shutdown() {
    for (auto &&item: services) {
        item->shutdown();
    }
    return true;
}
