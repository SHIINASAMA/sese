#include "sese/system/NetworkUtil.h"
#include "sese/net/IPv6Address.h"
#include <vector>


#ifdef SESE_PLATFORM_WINDOWS

#include <iphlpapi.h>

std::vector<sese::NetworkInterface> sese::NetworkUtil::getNetworkInterface() noexcept {
    ULONG rt = 0;
    ULONG size = 0;
    std::unique_ptr<CHAR[]> buffer;
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;

    std::vector<NetworkInterface> result;

    rt = GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
            nullptr,
            pAddresses,
            &size
    );

    // 此处必为 ERROR_BUFFER_OVERFLOW，作用仅为获取实际所需大小
    if (rt != ERROR_BUFFER_OVERFLOW) {
        return result;
    }

    buffer = std::make_unique<CHAR[]>(size);
    pAddresses = (PIP_ADAPTER_ADDRESSES) buffer.get();

    rt = GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
            nullptr,
            pAddresses,
            &size
    );

    // 此处为 ERROR_SUCCESS 为成功
    if (rt != ERROR_SUCCESS) {
        return result;
    }

    NetworkInterface Interface;
    PIP_ADAPTER_UNICAST_ADDRESS_LH unicastAddress;
    while (pAddresses) {
        if (pAddresses->IfType == IF_TYPE_ETHERNET_CSMACD) {
            Interface.name = pAddresses->AdapterName;

            unicastAddress = pAddresses->FirstUnicastAddress;
            while (unicastAddress) {
                auto pSockAddr = (SOCKADDR *) unicastAddress->Address.lpSockaddr;
                if (pSockAddr->sa_family == AF_INET) {
                    sockaddr_in sockaddrIn = *(sockaddr_in *) unicastAddress->Address.lpSockaddr;
                    Interface.ipv4Addresses.emplace_back(std::make_shared<IPv4Address>(sockaddrIn));
                } else if (pSockAddr->sa_family == AF_INET6) {
                    sockaddr_in6 sockaddrIn6 = *(sockaddr_in6 *) unicastAddress->Address.lpSockaddr;
                    Interface.ipv6Addresses.emplace_back(std::make_shared<IPv6Address>(sockaddrIn6));
                }
                unicastAddress = unicastAddress->Next;
            }

            memcpy(Interface.mac.data(), pAddresses->PhysicalAddress, 6);

            result.emplace_back(Interface);
            Interface.ipv4Addresses.clear();
            Interface.ipv6Addresses.clear();
        }

        pAddresses = pAddresses->Next;
    }

    return result;
}

#endif

#ifdef SESE_PLATFORM_LINUX

#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <map>

std::vector<sese::NetworkInterface> sese::NetworkUtil::getNetworkInterface() noexcept {
    std::vector<NetworkInterface> interfaces;
    std::map<std::string, NetworkInterface> map;
    struct ifaddrs *address = nullptr;

    // 这些信息仅用于获取网卡名称、 IPv4 和 mac 信息
    // glib 2.3.3 以下不支持使用其获取 IPv6 相关信息
    getifaddrs(&address);

    while (address) {
        if (address->ifa_addr->sa_family == AF_INET) {
            auto iterator = map.find(address->ifa_name);
            sockaddr_in addr = *(sockaddr_in *) (address->ifa_addr);
            if (iterator != map.end()) {
                iterator->second.ipv4Addresses.emplace_back(std::make_shared<IPv4Address>(addr));
            } else {
                auto i = NetworkInterface();
                i.ipv4Addresses.emplace_back(std::make_shared<IPv4Address>(addr));
                map[i.name] = i;
            }
        } else if (address->ifa_addr->sa_family == AF_PACKET) {
            auto iterator = map.find(address->ifa_name);
            if (iterator != map.end()) {
                iterator->second.name = address->ifa_name;
                memcpy(iterator->second.mac.data(), address->ifa_addr, 6);
            } else {
                auto i = NetworkInterface();
                i.name = address->ifa_name;
                memcpy(i.mac.data(), address->ifa_addr, 6);
                map[i.name] = i;
            }
        }

        address = address->ifa_next;
    }

    freeifaddrs(address);

    // 用于获取 IPv6 信息
    FILE *f = fopen("/proc/net/if_inet6", "r");
    if (f != nullptr) {
        int ret, scope, prefix;
        unsigned char ipv6[16];
        char name[IFNAMSIZ];
        char address[INET6_ADDRSTRLEN];
        while (19 ==
               fscanf(f,// NOLINT
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
            if (iterator != map.end()) {
                inet_ntop(AF_INET6, ipv6, address, sizeof(address));
                auto addr = IPv6Address::create(address, 0);
                iterator->second.ipv6Addresses.emplace_back(addr);
            }
        }

        fclose(f);
    }

    // 整合信息
    interfaces.reserve(map.size());
    for (decltype(auto) i: map) {
        interfaces.emplace_back(i.second);
    }

    return interfaces;
}

#endif

#ifdef SESE_PLATFORM_APPLE

#include <map>
#include <ifaddrs.h>
#include <net/if_dl.h>

std::vector<sese::NetworkInterface> sese::NetworkUtil::getNetworkInterface() noexcept {
    std::vector<NetworkInterface> interfaces;
    std::map<std::string, NetworkInterface> map;
    struct ifaddrs *address = nullptr;

    getifaddrs(&address);

    while (address) {
        if (address->ifa_addr->sa_family == AF_INET) {
            auto iterator = map.find(address->ifa_name);
            sockaddr_in addr = *(sockaddr_in *) (address->ifa_addr);
            if (iterator != map.end()) {
                iterator->second.ipv4Addresses.emplace_back(std::make_shared<IPv4Address>(addr));
            } else {
                auto i = NetworkInterface();
                i.ipv4Addresses.emplace_back(std::make_shared<IPv4Address>(addr));
                map[i.name] = i;
            }
        } else if (address->ifa_addr->sa_family == AF_INET6) {
            auto iterator = map.find(address->ifa_name);
            sockaddr_in6 addr = *(sockaddr_in6 *) (address->ifa_addr);
            if (iterator != map.end()) {
                iterator->second.ipv6Addresses.emplace_back(std::make_shared<IPv6Address>(addr));
            } else {
                auto i = NetworkInterface();
                i.ipv6Addresses.emplace_back(std::make_shared<IPv6Address>(addr));
                map[i.name] = i;
            }
        } else if (address->ifa_addr->sa_family == AF_LINK) {
            auto iterator = map.find(address->ifa_name);
            auto ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(address)->ifa_addr);
            if (iterator != map.end()) {
                iterator->second.name = address->ifa_name;
                memcpy(iterator->second.mac.data(), ptr, 6);
            } else {
                auto i = NetworkInterface();
                i.name = address->ifa_name;
                memcpy(i.mac.data(), ptr, 6);
                map[i.name] = i;
            }
        }

        address = address->ifa_next;
    }

    freeifaddrs(address);

    interfaces.reserve(map.size());
    for (decltype(auto) i: map) {
        interfaces.emplace_back(i.second);
    }

    return interfaces;
}

#endif