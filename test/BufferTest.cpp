#include "../src/net/Buffer.hpp"
#include <gtest/gtest.h>

TEST(buffer, init_test)
{
    chauncy::Buffer buffer;
    EXPECT_EQ(buffer.readableBytes(), 0);
    EXPECT_EQ(buffer.writeableBytes(), chauncy::Buffer::INIT_BUFFER_SIZE);
}

TEST(buffer, wr_test)
{
    chauncy::Buffer buffer;
    int32_t num32 = 100;
    int64_t num64 = 100;
    buffer.appendInt32(num32);
    EXPECT_EQ(buffer.readableBytes(), sizeof(int32_t));
    buffer.appendInt64(num64);
    EXPECT_EQ(buffer.readableBytes(), sizeof(int64_t) + sizeof(int32_t));

    int32_t retriveNum32 = *((const int32_t*)buffer.peek());
    buffer.retrieveInt32();
    EXPECT_EQ(buffer.readableBytes(), sizeof(int64_t));
    EXPECT_EQ(retriveNum32, num32);

    int64_t retriveNum64 = *((const int64_t*)buffer.peek());
    buffer.retrieveInt64();
    EXPECT_EQ(buffer.readableBytes(), 0);
    EXPECT_EQ(retriveNum64, num64);
}

TEST(buffer, str_test)
{
    chauncy::Buffer buffer;
    std::string str { "Hello World" };
    std::string_view svStr { "Hello World" };

    buffer.appendString(str);
    EXPECT_EQ(buffer.readableBytes(), str.size());
    std::string retrievedStr = buffer.retriveAllAsString();

    buffer.appendStringView(svStr);
    EXPECT_EQ(buffer.readableBytes(), svStr.size());

    std::string retrieveStr = buffer.retriveAllAsString();
    EXPECT_EQ(buffer.readableBytes(), 0);

    EXPECT_EQ(str, retrievedStr);
    EXPECT_EQ(str, retrieveStr);
}

TEST(buffer, find_crlf)
{

    chauncy::Buffer buffer;
    std::string noCRLF { "Hello World" };

    buffer.appendString(noCRLF);
    EXPECT_EQ(buffer.findCRLF(), nullptr);

    std::string hasCRLF { "Hello World\r\n" };
    buffer.appendString(hasCRLF);
    EXPECT_NE(buffer.findCRLF(), nullptr);

    const char* crlf = buffer.findCRLF();
    EXPECT_EQ(*crlf, '\r');
    EXPECT_EQ(*(crlf + 1), '\n');
}

TEST(buffer, big_content)
{
    chauncy::Buffer buffer;
    size_t totalRotate = 10000;
    int64_t buf { 0 };

    for (int i = 0; i < totalRotate; i++) {
        buffer.appendInt64(buf);
    }

    EXPECT_EQ(buffer.readableBytes(), totalRotate * sizeof(int64_t));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}