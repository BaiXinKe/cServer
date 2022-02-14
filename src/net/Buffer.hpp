#ifndef _BUFFER_HPP
#define _BUFFER_HPP
#include <string>
#include <vector>

namespace chauncy {

class Buffer {
public:
    static constexpr size_t INIT_BUFFER_SIZE { 8192 };
    static constexpr size_t PREPEND_SIZE { 8 };

    explicit Buffer(size_t init_buffer_size = INIT_BUFFER_SIZE);

    ~Buffer() = default;

    const char* peek() const { return &buffer_[0] + readIndex_; }

    const char* findCRLF() const;

    void retrieve(size_t len);
    void retrieveInt32();
    void retrieveInt64();
    void retrieveAll();
    std::string retriveAllAsString();

    void append(const void* buf, size_t len);
    void appendInt32(int32_t num);
    void appendInt64(int64_t num);
    void appendString(const std::string& str);
    void appendStringView(std::string_view);

    size_t readableBytes() const
    {
        return writeIndex_ - readIndex_;
    }
    size_t prependBytes() const { return readIndex_ - prependIndex_; }
    size_t writeableBytes() const { return buffer_.size() - writeIndex_; }

private:
    char* begin() { return &buffer_[0] + readIndex_; }
    char* end() { return &buffer_[0] + writeIndex_; }

    const char* begin() const { return &buffer_[0] + readIndex_; }
    const char* end() const { return &buffer_[0] + writeIndex_; }

    void expandSpace();

private:
    std::vector<char> buffer_;
    size_t prependIndex_;
    size_t readIndex_;
    size_t writeIndex_;
};
}

#endif