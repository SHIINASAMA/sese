#include <sese/net/Socket.h>
#include <sese/util/Util.h>

sese::socket_t sese::net::Socket::socket(int family, int type, int protocol) noexcept {
    return ::socket(family, type, protocol);
}

sese::socket_t sese::net::Socket::accept(socket_t socket, sockaddr *addr, socklen_t *len) noexcept {
    return ::accept(socket, addr, len);
}

int sese::net::Socket::listen(socket_t socket, int backlog) noexcept {
    return ::listen(socket, backlog);
}

int sese::net::Socket::bind(socket_t socket, const sockaddr *addr, socklen_t addrLen) noexcept {
    return ::bind(socket, addr, addrLen);
}

int sese::net::Socket::connect(socket_t socket, const sockaddr *addr, socklen_t addrLen) noexcept {
    return ::connect(socket, addr, addrLen);
}

#ifdef SESE_PLATFORM_WINDOWS

int64_t sese::net::Socket::read(socket_t socket, void *buffer, size_t len, int flags) noexcept {
    return ::recv(socket, (char *) buffer, (int) len, flags);
}

int64_t sese::net::Socket::write(socket_t socket, const void *buffer, size_t len, int flags) noexcept {
    return ::send(socket, (const char *) buffer, (int) len, flags);
}

int sese::net::Socket::setNonblocking(socket_t socket) noexcept {
    unsigned long ul = 1;
    return ioctlsocket(socket, FIONBIO, &ul);
}

void sese::net::Socket::close(socket_t socket) noexcept {
    closesocket(socket);
}

int sese::net::getNetworkError() noexcept {
    return WSAGetLastError();
}

std::string sese::net::getNetworkErrorString(int error) noexcept {
    char *msg = nullptr;
    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            error,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            (LPTSTR) &msg,
            0,
            nullptr
    );
    return msg;
}
#else

#include <unistd.h>
#include <fcntl.h>

int64_t sese::net::Socket::write(socket_t socket, const void *buffer, size_t len, int flags) noexcept {
    return ::send(socket, buffer, len, flags);
}

int64_t sese::net::Socket::read(socket_t socket, void *buffer, size_t len, int flags) noexcept {
    return ::recv(socket, buffer, len, flags);
}

// 一般不会发生错误
// GCOVR_EXCL_START
int sese::net::Socket::setNonblocking(socket_t socket) noexcept {
    auto option = fcntl(socket, F_GETFL);
    if (option != -1) {
        return fcntl(socket, F_SETFL, option | O_NONBLOCK);
    } else {
        return -1;
    }
}
// GCOVR_EXCL_STOP

void sese::net::Socket::close(socket_t socket) noexcept {
    ::close(socket);
}

int sese::net::getNetworkError() noexcept {
    return errno;
}

std::string sese::net::getNetworkErrorString(int error) noexcept {
    return sese::getErrorString(error);
}

#endif

#include <sese/util/RandomUtil.h>

uint16_t sese::net::createRandomPort() noexcept {
    return sese::RandomUtil::next<uint16_t>(1025, 65535);
}