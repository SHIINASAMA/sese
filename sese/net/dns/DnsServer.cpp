#include <sese/net/dns/DnsServer.h>
#include <sese/net/dns/DnsUtil.h>
#include <sese/io/InputBufferWrapper.h>
#include <sese/io/OutputBufferWrapper.h>
#include <sese/util/Util.h>

sese::net::dns::DnsServer::Ptr sese::net::dns::DnsServer::create(const sese::net::dns::DNSConfig *config) noexcept {
    auto socket = std::make_shared<sese::net::Socket>(Socket::Family::IPv4, Socket::Type::UDP, IPPROTO_IP);
    if (socket->bind(config->address)) {
        return nullptr;
    }

    socket->setNonblocking();

    auto ptr = new DnsServer;
    ptr->socket = socket;

    for (auto &item: config->hostIPv4Map) {
        // auto v = text::StringBuilder::split(item.second, ".");
        // if (v.size() != 4) {
        //     continue;
        // }
        // char *end;
        // uint8_t buffer[4];
        // for (int i = 0; i < 4; ++i) {
        //     buffer[i] = (uint8_t) std::strtol(v[i].c_str(), &end, 10);
        // }
        uint8_t buffer[4];
        sese::net::inetPton(AF_INET, item.second.c_str(), buffer);
        ptr->hostIPv4Map[item.first] = std::string((const char *) buffer, 4);
    }

    for (auto &item: config->hostIPv6Map) {
        uint8_t buffer[16];
        sese::net::inetPton(AF_INET6, item.second.c_str(), buffer);
        ptr->hostIPv6Map[item.first] = std::string((const char *) buffer, 16);
    }

    return std::unique_ptr<DnsServer>(ptr);
}

sese::net::dns::DnsServer::~DnsServer() noexcept {
    if (thread && !isShutdown) {
        shutdown();
        thread = nullptr;
    }

    if (socket) {
        socket->close();
        socket = nullptr;
    }
}

void sese::net::dns::DnsServer::start() noexcept {
    this->thread = std::make_unique<Thread>([this] { this->loop(); }, "DNS");
    this->thread->start();
}

void sese::net::dns::DnsServer::shutdown() noexcept {
    if (thread) {
        isShutdown = true;

        thread->join();
        socket->close();

        thread = nullptr;
        socket = nullptr;
    }
}

void sese::net::dns::DnsServer::loop() noexcept {
    auto clientAddress = std::make_shared<sese::net::IPv4Address>();
    while (true) {
        if (isShutdown) {
            break;
        }

        uint8_t buffer[DNS_PACKAGE_SIZE];
        auto len = socket->recv(buffer, sizeof(buffer), clientAddress, 0);
        if (len <= 0) {
            sese::sleep(0);
            continue;
        }

        auto input = sese::io::InputBufferWrapper((const char *) buffer + 12, len - 12);
        auto output = sese::io::OutputBufferWrapper((char *) buffer + 12, sizeof(buffer) - 12);

        FrameHeaderInfo info;
        DndSession session;
        DnsUtil::decodeFrameHeaderInfo(buffer, info);
        DnsUtil::decodeQueries(info.questions, &input, session.getQueries());

        info.flags.rcode = SESE_DNS_RCODE_NO_ERROR;
        info.flags.RD = 0;
        info.flags.QR = 1;
        for (auto &q: session.getQueries()) {
            if (q.getType() == SESE_DNS_QR_TYPE_A) {
                auto iterator = hostIPv4Map.find(q.getName());
                if (iterator == hostIPv4Map.end()) {
                    info.flags.rcode = SESE_DNS_RCODE_NAME_ERROR;
                    continue;
                } else {
                    uint16_t offset = q.getOffset();
                    offset |= 0b1100'0000'0000'0000;
                    offset = ToBigEndian16(offset);
                    std::string name = {(const char *) &offset, 2};
                    session.getAnswers().emplace_back(
                            name,
                            SESE_DNS_QR_TYPE_A,
                            SESE_DNS_QR_CLASS_IN,
                            600,
                            4,
                            iterator->second
                    );
                }
                info.answerPrs += 1;
            } else if (q.getType() == SESE_DNS_QR_TYPE_AAAA) {
                auto iterator = hostIPv6Map.find(q.getName());
                if (iterator == hostIPv6Map.end()) {
                    info.flags.rcode = SESE_DNS_RCODE_NAME_ERROR;
                    continue;
                } else {
                    uint16_t offset = q.getOffset();
                    offset |= 0b1100'0000'0000'0000;
                    offset = ToBigEndian16(offset);
                    std::string name = {(const char *) &offset, 2};
                    session.getAnswers().emplace_back(
                            name,
                            SESE_DNS_QR_TYPE_AAAA,
                            SESE_DNS_QR_CLASS_IN,
                            600,
                            16,
                            iterator->second
                    );
                }
                info.answerPrs += 1;
            } else {
                info.flags.rcode = SESE_DNS_RCODE_NOT_IMPLEMENTED;
                continue;
            }
        }

        DnsUtil::encodeFrameHeaderInfo(buffer, info);
        DnsUtil::encodeQueries(&output, session.getQueries());
        DnsUtil::encodeAnswers(&output, session.getAnswers());
        socket->send(buffer, 12 + output.getLength(), clientAddress, 0);
    }
}