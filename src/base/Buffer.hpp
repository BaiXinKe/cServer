#ifndef _cServer_BUFFER_HPP__
#define _cServer_BUFFER_HPP__

#include <vector>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <netdb.h>
#include <string_view>

namespace cServer::base
{

    class Buffer
    {
    public:
        static constexpr std::size_t kCheapPrepend = 8;
        static constexpr std::size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kCheapPrepend + BufferSize), read_pos_{kCheapPrepend}, write_pos_{kCheapPrepend}
        {
            assert(readableBytes() == kCheapPrepend);
            assert(writeableBytes() == kInitialSize);
            assert(prependableBytes() == kCheapPrepend);
        }

        void swap(Buffer &other) noexcept
        {
            this->buffer_.swap(other.buffer_);
            std::swap(read_pos_, other.read_pos_);
            std::swap(write_pos_, other.write_pos_);
        }

        size_t capacity() const noexcept
        {
            return buffer_.size();
        }

        size_t readableBytes() const noexcept
        {
            return write_pos_ - read_pos_;
        }

        size_t writeableBytes() const noexcept
        {
            return capacity() - write_pos_;
        }

        size_t prependableBytes() const noexcept
        {
            return read_pos_;
        }

        const char *begin() const noexcept
        {
            return buffer_.data();
        }

        char *begin() noexcept
        {
            return buffer_.data();
        }

        const char *Peek() const noexcept
        {
            return this->begin() + read_pos_;
        }

        char *Peek() noexcept
        {
            return this->begin() + read_pos_;
        }

        const char *findCRLF() const noexcept
        {
            const char *crlf = std::search(this->Peek(), beginWrite(), kCRLF, kCRLF + 2);
            return crlf == beginWrite() ? nullptr : crlf;
        }

        const char *findCRLF(const char *start) const
        {
            assert(this->Peek() <= start);
            assert(this->beginWrite() >= start);
            const char *crlf = std::search(this->Peek(), start, kCRLF, kCRLF + 2);
            return crlf == start ? nullptr : crlf;
        }

        const char *findEOL() const noexcept
        {
            const void *eol = memchr(this->Peek(), '\n', readableBytes());
            return static_cast<const char *>(eol);
        }

        const char *findEOL(const char *start) const noexcept
        {
            assert(this->Peek() <= start);
            assert(this->beginWrite() >= start);
            const void *eol = memchr(start, '\n', beginWrite() - start);
            return static_cast<const char *>(eol);
        }

        void retrieve(size_t len)
        {
            assert(len <= this->readableBytes());
            if (len < readableBytes())
            {
                read_pos_ += len;
            }
            else
            {
                retrieveAll();
            }
        }

        void retrieveUtil(const char *end)
        {
            assert(end <= beginWrite());
            assert(end >= this->Peek());
            this->retrieve(end - this->Peek());
        }

        void retrieveInt64()
        {
            retrieve(sizeof(int64_t));
        }

        void retrieveInt32()
        {
            retrieve(sizeof(int32_t));
        }

        void retrieveInt16()
        {
            retrieve(sizeof(int16_t));
        }

        void retrieveInt8()
        {
            retrieve(sizeof(int8_t));
        }

        std::string retrieveAsString(std::size_t len)
        {
            assert(len <= readableBytes());
            std::string res{this->Peek(), len};
            retrieve(len);
            return res;
        }

        std::string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        std::string_view toStringView() const
        {
            return std::string_view(this->Peek(), readableBytes());
        }

        void retrieveAll()
        {
            read_pos_ = write_pos_ = kCheapPrepend;
        }

