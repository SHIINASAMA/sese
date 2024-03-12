/**
 * @file IPAddress.h
 * @author kaoru
 * @brief IP 地址类
 * @date 2022年4月4日
 */
#pragma once

#include "sese/net/Address.h"
#include "sese/util/Endian.h"

namespace sese::net {

/**
 * @brief IP 地址类
 */
class API IPAddress : public Address {
public:
    using Ptr = std::shared_ptr<IPAddress>;

    static Ptr create(const char *address, uint16_t port = 0);

public:
    [[nodiscard]] virtual Ptr getBroadcastAddress(uint32_t prefix_len) const noexcept = 0;
    [[nodiscard]] virtual Ptr getNetworkAddress(uint32_t prefix_len) const noexcept = 0;
    [[nodiscard]] virtual Ptr getSubnetMask(uint32_t prefix_len) const noexcept = 0;
    virtual void setPort(uint16_t port) noexcept = 0;
    [[nodiscard]] virtual uint16_t getPort() const noexcept = 0;
    virtual void setFamily(uint16_t family) noexcept = 0;
    [[nodiscard]] virtual uint16_t getFamily() noexcept = 0;
};

} // namespace sese::net