#include "../src/net/InetAddr.hpp"
#include <arpa/inet.h>
#include <gtest/gtest.h>

TEST(InetAddr, init_v4)
{
    chauncy::IPv4Address addr_;
    EXPECT_EQ(addr_.addrSize(), sizeof(sockaddr_in));

    chauncy::IPv4Address addrPort { 1234 };
    EXPECT_EQ(addrPort.port(), 1234);

    chauncy::IPv4Address addrIpPort { "192.168.1.1", 2345 };
    EXPECT_EQ(addrIpPort.ip(), std::string("192.168.1.1"));
    EXPECT_EQ(addrIpPort.port(), 2345);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}