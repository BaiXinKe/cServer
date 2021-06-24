#ifndef _HTTPSERVER_SRC_NET_ADDRESS__
#define _HTTPSERVER_SRC_NET_ADDRESS__

#include <memory>
#include <netdb.h>
#include <string_view>
#include <string>

namespace httpServer{
enum class socketFamily: int{
    IPv4 = AF_INET,
    IPv6 = AF_INET6,
};

class socketAddress{
public:
    socketAddress(std::string_view ip, uint16_t port, socketFamily sock_faimly);

    socketAddress(socketAddress const&) = default;
    socketAddress& operator=(socketAddress const&) = default;

    socketAddress(socketAddress&&) = default;
    socketAddress& operator=(socketAddress&&) = default;

    sockaddr* get_sockaddr();

    std::string ip() const;
    uint16_t port() const;

    ~socketAddress();
private:
    void init_ipv4();
    void init_ipv6();

    sockaddr_in* get_sockaddr_in_ptr();
    sockaddr_in6* get_sockaddr_in6_ptr();

private:
    socketFamily family;
    std::string ip_;
    uint16_t port_;
    sockaddr_storage sa;
};

using socketAddressPtr = std::shared_ptr<socketAddress>;
}
#endif