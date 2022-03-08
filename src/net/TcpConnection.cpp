#include "TcpConnection.hpp"
#include "EventLoop.hpp"
#include <spdlog/spdlog.h>

void Duty::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    spdlog::info("{}->{} is {}", conn->getLocalAddr()->toIpPort(), conn->getPeerAddr()->toIpPort(), (conn->connected() ? "UP" : "DOWN"));
}

void Duty::defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf, Timestamp)
{
    buf->retrieveAll();
}

Duty::TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
    Handler handler, const InetAddr& localaddr, const InetAddr& peeraddr)
    : state_ { State::kConnecting }
    , loop_ { loop }
    , name_ { name }
    , handler_ { handler }
    , socket_ { std::make_unique<Socket>(handler_) }
    , localAddr_ { localaddr }
    , peerAddr_ { peeraddr }
    , channel_ { std::make_unique<Channel>(loop, handler) }
    , highWaterMark_ { 64 * 1024 * 1024 }
    , reading_ { false }
{
    channel_->setReadCallback([this] { this->handleRead(); });
    channel_->setWriteCallback([this] { this->handleWrite(); });
    channel_->setErrorCallback([this] { this->handleError(); });
    channel_->setCloseCallback([this] { this->handleClose(); });

    socket_->setKeepAlive(true);
}

void Duty::TcpConnection::send(const void* data, size_t len)
{
    this->send(std::string_view(static_cast<const char*>(data), len));
}

void Duty::TcpConnection::send(std::string_view message)
{
    if (state_ == State::kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop_->runInLoop([this, message] {
                this->sendInLoop(std::string(message));
            });
        }
    }
}

void Duty::TcpConnection::send(Buffer* buf)
{
    if (state_ == State::kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->Peek(), buf->readable());
            buf->retrieveAll();
        } else {
            loop_->runInLoop([this, buf] {
                this->sendInLoop(buf->retrieveAllAsString());
            });
        }
    }
}

void Duty::TcpConnection::sendInLoop(std::string_view message)
{
    sendInLoop(message.data(), message.size());
}

void Duty::TcpConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remain = len;
    bool faultError { false };
    if (state_ == State::kDisconnected) {
        spdlog::warn("disconnected, give up writing");
        return;
    }

    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readable() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remain = len - nwrote;
            if (remain == 0 && writeCompletecb_) {
                loop_->runInLoop([self = shared_from_this()] {
                    self->writeCompletecb_(self);
                });
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                spdlog::error("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    assert(remain <= len);
    if (!faultError && remain > 0) {
        size_t oldLen = outputBuffer_.readable();
        if (oldLen + remain >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkcb_) {
            loop_->runInLoop([self = shared_from_this(), oldLen, remain] {
                self->highWaterMarkcb_(self, oldLen + remain);
            });
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remain);
        if (!channel_->isWriting()) {
            channel_->enableWrite();
        }
    }
}

void Duty::TcpConnection::shutdown()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWR();
    }
}

void Duty::TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWR();
    }
}

void Duty::TcpConnection::forceClose()
{
    if (state_ == State::kConnected || state_ == State::kDisconnecting) {
        state_ = State::kDisconnecting;
        loop_->runInLoop([self = shared_from_this()] {
            self->forceCloseInLoop();
        });
    }
}

void Duty::TcpConnection::forceCloseWithDelay(double seconds)
{
    if (state_ == State::kConnected || state_ == State::kDisconnecting) {
        state_ = State::kDisconnecting;
        loop_->runAfter([self = shared_from_this()] {
            self->forceClose();
        },
            seconds);
    }
}

void Duty::TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == State::kConnected || state_ == State::kDisconnecting) {
        handleClose();
    }
}

void Duty::TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void Duty::TcpConnection::startRead()
{
    loop_->runInLoop([this] {
        this->startReadInLoop();
    });
}

void Duty::TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading()) {
        channel_->enableRead();
    }
}

void Duty::TcpConnection::stopRead()
{
    loop_->runInLoop([this] {
        this->stopReadInLoop();
    });
}

void Duty::TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading()) {
        channel_->disableRead();
        reading_ = false;
    }
}

void Duty::TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == State::kConnecting);
    state_ = State::kConnected;

    channel_->tie(shared_from_this());
    channel_->enableRead();

    connectioncb_(shared_from_this());
}

void Duty::TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == State::kConnected) {
        state_ = State::kDisconnected;
        channel_->disableAll();

        connectioncb_(shared_from_this());
    }
    channel_->remove();
}

void Duty::TcpConnection::handleRead()
{
    loop_->assertInLoopThread();
    int savedErrno = 0;
    Timestamp receiveTime { now() };
    ssize_t n = inputBuffer_.readHandler(channel_->fd(), &savedErrno);
    if (n > 0) {
        auto from_this = shared_from_this();
        messagecb_(from_this, &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        spdlog::error("TcpConnection::handleRead");
        handleError();
    }
}

void Duty::TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.Peek(), outputBuffer_.readable());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readable() == 0) {
                channel_->disableWrite();
                if (writeCompletecb_) {
                    loop_->runInLoop([self = shared_from_this()] {
                        self->writeCompletecb_(self);
                    });
                }
                if (state_ == State::kDisconnecting) {
                    this->shutdownInLoop();
                }
            }
        } else {
            spdlog::error("TcpConnection::handleWrite");
        }
    } else {
        spdlog::trace("Connection fd = {} is down, no more writing", channel_->fd());
    }
}

void Duty::TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    spdlog::trace("fd = {}", channel_->fd());
    assert(state_ == State::kConnected || state_ == State::kDisconnecting);
    state_ = State::kDisconnected;

    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectioncb_(guardThis);
    closecb_(guardThis);
}

void Duty::TcpConnection::handleError()
{
    int error {};
    socklen_t errLen = sizeof(error);
    int err = ::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &error, &errLen);
    (void)err;
    spdlog::error("TcpConnection::handleError [ {} ] - SO_ERROR {} {}", name_, error, strerror(error));
}