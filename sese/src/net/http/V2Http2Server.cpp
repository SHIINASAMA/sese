#include "sese/net/http/V2Http2Server.h"
#include "sese/net/http/Http2FrameInfo.h"
#include "sese/net/http/HttpUtil.h"
#include "sese/net/http/HttpServer.h"
#include "sese/util/Endian.h"

#include <cmath>

using namespace sese::net::http;
using namespace sese::net::v2::http;

using HttpContext = sese::net::http::HttpServiceContext<sese::net::v2::IOContext>;

void Http2Server::onConnect(sese::net::v2::IOContext &ctx) noexcept {
}

void Http2Server::onHandle(sese::net::v2::IOContext &ctx) noexcept {
    mutex.lock();
    auto connIterator = connMap.find(ctx.getIdent());
    if (connIterator == connMap.end()) {
        mutex.unlock();
        char buffer[MAGIC_STRING.length()];
        ctx.peek(buffer, MAGIC_STRING.length());
        if (MAGIC_STRING == buffer) {
            ctx.read(buffer, MAGIC_STRING.length());
            auto conn = std::make_shared<Http2Connection>();
            conn->socket = ctx.getIdent();
            mutex.lock();
            connMap[conn->socket] = conn;
            mutex.unlock();
            onHttp2Handle(ctx, connIterator->second);
        } else {
            onHttpHandle(ctx);
        }
    } else {
        onHttp2Handle(ctx, connIterator->second);
    }
}

void Http2Server::onClosing(sese::net::v2::IOContext &ctx) noexcept {
    mutex.lock();
    auto iterator = connMap.find(ctx.getIdent());
    if (iterator != connMap.end()) {
        connMap.erase(iterator);
    }
    mutex.unlock();
}

void Http2Server::onHttpHandle(sese::net::v2::IOContext &ctx) noexcept {
    auto httpContext = HttpContext();
    httpContext.reset(&ctx);

    if (!sese::net::http::HttpUtil::recvRequest(&httpContext, &httpContext.request)) {
        httpContext.close();
        return;
    }

    // 此处判断升级握手

    if (!httpContext.isFlushed()) {
        if (!httpContext.flush()) {
            httpContext.close();
            return;
        }
    }
}

void Http2Server::onHttp2Handle(sese::net::v2::IOContext &ctx, net::http::Http2Connection::Ptr conn) noexcept {
    Http2FrameInfo frame{};
    while (true) {
        if (!readFrame(ctx, frame)) {
            ctx.close();
            return;
        }

        // SETTINGS 帧
        if (frame.ident == 0) {
            if (frame.type == FRAME_TYPE_SETTINGS) {
                // 为当前连接更改设置
            } else {
                // 非法帧
                conn->mutex.lock();
                sendGoaway(ctx, frame.ident, GOAWAY_PROTOCOL_ERROR);
                conn->mutex.unlock();
                ctx.close();
                break;
            }
        }

        auto stream = conn->find(frame.ident);
        if (!stream) {
            stream = std::make_shared<Http2Stream>();
            conn->addStream(frame.ident, stream);
        }

        // HEADER 帧
        if (frame.type == FRAME_TYPE_HEADERS) {
            char buffer[1024];
            size_t length = 0;
            while (length < frame.length) {
                auto need = frame.length - length > 1024 ? 1024 : frame.length - length;
                auto l = ctx.read(buffer, need);
                if (l > 0) {
                    stream->buffer.write(buffer, l);
                    length += l;
                } else {
                    ctx.close();
                    break;
                }
            }
            if (frame.flags == FRAME_FLAG_END_HEADERS) {
                // 触发头解析
                if (!decode(stream.get(), conn->dynamicTable4recv, stream->requestHeader)) {
                    // 解析失败关闭整个连接
                    ctx.close();
                    break;
                }
                // Content-Length 不为 0 则需要接收 data 帧
                if (stream->requestHeader.get("Content-Length", "undef") == "undef") {
                    //todo 触发请求解析 - 任务提交
                }
            }
        }
            // DATA 帧
        else if (frame.type == FRAME_TYPE_DATA) {
            char buffer[1024];
            size_t length = 0;
            while (length < frame.length) {
                auto need = frame.length - length > 1024 ? 1024 : frame.length - length;
                auto l = ctx.read(buffer, need);
                if (l > 0) {
                    stream->buffer.write(buffer, l);
                    length += l;
                } else {
                    ctx.close();
                    break;
                }
            }
            if (frame.flags == FRAME_FLAG_END_STREAM) {
                //todo 触发请求解析 - 任务提交
            }
        } else {
            // 不处理
        }
    }
}

