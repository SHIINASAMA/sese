#include <sese/service/Http2Service.h>
#include <sese/net/http/HPackUtil.h>
#include <sese/text/StringBuilder.h>
#include <sese/util/Util.h>

#define CONFIG ((sese::service::Http2Config *) this->config)
#define RECV_BUFFER (conn->req.getBody())
#define SEND_BUFFER (conn->buffer)
#define TEMP_BUFFER (conn->resp.getBody())

sese::service::Http2Service::Http2Service(sese::service::Http2Config *config) : HttpService(config) {
}

sese::service::Http2Service::~Http2Service() noexcept {
    for (decltype(auto) item: conn2Map) {
        delete item.second;
    }
    conn2Map.clear();
}

void sese::service::Http2Service::onClose(sese::event::BaseEvent *event) {
    HttpService::onClose(event);
}

void sese::service::Http2Service::onHandle(sese::net::http::HttpConnection *conn) noexcept {
    auto iterator = conn2Map.find(conn);
    if (iterator == conn2Map.end()) {
        HttpService::onHandle(conn);
        return;
    }

    onHandleHttp2(conn);
}

void sese::service::Http2Service::onHandleUpgrade(sese::net::http::HttpConnection *conn) noexcept {
    auto &req = conn->req;
    auto &resp = conn->resp;
    auto fail = [&]() {
        resp.setCode(200);
        if (conn->timeoutEvent) {
            this->freeTimeoutEvent(conn->timeoutEvent);
            conn->timeoutEvent = nullptr;
        }
    };

    if (req.getType() != net::http::RequestType::Get) {
        fail();
        return;
    }

    if (req.getUrl() != CONFIG->upgradePath) {
        fail();
        return;
    }

    const char *SSL = "h2";
    const char *NoSSL = "h2c";
    const char *UPGRADE_CMP_STR = SSL;
    if (!CONFIG->servCtx) {
        UPGRADE_CMP_STR = NoSSL;
    }

    auto upgradeStr = req.get("upgrade", "undef");
    if (!sese::strcmpDoNotCase(upgradeStr.c_str(), UPGRADE_CMP_STR)) {
        fail();
        return;
    }

    // 状态变量
    bool upgrade = false;
    bool settings = false;
    // 确认状态
    auto connectString = req.get("connection", "undef");
    auto upgradeString = req.get("upgrade", "undef");
    if (connectString != "undef") {
        auto connectVector = text::StringBuilder::split(connectString, ", ");
        for (decltype(auto) str: connectVector) {
            if (strcasecmp(str.c_str(), "upgrade") == 0) {
                upgrade = true;
            } else if (strcasecmp(str.c_str(), "http2-settings") == 0) {
                settings = true;
            }
        }
    }

    if (!upgrade) {
        fail();
        return;
    }

    auto conn2 = new sese::net::http::Http2Connection;
    conn2->data = conn;
    // 需要解析 Http2-Settings 字段
    if (settings) {
        auto settingStr = req.get("http2-settings", "");
        conn2->decodeHttp2Settings(settingStr);
    }

    resp.setCode(101);
    resp.set("connection", "upgrade");
    resp.set("upgrade", UPGRADE_CMP_STR);
    conn2Map[conn] = conn2;
}

void sese::service::Http2Service::onHandleHttp2(net::http::HttpConnection *conn) noexcept {
    net::http::Http2FrameInfo frame{};

    auto conn2 = conn2Map[conn];
    if (conn2->first) {
        conn2->first = false;
        RECV_BUFFER.trunc(24);

        frame.type = net::http::FRAME_TYPE_SETTINGS;
        writeFrame(conn, frame);
    }

    while (true) {
        auto ret = readFrame(conn, frame);
        if (!ret) {
            return;
        }

        if (frame.type == net::http::FRAME_TYPE_SETTINGS) {
            onSettingsFrame(conn2, frame);
            continue;
        } else if (frame.type == net::http::FRAME_TYPE_WINDOW_UPDATE) {
            onWindowUpdateFrame(conn2, frame);
            continue;
        } else if (frame.type == net::http::FRAME_TYPE_HEADERS || frame.type == net::http::FRAME_TYPE_CONTINUATION) {
            onHeadersFrame(conn2, frame);
            continue;
        } else if (frame.type == net::http::FRAME_TYPE_DATA) {
            continue;
        } else {
            continue;
        }
    }
}

void sese::service::Http2Service::onTimeout(sese::service::TimeoutEvent *timeoutEvent) {
    HttpService::onTimeout(timeoutEvent);
}

void sese::service::Http2Service::requestFromHttp2(net::http::Request &request) noexcept {
    request.setVersion(net::http::HttpVersion::VERSION_2);
    request.setUrl(request.get(":path", "/"));
    auto method = request.get(":method", "get");
    if (strcmpDoNotCase(method.c_str(), "get")) {
        request.setType(net::http::RequestType::Get);
    } else if (strcmpDoNotCase(method.c_str(), "options")) {
        request.setType(net::http::RequestType::Options);
    } else if (strcmpDoNotCase(method.c_str(), "post")) {
        request.setType(net::http::RequestType::Post);
    } else if (strcmpDoNotCase(method.c_str(), "head")) {
        request.setType(net::http::RequestType::Head);
    } else if (strcmpDoNotCase(method.c_str(), "put")) {
        request.setType(net::http::RequestType::Put);
    } else if (strcmpDoNotCase(method.c_str(), "delete")) {
        request.setType(net::http::RequestType::Delete);
    } else if (strcmpDoNotCase(method.c_str(), "trace")) {
        request.setType(net::http::RequestType::Trace);
    } else if (strcmpDoNotCase(method.c_str(), "connect")) {
        request.setType(net::http::RequestType::Connect);
    } else {
        request.setType(net::http::RequestType::Another);
    }
}

