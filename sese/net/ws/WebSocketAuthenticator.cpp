#include "sese/net/ws/WebsocketAuthenticator.h"
#include "sese/util/Random.h"
#include "sese/io/InputBufferWrapper.h"
#include "sese/io/OutputBufferWrapper.h"
#include "sese/convert/Base64Converter.h"
#include "sese/convert/SHA1Util.h"

#include <random>

using sese::io::InputBufferWrapper;
using sese::io::OutputBufferWrapper;

const char *sese::net::ws::WebsocketAuthenticator::append_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

void sese::net::ws::WebsocketAuthenticator::generateKey(uint8_t *key) {
    auto dev = std::random_device();
    auto r = static_cast<uint64_t>(dev());
    auto p = reinterpret_cast<uint8_t *>(&r);
    for (int i = 0; i < 8; ++i) {
        key[i] = *(p + i);
    }
    r = static_cast<decltype(r)>(dev());
    for (int i = 0; i < 8; ++i) {
        key[i + 8] = *(p + i);
    }
}

std::pair<std::unique_ptr<char[]>, std::unique_ptr<char[]>> sese::net::ws::WebsocketAuthenticator::generateKeyPair() noexcept {
    // 生成二进制 key
    uint8_t buffer[16];
    generateKey(buffer);
    // 生成字符串 key
    auto key_string = std::unique_ptr<char[]>(new char[25]);
    key_string.get()[24] = 0;
    {
        auto in = InputBufferWrapper(reinterpret_cast<const char *>(buffer), 16);
        auto out = OutputBufferWrapper((char *) key_string.get(), 24);
        Base64Converter::encode(&in, &out);
    }

    // 拼接 key
    char buffer0[24 + 36];
    memcpy(buffer0 + 0, key_string.get(), 24);
    memcpy(buffer0 + 24, append_string, 36);

    char buffer1[20];
    {
        auto in = InputBufferWrapper(buffer0, 60);
        auto out = OutputBufferWrapper(buffer1, 20);
        SHA1Util::encode(&in, &out);
    }

    auto result_string = std::unique_ptr<char[]>(new char[29]);
    result_string.get()[28] = 0;
    {
        auto in = InputBufferWrapper(buffer1, 20);
        auto out = OutputBufferWrapper(result_string.get(), 28);
        Base64Converter::encode(&in, &out);
    }

    return {std::move(key_string), std::move(result_string)};
}

bool sese::net::ws::WebsocketAuthenticator::verify(const char *key, const char *result) noexcept {
    // 拼接 key
    char buffer0[24 + 36];
    memcpy(buffer0 + 0, key, 24);
    memcpy(buffer0 + 24, append_string, 36);

    char buffer1[20];
    {
        auto in = InputBufferWrapper(buffer0, 60);
        auto out = OutputBufferWrapper(buffer1, 20);
        SHA1Util::encode(&in, &out);
    }

    char buffer2[29];
    buffer2[28] = 0;
    {
        auto in = InputBufferWrapper(buffer1, 20);
        auto out = OutputBufferWrapper(buffer2, 28);
        Base64Converter::encode(&in, &out);
    }

    return 0 == strcmp(result, buffer2);
}

std::unique_ptr<char[]> sese::net::ws::WebsocketAuthenticator::toResult(const char *key) noexcept {
    // 拼接 key
    char buffer0[24 + 36];
    memcpy(buffer0 + 0, key, 24);
    memcpy(buffer0 + 24, append_string, 36);

    char buffer1[20];
    {
        auto in = InputBufferWrapper(buffer0, 60);
        auto out = OutputBufferWrapper(buffer1, 20);
        SHA1Util::encode(&in, &out);
    }

    auto res = std::unique_ptr<char[]>(new char[29]);
    res.get()[28] = 0;
    {
        auto in = InputBufferWrapper(buffer1, 20);
        auto out = OutputBufferWrapper(res.get(), 28);
        Base64Converter::encode(&in, &out);
    }
    return std::move(res);
}
