#include "../src/net/ListenSocket.hpp"
#include "../src/net/InetAddr.hpp"
#include <gtest/gtest.h>

TEST(ListenSocket, acceptTest)
{
    chauncy::ListenSocket listenSocket(chauncy::SocketType::IPv4);
    chauncy::IPv4Address addr { 1234 };

    listenSocket.bind(addr);
    listenSocket.listen();

    chauncy::IPv4Address addrNewConn;
    listenSocket.accept(addrNewConn);

    EXPECT_EQ(addrNewConn.addrSize(), sizeof(sockaddr_in));
    std::cerr << addrNewConn.port() << "\n";
    EXPECT_EQ(addrNewConn.ip(), std::string("127.0.0.1"));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}