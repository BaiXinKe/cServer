#include "Buffer.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>

#include <sys/uio.h>

constexpr size_t Duty::Buffer::INIT_PREPEND_SIZE;
constexpr size_t Duty::Buffer::DEFAULT_INIT_BUFFER_SIZE;
constexpr double Duty::Buffer::INCREASE_RATE;
constexpr const char Duty::Buffer::CRLF[];

Duty::Buffer::Buffer(size_t init_size)
    : read_index_ { INIT_PREPEND_SIZE }
    , write_index_ { INIT_PREPEND_SIZE }
    , data_(INIT_PREPEND_SIZE + init_size, '\0')
    , capacity_ { init_size }
{
}

const char* Duty::Buffer::findCRLF() const
{
    assert(write_index_ != data_.size());

    const char* crlf = std::search(this->begin(), this->end(), CRLF, CRLF + 2);
    return crlf == this->end() ? nullptr : crlf;
}

void Duty::Buffer::append(const void* buff, size_t size)
{
    const char* buf = reinterpret_cast<const char*>(buff);

    if (writeable() >= size) {
        memcpy(this->end(), buf, size);
        this->write_index_ += size;
        return;
    }

    ::memmove(&data_[0] + INIT_PREPEND_SIZE, begin(), readable());
    write_index_ = readable() + INIT_PREPEND_SIZE;
    read_index_ = INIT_PREPEND_SIZE;

    if (writeable() >= size) {
        append(buf, size);
        return;
    }

    expandSpace(size);
    append(buf, size);
}

void Duty::Buffer::expandSpace(size_t size)
{
    size_t currSize = data_.size();
    size_t newSize = std::max(static_cast<size_t>(currSize * INCREASE_RATE), currSize + size * 2);
    data_.resize(newSize);
}

void Duty::Buffer::appendInt8(int8_t val)
{
    append(&val, sizeof(int8_t));
}

void Duty::Buffer::appendInt16(int16_t val)
{
    val = ::htobe16(val);
    append(&val, sizeof(int16_t));
}

void Duty::Buffer::appendInt32(int32_t val)
{
    val = ::htobe32(val);
    append(&val, sizeof(int32_t));
}

void Duty::Buffer::appendInt64(int64_t val)
{
    val = ::htobe64(val);
    append(&val, sizeof(int64_t));
}

void Duty::Buffer::retrieve(size_t size)
{
    if (readable() < size)
        return;
    read_index_ += size;
    if (read_index_ == write_index_)
        read_index_ = write_index_ = INIT_PREPEND_SIZE;
}

void Duty::Buffer::retrieveInt8()
{
    this->retrieve(sizeof(int8_t));
}

void Duty::Buffer::retrieveInt16()
{
    this->retrieve(sizeof(int16_t));
}

void Duty::Buffer::retrieveInt32()
{
    this->retrieve(sizeof(int32_t));
}

void Duty::Buffer::retrieveInt64()
{
    this->retrieve(sizeof(int64_t));
}

void Duty::Buffer::retrieveAll()
{
    size_t total = readable();
    this->retrieve(total);
}

std::string Duty::Buffer::retrieveAsString(size_t size)
{
    assert(readable() >= size);
    std::string res { Peek(), Peek() + size };
    this->retrieve(size);

    return res;
}

std::string Duty::Buffer::retrieveAllAsString()
{
    std::string res { this->begin(), this->end() };
    this->retrieveAll();
    return res;
}

std::string_view Duty::Buffer::retrieveAsStringView()
{
    return std::string_view(this->Peek(), readable());
}

int8_t Duty::Buffer::peekInt8() const
{
    assert(readable() >= sizeof(int8_t));

    int8_t res {};
    ::memcpy(&res, Peek(), sizeof(int8_t));
    return res;
}

int16_t Duty::Buffer::peekInt16() const
{
    assert(readable() >= sizeof(int16_t));

    int16_t res {};
    ::memcpy(&res, Peek(), sizeof(int16_t));
    return ::be16toh(res);
}

int32_t Duty::Buffer::peekInt32() const
{
    assert(readable() >= sizeof(int32_t));

    int32_t res {};
    ::memcpy(&res, Peek(), sizeof(int32_t));
    return ::be32toh(res);
}

int64_t Duty::Buffer::peekInt64() const
{
    assert(readable() >= sizeof(int64_t));

    int64_t res {};
    ::memcpy(&res, Peek(), sizeof(int64_t));
    return ::be64toh(res);
}

int8_t Duty::Buffer::readInt8()
{
    int8_t res { this->peekInt8() };
    retrieveInt8();
    return res;
}

int16_t Duty::Buffer::readInt16()
{
    int16_t res { this->peekInt16() };
    retrieveInt16();
    return res;
}

int32_t Duty::Buffer::readInt32()
{
    int32_t res { this->peekInt32() };
    retrieveInt32();
    return res;
}

int64_t Duty::Buffer::readInt64()
{
    int64_t res { this->peekInt64() };
    retrieveInt64();
    return res;
}

ssize_t Duty::Buffer::readHandler(Handler handler, int* savedErrno)
{
    constexpr size_t BUF_SIZE { 65535 };

    char buf[BUF_SIZE] {};

    struct iovec iov[2];
    iov[0].iov_base = this->end();
    iov[0].iov_len = writeable();
    iov[1].iov_base = buf;
    iov[1].iov_len = BUF_SIZE;

    int ret = ::readv(handler, iov, 2);
    if (ret == -1) {
        *savedErrno = errno;
    } else if (ret <= writeable()) {
        write_index_ += ret;
        return ret;
    }

    ssize_t totalRead = ret;

    ret -= writeable();
    write_index_ += writeable();
    this->append(buf, ret);

    return totalRead;
}