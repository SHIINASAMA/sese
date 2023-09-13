/**
* @file NetworkUtil.h
* @author kaoru
* @version 0.1
* @brief 网络工具类
* @date 2023年9月13日
*/

#pragma once

#include "sese/net/IPv6Address.h"

#include <array>

namespace sese::system {

    /// 网络接口信息
    struct NetworkInterface {
        std::string name;
        std::vector<sese::net::IPv4Address::Ptr> ipv4Addresses;
        std::vector<sese::net::IPv6Address::Ptr> ipv6Addresses;
        std::array<unsigned char, 6> mac;
    };

    /// 网络工具类
    class NetworkUtil {
    public:
        static std::vector<NetworkInterface> getNetworkInterface() noexcept;
    };
}