#ifndef _CSERVER_CHANNEL_H_
#define _CSERVER_CHANNEL_H_
#include <functional>
#include <sys/epoll.h>

namespace cServer::net {

enum {
    IN_READ = EPOLLIN | EPOLLHUP,
    OUT_WRITE = EPOLLOUT,
    IN_EXPECT = EPOLLERR
};

class Channel {
public:
    using ReadCallback = std::function<void()>;
    using WriteCallback = std::function<void()>;
    using ExceptionCallback = std::function<void()>;

    void setReadCallback(ReadCallback cb)
    {
        readcb_ = std::move(cb);
    }

    void setWriteCallback(WriteCallback cb)
    {
        writecb_ = std::move(cb);
    }

    void setExceptionCallback(ExceptionCallback cb)
    {
        exceptioncb_ = std::move(cb);
    }

    void enableRead()
    {
        event_ |= IN_READ;
    }
    void enableWrite()
    {
        event_ |= OUT_WRITE;
    }
    void enableException()
    {
        event_ |= IN_EXPECT;
    }

    void disableRead()
    {
        event_ &= ~IN_READ;
    }

    void disableWrite()
    {
        event_ &= ~OUT_WRITE;
    }
    void disableException()
    {
        event_ &= ~IN_EXPECT;
    }

    void setRevent(int revent)
    {
        revent_ = revent;
    }

    void resetRevet()
    {
        revent_ = 0;
    }

    int fd() const
    {
        return fd_;
    }

    int event() const
    {
        return event_;
    }

private:
    int fd_;
    int event_;
    int revent_;

    ReadCallback readcb_;
    WriteCallback writecb_;
    ExceptionCallback exceptioncb_;
};

};

#endif