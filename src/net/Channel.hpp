#ifndef CHANNEL_HPP__
#define CHANNEL_HPP__

#include "EventEnum.hpp"
#include "Handler.hpp"

#include <atomic>
#include <functional>
#include <memory>

namespace Duty {

class EventLoop;
class TcpConnection;

class Channel {
public:
    using ReadCallback = std::function<void()>;
    using WriteCallback = std::function<void()>;
    using ErrorCallback = std::function<void()>;
    using ClosedCallback = std::function<void()>;

public:
    explicit Channel(EventLoop* loop, Handler fd = InvaildHandler);
    ~Channel();

    void setReadCallback(const ReadCallback& cb) { readcb_ = cb; }
    void setWriteCallback(const WriteCallback& cb) { writecb_ = cb; }
    void setErrorCallback(const ErrorCallback& cb) { errorcb_ = cb; }
    void setCloseCallback(const ClosedCallback& cb) { closecb_ = cb; }

    void setRevent(int revent) { revent_ = revent; }
    int getEvent() const { return event_; }

    int fd() const { return handler_; }

    void enableRead()
    {
        event_ |= IN;
        reading_ = true;
        update();
    }

    void enableWrite()
    {
        event_ |= OUT;
        writing_ = true;
        update();
    }

    void disableRead()
    {
        event_ &= ~IN;
        reading_ = false;
        update();
    }

    void disableWrite()
    {
        event_ &= ~OUT;
        writing_ = false;
        update();
    }

    void disableAll()
    {
        event_ = empty_event_;
        update();
    }

    bool isWriting() const
    {
        return writing_.load();
    }

    bool isReading() const
    {
        return reading_.load();
    }

    void handleEvent();

    void tie(const std::weak_ptr<void>&);
    void remove();

private:
    void update();

private:
    EventLoop* loop_;

    Handler handler_;

    static constexpr int empty_event_ { 0 };

    int event_;
    int revent_;
    std::atomic<bool> eventHanding_;

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadCallback readcb_;
    WriteCallback writecb_;
    ErrorCallback errorcb_;
    ClosedCallback closecb_;

    std::atomic<bool> writing_, reading_;
};
}

#endif