void sese::service::Http2Service::writeFrame(net::http::HttpConnection *conn, const net::http::Http2FrameInfo &info) noexcept {
    auto len = ToBigEndian32(info.length);
    auto ident = ToBigEndian32(info.ident);

    char buffer[9];
    memcpy(buffer + 0, ((const char *) &len) + 1, 3);
    memcpy(buffer + 3, &info.type, 1);
    memcpy(buffer + 4, &info.flags, 1);
    memcpy(buffer + 5, &ident, 4);

    SEND_BUFFER.write(buffer, 9);
}

#define ASSERT_READ(buf, len)                \
    if (RECV_BUFFER.read(buf, len) != len) { \
        printf("%d\n", len);                 \
        return false;                        \
    }                                        \
    SESE_MARCO_END

bool sese::service::Http2Service::readFrame(sese::net::http::HttpConnection *conn, sese::net::http::Http2FrameInfo &info) noexcept {
    uint8_t buffer[9]{};
    ASSERT_READ(buffer, 9);

    memset(&info, 0, sizeof(info));
    memcpy(((char *) &info.length) + 1, buffer + 0, 3);
    memcpy(&info.type, buffer + 3, 1);
    memcpy(&info.flags, buffer + 4, 1);
    memcpy(&info.ident, buffer + 5, 4);

    info.length = FromBigEndian32(info.length);
    info.ident = FromBigEndian32(info.ident);
    return true;
}


void sese::service::Http2Service::onSettingsFrame(net::http::Http2Connection *conn2, sese::net::http::Http2FrameInfo &info) noexcept {
    auto conn = (net::http::HttpConnection *) conn2->data;

    char buffer[6];
    auto ident = (uint16_t *) &buffer[0];
    auto value = (uint32_t *) &buffer[2];

    size_t length = 0;

    while (RECV_BUFFER.read(buffer, 6) == 6) {
        *ident = FromBigEndian16(*ident);
        *value = FromBigEndian32(*value);

        switch (*ident) {
            case net::http::SETTINGS_HEADER_TABLE_SIZE:
                conn2->dynamicTable1.resize(*value);
                break;
            case net::http::SETTINGS_MAX_CONCURRENT_STREAMS:
                conn2->maxConcurrentStream = *value;
                break;
            case net::http::SETTINGS_MAX_FRAME_SIZE:
                conn2->maxFrameSize = *value;
                break;
            case net::http::SETTINGS_ENABLE_PUSH:
                conn2->enablePush = *value;
                break;
            case net::http::SETTINGS_MAX_HEADER_LIST_SIZE:
                conn2->maxHeaderListSize = *value;
            case net::http::SETTINGS_INITIAL_WINDOW_SIZE:
                conn2->windowSize = *value;
            default:
                // 按照标准忽略该设置
                break;
        }

        length += 6;
        if (length == info.length) {
            break;
        }
    }
}

void sese::service::Http2Service::onWindowUpdateFrame(net::http::Http2Connection *conn2, net::http::Http2FrameInfo &info) noexcept {
    auto conn = (net::http::HttpConnection *) conn2->data;
    // 不做控制，读取负载
    uint32_t data;
    RECV_BUFFER.read(&data, sizeof(data));
}

void sese::service::Http2Service::onHeadersFrame(sese::net::http::Http2Connection *conn2, sese::net::http::Http2FrameInfo &info) noexcept {
    net::http::Http2Stream::Ptr stream;
    auto conn = (net::http::HttpConnection *) conn2->data;
    auto iterator = conn2->streamMap.find(info.ident);
    if (iterator == conn2->streamMap.end()) {
        stream = std::make_shared<net::http::Http2Stream>();
        conn2->streamMap[info.ident] = stream;
    } else {
        stream = iterator->second;
    }

    char buffer[MTU_VALUE];
    size_t length = 0;
    while (true) {
        auto need = info.length - length > MTU_VALUE ? MTU_VALUE : info.length - length;
        if (need == 0) {
            break;
        }
        auto l = RECV_BUFFER.read(buffer, need);
        if (l > 0) {
            stream->req.getBody().write(buffer, l);
        } else {
            // handle error
            break;
        }
        length += l;
    }
    stream->headerSize += length;

    if (info.flags == net::http::FRAME_FLAG_END_HEADERS) {
        if (!net::http::HPackUtil::decode(&stream->req.getBody(), stream->headerSize, conn2->dynamicTable1, stream->req)) {
            // handle error
        }
        stream->req.getBody().freeCapacity();
    }

    auto contentLength = stream->req.get("content-length", "0");
    if (contentLength == "0") {
        requestFromHttp2(stream->req);
        // todo 选择控制器
    }
}

void sese::service::Http2Service::onDataFrame(net::http::Http2Connection *conn2, net::http::Http2FrameInfo &info) noexcept {
    net::http::Http2Stream::Ptr stream;
    auto conn = (net::http::HttpConnection *) conn2->data;
    auto iterator = conn2->streamMap.find(info.ident);
    if (iterator == conn2->streamMap.end()) {
        stream = std::make_shared<net::http::Http2Stream>();
        conn2->streamMap[info.ident] = stream;
    } else {
        stream = iterator->second;
    }

    char buffer[MTU_VALUE];
    size_t length = 0;
    while (true) {
        auto need = info.length - length > MTU_VALUE ? MTU_VALUE : info.length - length;
        if (need == 0) {
            break;
        }
        auto l = RECV_BUFFER.read(buffer, need);
        if (l > 0) {
            stream->req.getBody().write(buffer, l);
        } else {
            // handle error
            break;
        }
        length += l;
    }


    if (info.flags == net::http::FRAME_FLAG_END_STREAM) {
        // todo 选择控制器
    }
}