        void append(const char *data, size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data, data + len, this->beginWrite());
            hashWritten(len);
        }

        void ensureWriteableBytes(size_t len)
        {
            if (len > writeableBytes())
            {
                makeSpace(len);
            }
            assert(writeableBytes() <= len);
        }

        void hasWritten(size_t len)
        {
            assert(len <= writeableBytes());
            write_pos_ += len;
        }

        void appendInt8(int8_t num)
        {
            append(static_cast<char *>(&num), sizeof(int8_t));
        }

        void appendInt16(int16_t num)
        {
            num = htobe16(num);
            append(static_cast<char *>(&num), sizeof(int16_t));
        }

        void appendInt32(int32_t num)
        {
            num = htobe32(num);
            append(static_cast<char *>(&num), sizeof(int32_t));
        }

        void appendInt64(int64_t num)
        {
            num = htobe64(num);
            append(static_cast<char *>(&num), sizeof(int64_t));
        }

        void unwrite(size_t len)
        {
            assert(len <= readableBytes());
            write_pos_ -= len;
        }

        int64_t readInt64()
        {
            int64_t result = peekInt64();
            retrieveInt64();
            return result;
        }

        int32_t readInt32()
        {
            int32_t result = peekInt32();
            retrieveInt32();
            return result;
        }

        int16_t readInt16()
        {
            int16_t result = peekInt16();
            retrieveInt16();
            return result;
        }

        int8_t readInt8()
        {
            int8_t result = peekInt8();
            retrieveInt8();
            return result;
        }

        int64_t peekInt64()
        {
            assert(readableBytes() >= sizeof(int64_t));
            int64_t result;
            ::memcpy(&result, this->Peek(), sizeof(int64_t));
            return be64toh(result);
        }

        int32_t peekInt32()
        {
            assert(readableBytes() >= sizeof(int32_t));
            int32_t result;
            ::memcpy(&result, this->Peek(), sizeof(int32_t));
            return be32toh(result);
        }

        int16_t peekInt16()
        {
            assert(readableBytes() >= sizeof(int16_t));
            int16_t result;
            ::memcpy(&result, this->Peek(), sizeof(int16_t));
            return be16toh(result);
        }

        int8_t peekInt8()
        {
            assert(readableBytes() >= sizeof(int8_t));
            int8_t result;
            ::memcpy(&result, this->Peek(), sizeof(int8_t));
            return result;
        }

        void prepend(const void *data, size_t len)
        {
            assert(len <= prependableBytes());
            read_pos_ -= len;
            const char *d = static_cast<const char *>(data);
            std::copy(d, d + len, begin() + read_pos_);
        }

        void prependInt8(int8_t num)
        {
            prepend(&num, sizeof(int8_t));
        }

        void prependInt16(int16_t num)
        {
            num = htobe16(num);
            prepend(&num, sizeof(int16_t));
        }

        void prependInt32(int32_t num)
        {
            num = htobe32(num);
            prepend(&num, sizeof(int32_t));
        }

        void prependInt64(int64_t num)
        {
            num = htobe64(num);
            prepend(&num, sizeof(int64_t));
        }

        void shrink(size_t reserve)
        {
            size_t readable = readableBytes();
            std::copy(begin() + read_pos_, begin() + write_pos_, begin() + kCheapPrepend);
            read_pos_ = kCheapPrepend;
            write_pos_ = kCheapPrepend + readable;
            buffer_.resize(readableBytes() + kCheapPrepend);
        }

        ssize_t readFd(int fd, int *savedErrno);

    private:
        const char *beginWrite() const noexcept
        {
            return this->begin() + write_pos_;
        }

        char *beginWrite() noexcept
        {
            return this->begin() + write_pos_;
        }

        void makeSpace(size_t len)
        {
            if (writeableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                buffer_.resize(write_pos_ + len);
            }
            else
            {
                assert(kCheapPrepend < read_pos_);
                size_t readable = readableBytes();
                std::copy(begin() + read_pos_, begin() + write_pos_, begin() + kCheapPrepend);
                read_pos_ = kCheapPrepend;
                write_pos_ = read_pos_ + readable;
                assert(readable == readableBytes());
            }
        }

        size_t read_pos_;
        size_t write_pos_;

        std::vector<char> buffer_;
        static const char kCRLF[];
    };

}
#endif