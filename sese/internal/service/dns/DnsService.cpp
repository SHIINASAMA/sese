// Copyright 2024 libsese
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "DnsService.h"

#include <sese/net/dns/Config.h>
#include <sese/internal/net/AsioIPConvert.h>

using sese::internal::net::service::dns::DnsService;

DnsService::DnsService() : socket(io_service), buffer(), running(false) {
}

int DnsService::getLastError() {
    return error.value();
}

std::string DnsService::getLastErrorMessage() {
    return error.message();
}

bool DnsService::bind(const sese::net::IPAddress::Ptr &address) {
    auto addr = convert(address);
    auto endpoint = asio::ip::udp::endpoint(addr, address->getPort());
    if (!socket.is_open()) {
        socket.open(endpoint.protocol());
        socket.non_blocking(true);
    }
    error = socket.bind(endpoint, error);
    if (error) {
        return false;
    }
    return true;
}

void DnsService::setCallback(const sese::service::dns::Callback &callback) {
    this->callback = callback;
}

void DnsService::addUpstreamNameServer(const sese::net::IPAddress::Ptr &address) {
    resolver.addNameServer(address);
}

bool DnsService::addUpstreamNameServer(const std::string &ip, uint16_t port) {
    return resolver.addNameServer(ip, port);
}

void DnsService::addRecord(const std::string &name, const sese::net::IPAddress::Ptr &address) {
    if (address->getFamily() == AF_INET6) {
        v6map[name] = std::dynamic_pointer_cast<sese::net::IPv6Address>(address);
        return;
    }
    v4map[name] = std::dynamic_pointer_cast<sese::net::IPv4Address>(address);
}

bool DnsService::startup() {
    running = true;
    // clang-format off
    thread = std::make_unique<Thread>([&] {this->handle();}, "DnsService");
    // clang-format on
    thread->start();
    return true;
}

bool DnsService::shutdown() {
    running = false;
    if (thread->joinable()) {
        thread->join();
    }
    if (socket.is_open()) {
        socket.close();
    }
    return true;
}

void DnsService::handle() {
    while (running) {
        asio::ip::udp::endpoint endpoint;
        size_t length = socket.receive_from(asio::buffer(buffer.data(), buffer.size()), endpoint, 0, error);
        if (error) {
            if (error.value() == asio::error::timed_out) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            continue;
        }
        auto recv_package = sese::net::dns::DnsPackage::decode(buffer.data(), length);
        if (!recv_package) {
            continue;
        }
        auto flags = sese::net::dns::DnsPackage::Flags();
        flags.qr = true;
        auto send_package = sese::net::dns::DnsPackage::new_();
        send_package->setId(recv_package->getId());
        send_package->setFlags(flags.encode());
        auto addr = convert(endpoint);

        bool handled = false;
        if (callback) {
            handled = callback(addr, recv_package, send_package);
        }
        if (handled) {
            auto index = send_package->buildIndex();
            send_package->encode(buffer.data(), length, index);
            socket.send_to(asio::buffer(buffer.data(), length), endpoint, 0, error);
            continue;
        }

        auto &&answers = send_package->getAnswers();
        handleBySelf(recv_package->getQuestions(), send_package);
        handleByUpstream(recv_package->getQuestions(), send_package);
        // todo There is also no need to have a record upstream

        auto index = send_package->buildIndex();
        length = buffer.size();
        if (!send_package->encode(buffer.data(), length, index)) {
            continue;
        }
        socket.send_to(asio::buffer(buffer.data(), length), endpoint, 0, error);
    }
}

void DnsService::handleBySelf(
    std::vector<sese::net::dns::DnsPackage::Question> &questions,
    sese::net::dns::DnsPackage::Ptr &send_package
) {
    auto &answers = send_package->getAnswers();
    for (auto q_iterator = questions.begin(); q_iterator != questions.end();) {
        auto &&name = q_iterator->name;
        auto &&type = q_iterator->type;
        auto &&class_ = q_iterator->class_;
        if (class_ != sese::net::dns::CLASS_IN) {
            ++q_iterator;
            continue;
        }
        sese::net::dns::DnsPackage::Answer answer;
        answer.name = name;
        answer.type = type;
        answer.class_ = class_;
        answer.ttl = 114514;
        if (type == sese::net::dns::TYPE_A) {
            auto iterator = v4map.find(name);
            if (iterator != v4map.end()) {
                answer.data_length = 4;
                answer.data = std::make_unique<uint8_t[]>(answer.data_length);
                auto result = reinterpret_cast<sockaddr_in *>(iterator->second->getRawAddress());
                memcpy(answer.data.get(), &result->sin_addr.s_addr, 4);
                answers.push_back(std::move(answer));
                send_package->getQuestions().push_back(*q_iterator);
                q_iterator = questions.erase(q_iterator);
                continue;
            }
        } else if (type == sese::net::dns::TYPE_AAAA) {
            auto iterator = v6map.find(name);
            if (iterator != v6map.end()) {
                answer.data_length = 16;
                answer.data = std::make_unique<uint8_t[]>(answer.data_length);
                auto result = reinterpret_cast<sockaddr_in6 *>(iterator->second->getRawAddress());
                memcpy(answer.data.get(), &result->sin6_addr.s6_addr, 16);
                answers.push_back(std::move(answer));
                send_package->getQuestions().push_back(*q_iterator);
                q_iterator = questions.erase(q_iterator);
                continue;
            }
        }
        ++q_iterator;
    }
}

void DnsService::handleByUpstream(
    std::vector<sese::net::dns::DnsPackage::Question> &questions,
    sese::net::dns::DnsPackage::Ptr &send_package
) {
    // todo TTL issues will also need to be dealt with in the future
    auto &answers = send_package->getAnswers();
    for (auto q_iterator = questions.begin(); q_iterator != questions.end();) {
        auto &&name = q_iterator->name;
        auto &&type = q_iterator->type;
        auto &&class_ = q_iterator->class_;
        auto ips = resolver.resolve(name, type);
        if (ips.empty()) {
            ++q_iterator;
            continue;
        }
        bool handled = false;
        for (auto &&ip: ips) {
            sese::net::dns::DnsPackage::Answer answer;
            answer.name = name;
            answer.type = type;
            answer.class_ = class_;
            answer.ttl = 114514;
            if (ip->getFamily() == AF_INET) {
                answer.data_length = 4;
                answer.data = std::make_unique<uint8_t[]>(answer.data_length);
                auto result = reinterpret_cast<sockaddr_in *>(ip->getRawAddress());
                memcpy(answer.data.get(), &result->sin_addr.s_addr, 4);
                answers.push_back(std::move(answer));
                v4map[name] = std::dynamic_pointer_cast<sese::net::IPv4Address>(ip);
                handled = true;
            } else {
                answer.data_length = 16;
                answer.data = std::make_unique<uint8_t[]>(answer.data_length);
                auto result = reinterpret_cast<sockaddr_in6 *>(ip->getRawAddress());
                memcpy(answer.data.get(), &result->sin6_addr.s6_addr, 16);
                answers.push_back(std::move(answer));
                v6map[name] = std::dynamic_pointer_cast<sese::net::IPv6Address>(ip);
                handled = true;
            }
        }
        if (handled) {
            send_package->getQuestions().push_back(*q_iterator);
            q_iterator = questions.erase(q_iterator);
        } else {
            ++q_iterator;
        }
    }
}
