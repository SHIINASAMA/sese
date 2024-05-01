/// @file HttpServer_V3.h
/// @brief Http 服务器
/// @author kaoru
/// @date 2024年05月1日

#pragma once

#include <sese/service/http/HttpService_V3.h>
#include <sese/net/http/Controller.h>

namespace sese::service::http::v3 {

/// Http 服务器
class HttpServer final {
public:
    /// 注册控制器
    /// @tparam CTL 控制器类型
    /// @tparam ARGS 实例化参数类型
    /// @param args 实例化参数
    template<class CTL, class... ARGS>
    void regController(ARGS &&...args);

    /// 注册文件系统挂载点
    /// @param uri_prefix URI 前缀
    /// @param local 本地路径
    void regMountPoint(const std::string &uri_prefix, const std::string &local);

    /// 注册 Http 应用
    /// @param servlet Http 应用
    void regServlet(const net::http::Servlet &servlet);

    /// 设置 Keepalive
    /// @param seconds Keepalive 时间，最小值为 5
    void setKeepalive(uint32_t seconds);

    /// 注册 Http 服务
    /// @param address 监听地址
    /// @param context SSL 服务上下文，为空则不启用 SSL
    void regService(const net::IPAddress::Ptr &address, const security::SSLContext::Ptr &context);

    /// 设置服务器名称
    /// @param name 服务器名称
    void setName(const std::string &name);

    /// 启动服务
    /// @return 结果
    bool startup();

    /// 终止服务
    /// @return 结果
    bool shutdown();

private:
    std::string name;
    uint32_t keepalive = 5;
    std::vector<HttpService::Ptr> services;
    std::vector<net::http::Controller::Ptr> controllers;
    HttpService::MountPointMap mount_points;
    HttpService::ServletMap servlets;
};

template<class CTL, class... ARGS>
void HttpServer::regController(ARGS &&...args) {
    auto controller = std::make_shared<CTL>(std::forward<ARGS>(args)...);
    controller->init();
    for (auto &&servlet: *controller) {
        servlets.emplace(servlet.getUri(), servlet);
    }
    controllers.push_back(controller);
}

} // namespace sese::service::http::v3