#ifndef _cServer_ADDRESS__
#define _cServer_ADDRESS__

#include <netdb.h>
#include <string_view>
#include <cstring>
#include <arpa/inet.h>
#include <exception>
#include <stdexcept>
#include <memory>

namespace cServer::net
{
    struct IpConvertException : std::exception
    {
        const char *what() const noexcept override
        {
            return "Construct prepare Address class exception, "
                   "because of the inet_pton can not transfer"
                   " the ip address you improved to the address struct.";
        }
    };

    enum class IP
    {
        v4,
        v6
    };

    class Address
    {
    private:
        sockaddr_storage storage_;
        socklen_t addr_size_;
        IP version_;

        sockaddr_in *get_sockaddr_in()
        {
            return reinterpret_cast<sockaddr_in *>(&storage_);
        }

        sockaddr_in6 *get_sockaddr_in6()
        {
            return reinterpret_cast<sockaddr_in6 *>(&storage_);
        }

    public:
        explicit Address(std::string_view address, uint16_t port, IP version = IP::v4)
            : version_{version}
        {
            if (version == IP::v4)
            {
                sockaddr_in *addr_ = get_sockaddr_in();
                addr_->sin_family = AF_INET;
                addr_->sin_port = htons(port);
                if (inet_pton(AF_INET, address.data(), &addr_->sin_addr) == -1)
                {
                    throw IpConvertException();
                }
            }
            else if (version == IP::v6)
            {
                sockaddr_in6 *addr_ = get_sockaddr_in6();
                addr_->sin6_family = AF_INET6;
                addr_->sin6_port = htons(port);
                if (inet_pton(AF_INET6, address.data(), &addr_->sin6_addr) == -1)
                {
                    throw IpConvertException();
                }
            }
            else
            {
                throw std::logic_error{"Not implement this version of IP"};
            }

            this->addr_size_ = (IP::v4 == version) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
        }

        const sockaddr *get_sockaddr() const
        {
            return reinterpret_cast<const sockaddr *>(&this->storage_);
        }

        sockaddr *get_sockaddr()
        {
            return reinterpret_cast<sockaddr *>(&this->storage_);
        }

        socklen_t size() const
        {
            return this->addr_size_;
        }

        Address(Address const &other) = default;
        Address &operator=(Address const &other) = default;
        Address(Address &&) = default;
        Address &operator=(Address &&other) = default;
    };

    using AddressPtr = std::shared_ptr<Address>;
}

#endif