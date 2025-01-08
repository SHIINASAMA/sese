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

#include "sese/system/NetworkUtil.h"
#include "sese/net/IPv6Address.h"

#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <map>

using namespace sese::system;

std::vector<NetworkInterface> NetworkUtil::getNetworkInterface() noexcept {
    std::vector<NetworkInterface> interfaces;
    std::map<std::string, NetworkInterface> map;
    struct ifaddrs *p_if_address = nullptr;

    // These informations are used only to retrieve network card name, IPv4, and MAC information
    // glibc versions below 2.3.3 do not support retrieving IPv6-related information
    getifaddrs(&p_if_address);

    auto p_address = p_if_address;
    while (p_address) {
        // The trigger probability here is low,
        // and it can only be reproduced on servers with tunnel interfaces
        // GCOVR_EXCL_START
        if (p_address->ifa_addr == nullptr) {
            p_address = p_address->ifa_next;
            continue;
        }
        // GCOVR_EXCL_STOP

        if (p_address->ifa_addr->sa_family == AF_INET ||
            p_address->ifa_addr->sa_family == AF_PACKET) {

            auto iterator = map.find(p_address->ifa_name);
            if (iterator == map.end()) {
                auto i = NetworkInterface();
                iterator = map.insert({p_address->ifa_name, i}).first;
            }
            if (p_address->ifa_addr->sa_family == AF_INET) {
                sockaddr_in addr = *(sockaddr_in *) (p_address->ifa_addr);
                iterator->second.ipv4Addresses.emplace_back(std::make_shared<sese::net::IPv4Address>(addr));
            } else if (p_address->ifa_addr->sa_family == AF_PACKET) {
                iterator->second.name = p_address->ifa_name;
                memcpy(iterator->second.mac.data(), p_address->ifa_addr, 6);
            }
        }

        p_address = p_address->ifa_next;
    }

    freeifaddrs(p_if_address);

    // Used to obtain IPv6 information
    FILE *f = fopen("/proc/net/if_inet6", "r");
    // If it is false, the configuration file does not exist, and the IPv6 information is not obtained
    if (f != nullptr) { // GCOVR_EXCL_LINE
        int scope, prefix;
        unsigned char ipv6[16];
        char name[IFNAMSIZ];
        char address[INET6_ADDRSTRLEN];
        while (19 ==
               fscanf(f, // NOLINT
                      " %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx %*x %x %x %*x %s",
                      &ipv6[0],
                      &ipv6[1],
                      &ipv6[2],
                      &ipv6[3],
                      &ipv6[4],
                      &ipv6[5],
                      &ipv6[6],
                      &ipv6[7],
                      &ipv6[8],
                      &ipv6[9],
                      &ipv6[10],
                      &ipv6[11],
                      &ipv6[12],
                      &ipv6[13],
                      &ipv6[14],
                      &ipv6[15],
                      &prefix,
                      &scope,
                      name)) {
            auto iterator = map.find(name);
            // It can't be false here, because there must be MAC address information
            // if (iterator != map.end()) {
            inet_ntop(AF_INET6, ipv6, address, sizeof(address));
            auto addr = sese::net::IPv6Address::create(address, 0);
            iterator->second.ipv6Addresses.emplace_back(addr);
            // }
        }

        fclose(f);
    }

    interfaces.reserve(map.size());
    for (decltype(auto) i: map) {
        // Prevent empty names
        i.second.name = i.first;
        interfaces.emplace_back(i.second);
    }

    return interfaces;
}