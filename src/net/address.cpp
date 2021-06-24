#include "address.hpp"
#include <string_view>
#include <memory>
#include <cstring>
#include <arpa/inet.h>

using namespace httpServer;

socketAddress::socketAddress(std::string_view ip, uint16_t port, socketFamily sock_family)
    :ip_{ip}, port_{port}, family{sock_family}
{
    bzero(&sa, sizeof(sa));
    if(this->family == socketFamily::IPv4){
        init_ipv4();
    }else {
        init_ipv6();
    }
}

inline void 
socketAddress::init_ipv4(){
    sockaddr_in* net = get_sockaddr_in_ptr();
    net->sin_family = AF_INET;
    net->sin_port = htons(port_);
    if(ip_.empty() || ip_ == "0.0.0.0"){
        net->sin_addr.s_addr = htonl(INADDR_ANY);
    }else{
        inet_pton(AF_INET, ip_.c_str(), &net->sin_addr);
    }
}

inline void 
socketAddress::init_ipv6(){
    sockaddr_in6* net = get_sockaddr_in6_ptr();
    net->sin6_family = AF_INET6;
    net->sin6_port = htons(port_);
    if(ip_.empty() || ip_ == "::0"){
        net->sin6_addr = IN6ADDR_ANY_INIT;
    }else{
        inet_pton(AF_INET6, ip_.c_str(), &net->sin6_addr);
    }
}

inline std::string
socketAddress::ip() const{
    return ip_;
}

inline uint16_t
socketAddress::port() const{
    return port_;
}


inline sockaddr*
socketAddress::get_sockaddr(){
    return reinterpret_cast<sockaddr*>(&sa);
}

inline sockaddr_in* 
socketAddress::get_sockaddr_in_ptr(){
    return reinterpret_cast<sockaddr_in*>(&sa);
}

inline sockaddr_in6* 
socketAddress::get_sockaddr_in6_ptr(){
    return reinterpret_cast<sockaddr_in6*>(&sa);
}

socketAddress::~socketAddress(){}

#ifdef TEST_ADDRESS_UNIT
#include <gtest/gtest.h>

TEST(addressUnit, set_a_test){
    socketAddressPtr saPtr = std::make_shared<socketAddress>("", 8898, socketFamily::IPv4);
    sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(8898);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    
    EXPECT_EQ("", saPtr->ip());
    EXPECT_EQ(8898, saPtr->port());
    
    int cmpRes = memcmp(&sa, saPtr->get_sockaddr(), sizeof(sa));
    EXPECT_EQ(cmpRes, 0);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}



#endif