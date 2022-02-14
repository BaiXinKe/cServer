#include "Buffer.hpp"
#include <cassert>
#include <cstring>

using namespace chauncy;

constexpr size_t Buffer::INIT_BUFFER_SIZE;
constexpr size_t Buffer::PREPEND_SIZE;

Buffer::Buffer(size_t init_buffer_size)
    : buffer_(init_buffer_size + PREPEND_SIZE, '\0')
    , prependIndex_ { 0 }
    , readIndex_ { PREPEND_SIZE }
    , writeIndex_ { PREPEND_SIZE }
{
    assert(prependIndex_ == 0);
    assert(readIndex_ == writeIndex_);
}

const char* Buffer::findCRLF() const
{
    assert(readableBytes() > 0);
    const char* pos = peek();
    const char* CRLF = strstr(pos, "\r\n");
    return CRLF;
}

void Buffer::retrieve(size_t len)
{
    if (readableBytes() < len)
        return;
    readIndex_ += len;
    if (readIndex_ == writeIndex_) {
        readIndex_ = writeIndex_ = PREPEND_SIZE;
        prependIndex_ = 0;
    }
}

void Buffer::retrieveInt32()
{
    this->retrieve(sizeof(int32_t));
}

void Buffer::retrieveInt64()
{
    this->retrieve(sizeof(int64_t));
}

void Buffer::retrieveAll()
{
    this->readIndex_ = this->writeIndex_ = PREPEND_SIZE;
    prependIndex_ = 0;
}

std::string Buffer::retriveAllAsString()
{
    std::string res(peek(), readableBytes());
    retrieveAll();
    return res;
}

void Buffer::append(const void* buf, size_t len)
{
    if (writeableBytes() >= len) {
        memcpy(&buffer_[0] + writeIndex_, buf, len);
        writeIndex_ += len;
        return;
    }

    size_t totalCanWriteBytes = writeableBytes() + prependBytes() - PREPEND_SIZE;
    if (totalCanWriteBytes >= len) {
        size_t totalBytes = readableBytes();
        memcpy(&buffer_[0] + PREPEND_SIZE, begin(), totalBytes);
        prependIndex_ = 0;
        readIndex_ = PREPEND_SIZE;
        writeIndex_ = PREPEND_SIZE + totalBytes;

        append(buf, len);
    }

    expandSpace();
    append(buf, len);
}

void Buffer::expandSpace()
{
    size_t currSize = buffer_.size();
    size_t expandSize = currSize + 2 * INIT_BUFFER_SIZE;
    buffer_.resize(expandSize);
}

void Buffer::appendInt32(int32_t num)
{
    append((const void*)&num, sizeof(int32_t));
}

void Buffer::appendInt64(int64_t num)
{
    append((const void*)&num, sizeof(int64_t));
}

void Buffer::appendString(const std::string& str)
{
    append(str.data(), str.length());
}

void Buffer::appendStringView(std::string_view sv)
{
    append(sv.data(), sv.length());
}