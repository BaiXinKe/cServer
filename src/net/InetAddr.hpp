#ifndef INETADDR_HPP__
#define INETADDR_HPP__

#include <arpa/inet.h>
#include <string_view>

namespace chauncy {

enum class IPVersion {
    v4,
    v6
};

class InetAddr {
public:
    explicit InetAddr(IPVersion v);

    virtual const struct sockaddr* getaddr() const = 0;
    virtual struct sockaddr* getaddr() = 0;
    virtual size_t addrSize() const = 0;
    virtual uint16_t port() const = 0;
    virtual std::string ip() const = 0;

    IPVersion version() const;

    virtual ~InetAddr() = default;

private:
    IPVersion version_;
};

class IPv4Address : public InetAddr {
public:
    IPv4Address();
    explicit IPv4Address(uint16_t port);
    IPv4Address(std::string_view ip, uint16_t port);

    virtual const struct sockaddr* getaddr() const override;
    virtual struct sockaddr* getaddr() override;
    virtual size_t addrSize() const override;
    virtual uint16_t port() const override;
    virtual std::string ip() const override;

private:
    sockaddr_in addr_;
};

class IPv6Address : public InetAddr {
public:
    IPv6Address();
    explicit IPv6Address(uint16_t port);
    IPv6Address(std::string_view ip, uint16_t port);

    virtual const struct sockaddr* getaddr() const override;
    virtual struct sockaddr* getaddr() override;
    virtual size_t addrSize() const override;
    virtual uint16_t port() const override;
    virtual std::string ip() const override;

private:
    sockaddr_in6 addr_;
};

}

#endif