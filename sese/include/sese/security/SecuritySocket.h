/// \file SecuritySocket.h
/// \brief 安全套接字类
/// \author kaoru
/// \version 0.2
/// \date 2023年7月25日

#pragma once

#include <sese/net/Socket.h>

namespace sese::security {

    class API SSLContext;

    /// 安全套接字
    class API SecuritySocket final : public sese::net::Socket {
    public:
        using Socket = sese::net::Socket;
        using Address = sese::net::Address;

        SecuritySocket(std::shared_ptr<SSLContext> context, Socket::Family family, int32_t flags) noexcept;
        SecuritySocket(std::shared_ptr<SSLContext> context, void *ssl, socket_t fd) noexcept;

        int32_t connect(Address::Ptr address) noexcept override;
        [[nodiscard]] Socket::Ptr accept() const override;
        [[nodiscard]] int32_t shutdown(ShutdownMode mode) const override;
        int64_t read(void *buffer, size_t length) override;
        int64_t write(const void *buffer, size_t length) override;
        void close() override;

    private:
        std::shared_ptr<SSLContext> context;
        void *ssl = nullptr;
    };
}// namespace sese::security
