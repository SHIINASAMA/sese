/**
 * \file Client.h
 * \brief RPC 客户端
 * \author kaoru
 * \version 0.1
 */
#pragma once
#include <sese/net/IPv6Address.h>
#include <sese/net/rpc/Marco.h>
#include <sese/config/json/JsonTypes.h>

namespace sese::net::rpc {

    /// RPC 客户端
    class API Client final {
    public:
        explicit Client(const IPv4Address::Ptr &address, const std::string &version = SESE_RPC_VERSION_0_1) noexcept;

        json::ObjectData::Ptr call(const std::string &name, json::ObjectData::Ptr &args) noexcept;

    private:
        json::ObjectData::Ptr makeTemplateRequest(const std::string &name);

    private:
        IPv4Address::Ptr address;
        json::BasicData::Ptr version;
    };

    /// 获取返回码对应的错误信息
    /// \param code 返回码
    /// \return 错误信息
    API const char *getErrorMessage(int64_t code) noexcept;
}