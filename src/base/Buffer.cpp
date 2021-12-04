#include "Buffer.hpp"
#include <sys/uio.h>

const char Buffer::kCRLF[] = {"\r\n"};

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writeableBytes();
    vec[0].iov_base = begin() + write_pos_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        write_pos_ += n;
    }
    else
    {
        write_pos_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}
