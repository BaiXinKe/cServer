#include "InetAddr.hpp"
#include "Logger.hpp"
using namespace chauncy;

InetAddr::InetAddr(IPVersion v)
    : version_ { v }
{
}

IPVersion InetAddr::version() const
{
    return version_;
}

IPv4Address::IPv4Address()
    : InetAddr { IPVersion::v4 }
    , addr_ { 0 }
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = 0;
    addr_.sin_addr.s_addr = INADDR_ANY;
}

IPv4Address::IPv4Address(uint16_t port)
    : InetAddr { IPVersion::v4 }
    , addr_ { 0 }
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = INADDR_ANY;
}

IPv4Address::IPv4Address(std::string_view ip, uint16_t port)
    : InetAddr { IPVersion::v4 }
    , addr_ { 0 }
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    if (inet_pton(AF_INET, ip.data(), &addr_.sin_addr) == -1) {
        char errMsg[256] {};
        int size = snprintf(errMsg, sizeof(errMsg), "inet_pton: ");
        strerror_r(errno, errMsg + size, sizeof(errMsg) - size);
        CRITICAL(errMsg);
    }
}

struct sockaddr* IPv4Address::getaddr()
{
    return reinterpret_cast<sockaddr*>(&addr_);
}

const struct sockaddr* IPv4Address::getaddr() const
{
    return reinterpret_cast<const sockaddr*>(&addr_);
}

size_t IPv4Address::addrSize() const
{
    return sizeof(addr_);
}

uint16_t IPv4Address::port() const
{
    return ::ntohs(addr_.sin_port);
}

std::string IPv4Address::ip() const
{
    char ipstr[INET_ADDRSTRLEN] {};
    if (inet_ntop(AF_INET, &addr_.sin_addr, ipstr, INET_ADDRSTRLEN) == nullptr) {
        char errMsg[256] {};
        int size = snprintf(errMsg, sizeof(errMsg), "inet_ntop: ");
        strerror_r(errno, errMsg + size, sizeof(errMsg) - size);
        CRITICAL(errMsg);
    }

    return std::string(ipstr);
}

IPv6Address::IPv6Address()
    : InetAddr { IPVersion::v6 }
    , addr_ { 0 }
{
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = 0;
    addr_.sin6_addr = in6addr_any;
}

IPv6Address::IPv6Address(uint16_t port)
    : InetAddr { IPVersion::v6 }
    , addr_ { 0 }
{
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = htons(port);
    addr_.sin6_addr = in6addr_any;
}

IPv6Address::IPv6Address(std::string_view ip, uint16_t port)
    : InetAddr { IPVersion::v6 }
    , addr_ { 0 }
{
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = ::htons(port);
    if (inet_pton(AF_INET6, ip.data(), &addr_.sin6_addr) == -1) {
        char errMsg[256] {};
        int size = snprintf(errMsg, sizeof(errMsg), "inet_pton: ");
        strerror_r(errno, errMsg + size, sizeof(errMsg) - size);
        CRITICAL(errMsg);
    }
}

size_t IPv6Address::addrSize() const
{
    return sizeof(addr_);
}

struct sockaddr* IPv6Address::getaddr()
{
    return reinterpret_cast<sockaddr*>(&addr_);
}

const struct sockaddr* IPv6Address::getaddr() const
{
    return reinterpret_cast<const sockaddr*>(&addr_);
}
uint16_t IPv6Address::port() const
{
    return ::ntohs(addr_.sin6_port);
}

std::string IPv6Address::ip() const
{
    char ipstr[INET6_ADDRSTRLEN] {};
    if (inet_ntop(AF_INET6, &addr_.sin6_addr, ipstr, INET6_ADDRSTRLEN) == nullptr) {
        char errMsg[256] {};
        int size = snprintf(errMsg, sizeof(errMsg), "inet_ntop: ");
        strerror_r(errno, errMsg + size, sizeof(errMsg) - size);
        CRITICAL(errMsg);
    }

    return std::string(ipstr);
}