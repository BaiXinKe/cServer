#ifndef CHANNEL_HPP__
#define CHANNEL_HPP__

#include "EventEnum.hpp"
#include "Handler.hpp"
#include "TcpConnection.hpp"

#include <atomic>
#include <functional>

namespace Duty {

class EventLoop;
class TcpConnection;

class Channel {
public:
    using ReadCallback = std::function<void()>;
    using WriteCallback = std::function<void()>;
    using ErrorCallback = std::function<void()>;
    using ClosedCallback = std::function<void()>;

    enum class State {
        NO_REG, // channel not registe in pollList
        REG, // channel registed in polllist
        DEL // channel need removed form polllist
    };

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

    void enableError()
    {
        event_ |= ERR;
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

    void disableErr()
    {
        event_ &= ~ERR;
        update();
    }

    void disableAll()
    {
        event_ = empty_event_;
        update();
    }

    void removeFromEpollList()
    {
        state_ = State::DEL;
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

    State getState() const { return state_; }
    void setState(State state) { state_ = state; }

    void handleEvent();

    void tie(const std::weak_ptr<TcpConnection>&);
    void remove();

private:
    void update();

private:
    EventLoop* loop_;

    Handler handler_;

    static constexpr int empty_event_ { 0 };

    int event_;
    int revent_;

    std::weak_ptr<TcpConnection> tie_;

    State state_;

    ReadCallback readcb_;
    WriteCallback writecb_;
    ErrorCallback errorcb_;
    ClosedCallback closecb_;

    std::atomic<bool> writing_, reading_;
};

}

#endif