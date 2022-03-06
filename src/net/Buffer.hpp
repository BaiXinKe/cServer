#ifndef BUFFER_HPP__
#define BUFFER_HPP__

#include <string>
#include <vector>



#include "Handler.hpp"

namespace Duty {

class Buffer {
public:
    static constexpr size_t INIT_PREPEND_SIZE { 64 };
    static constexpr size_t DEFAULT_INIT_BUFFER_SIZE { 256 };
    static constexpr double INCREASE_RATE { 1.8 };
    static constexpr const char CRLF[] { "\r\n" };

    Buffer(size_t init_size = DEFAULT_INIT_BUFFER_SIZE);
    ~Buffer() = default;

    const char* findCRLF() const;

    const char* Peek() const { return &data_[read_index_]; }
    char* Peek() { return &data_[read_index_]; }

    void append(const void* buf, size_t size);

    void appendInt8(int8_t val);
    void appendInt16(int16_t val);
    void appendInt32(int32_t val);
    void appendInt64(int64_t val);

    void retrieve(size_t size);
    void retrieveInt8();
    void retrieveInt16();
    void retrieveInt32();
    void retrieveInt64();
    void retrieveAll();

    std::string retrieveAsString(size_t size);
    std::string retrieveAllAsString();

    std::string_view retrieveAsStringView();

    int8_t peekInt8() const;
    int16_t peekInt16() const;
    int32_t peekInt32() const;
    int64_t peekInt64() const;

    int8_t readInt8();
    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();

    size_t readable() const { return write_index_ - read_index_; }
    size_t prependSize() const { return read_index_; }

    ssize_t readHandler(Handler handler, int* savedErrno);

private:
    char* begin() { return &data_[0] + read_index_; }
    const char* begin() const { return &data_[0] + read_index_; }
    char* end() { return &data_[0] + write_index_; }
    const char* end() const { return &data_[0] + write_index_; }

private:
    size_t writeable() const { return data_.size() - write_index_; }
    void expandSpace(size_t size);

private:
    std::vector<char> data_;

    size_t read_index_;
    size_t write_index_;

    size_t capacity_;
};

}

#endif