bool Http2Server::readFrame(IOContext &ctx, sese::net::http::Http2FrameInfo &frame) noexcept {
    uint8_t buffer[9]{};
    auto read = ctx.read(buffer, 9);
    if (read != 9) return false;

    memcpy(&frame.length, buffer + 0, 3);
    memcpy(&frame.type, buffer + 3, 1);
    memcpy(&frame.flags, buffer + 4, 1);
    memcpy(&frame.ident, buffer + 5, 4);

    frame.length = FromBigEndian32(frame.length);
    frame.ident = FromBigEndian32(frame.ident);
    return true;
}

void Http2Server::sendGoaway(sese::net::v2::IOContext &ctx, uint32_t sid, uint32_t eid) noexcept {
    int32_t buffer[2];
    buffer[0] = ToBigEndian32(sid) >> 1;// NOLINT
    buffer[1] = ToBigEndian32(eid);     // NOLINT
    ctx.write(&buffer, 2 * sizeof(uint32_t));
}

bool Http2Server::decode(sese::InputStream *input, DynamicTable &dynamicTable, Header &header) noexcept {
    uint8_t buf;
    int64_t len;
    while ((len = input->read(&buf, 1)) > 0) {
        // key & value 均在索引
        if (buf & 0b1000'0000) {
            uint32_t index = 0;
            decodeInteger(buf, input, index, 7);
            if (index == 0) return false;

            auto pair = dynamicTable.get(index);
            if (pair == std::nullopt) return false;
            header.set(pair->first, pair->second);
        } else {
            uint32_t index = 0;
            bool isStore;
            if (0b0100'0000 == (buf & 0b1100'0000)) {
                // 添加至动态表
                decodeInteger(buf, input, index, 6);
                isStore = true;
            } else {
                // 不添加至动态表
                decodeInteger(buf, input, index, 4);
                isStore = false;
            }

            std::string key;
            if (0 != index) {
                auto ret = dynamicTable.get(index);
                if (ret == std::nullopt) {
                    return false;
                }
                key = ret.value().first;
            } else {
                auto ret = decodeString(input);
                if (ret == std::nullopt) {
                    return false;
                }
                key = ret.value();
            }

            auto ret = decodeString(input);
            if (ret == std::nullopt) {
                return false;
            }

            if (isStore) {
                dynamicTable.set(key, ret.value());
            }

            header.set(key, ret.value());
        }
    }
    return true;
}

void Http2Server::decodeInteger(uint8_t &buf, sese::InputStream *input, uint32_t &dest, uint8_t n) noexcept {
    const auto two_N = static_cast<uint16_t>(std::pow(2, n) - 1);
    dest = buf & two_N;
    if (dest == two_N) {
        uint64_t M = 0;
        while ((input->read(&buf, 1)) > 0) {
            dest += (buf & 0x7F) << M;
            M += 7;

            if (!(buf & 0x80)) {
                break;
            }
        }
    }
}

std::optional<std::string> Http2Server::decodeString(sese::InputStream *input) noexcept {
    uint8_t buf;
    input->read(&buf, 1);
    uint8_t len = (buf & 0x7F);
    bool isHuffman = (buf & 0x80) == 0x80;

    char buffer[UINT8_MAX]{};
    if (len != input->read(buffer, len)) {
        return nullptr;
    }

    if (isHuffman) {
        return decoder.decode(buffer, len);
    }

    return buffer;
}
