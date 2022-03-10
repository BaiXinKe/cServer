#include "TcpConnection.hpp"
#include "Buffer.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"

#include <spdlog/spdlog.h>

Duty::TcpConnection::TcpConnection(EventLoop* loop, std::string_view name,
    Handler handler, const InetAddr& localAddr, const InetAddr& peerAddr)
    : state_ { State::Connecting }
    , loop_ { loop }
    , name_ { name }
    , handler_ { handler }
    , channel_ { loop, handler }
    , socket_ { std::make_unique<Socket>(handler_) }
    , localAddr_ { localAddr }
    , peerAddr_ { peerAddr }
    , connectioncb_ {}
    , messagecb_ {}
    , writeCompletecb_ {}
    , closecb_ {}
    , highWaterMark_ { 16 * 1024 * 1024 }
    , highWaterMarkcb_ {}
    , inputBuffer_ { std::make_unique<Buffer>() }
    , outputBuffer_ { std::make_unique<Buffer>() }

{
    loop_->runInLoop([this] { this->channel_.setReadCallback([this] { this->handleRead(); }); });
    loop_->runInLoop([this] { this->channel_.setWriteCallback([this] { this->handleWrite(); }); });
    loop_->runInLoop([this] { this->channel_.setErrorCallback([this] { this->handleError(); }); });
    loop_->runInLoop([this] { this->channel_.setCloseCallback([this] { this->handleClose(); }); });

    socket_->setKeepAlive(true);
}

void Duty::TcpConnection::handleRead()
{
    int savedErrno {};
    ssize_t readBytes = inputBuffer_->readHandler(this->handler_, &savedErrno);

    Timestamp curr { now() };
    if (readBytes > 0) {
        if (messagecb_) {
            this->messagecb_(shared_from_this(), inputBuffer_.get(), curr);
        }
    } else if (readBytes == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        spdlog::error("TcpConnection handeRead (from inputBuffer->ReadHandler)");
        handleError();
    }
}

void Duty::TcpConnection::handleError()
{
    int error {};
    socklen_t error_length { sizeof(error) };
    int ret = ::getsockopt(this->handler_, SOL_SOCKET, SO_ERROR, &error, &error_length);
    assert(ret >= 0);
    spdlog::error("Error due to :[{}]", strerror(error));
}

void Duty::TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    spdlog::trace("fd = {}", this->handler_);
    assert(state_ == State::Connected || state_ == State::Disconnecting);
    state_ = State::Disconnected;

    channel_.disableAll();

    TcpConnectionPtr guardThis { shared_from_this() };
    connectioncb_(guardThis);
    closecb_(guardThis);
}

void Duty::TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();

    ssize_t nwriten {};
    if (channel_.isWriting()) {
        nwriten = ::write(this->handler_, outputBuffer_->Peek(), outputBuffer_->readable());
        if (nwriten > 0) {
            outputBuffer_->retrieve(nwriten);
            if (outputBuffer_->readable() == 0) {
                channel_.disableWrite();
                if (writeCompletecb_) {
                    this->writeCompletecb_(shared_from_this());
                }
            }

            if (state_ == State::Disconnecting) {
                this->shutdownInLoop();
            }
        } else {
            spdlog::error("TcpConnection::handleWrite: [{}]", strerror(errno));
        }
    } else {
        spdlog::trace("Connection fd = {} is down, no more writing", channel_.fd());
    }
}

void Duty::TcpConnection::shutdown()
{
    if (state_ == State::Connected) {
        state_ = State::Disconnecting;

        this->loop_->runInLoop([self = shared_from_this()] { self->shutdownInLoop(); });
    }
}

void Duty::TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_.isWriting()) {
        socket_->shutdownWR();
    }
}

void Duty::TcpConnection::forceClose()
{
    if (state_ == State::Connected || state_ == State::Disconnecting) {
        state_ = State::Disconnecting;
        this->loop_->runInLoop([self = shared_from_this()] { self->forceCloseInLoop(); });
    }
}

void Duty::TcpConnection::forceCloseWithDelay(double seconds)
{
    if (state_ == State::Connected || state_ == State::Disconnecting) {
        state_ = State::Disconnecting;
        this->loop_->runAfter([self = shared_from_this()] { self->forceCloseInLoop(); }, seconds);
    }
}

void Duty::TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == State::Connected || state_ == State::Disconnecting) {
        handleClose();
    }
}

void Duty::TcpConnection::setTcpNoDelay(bool on)
{
    this->socket_->setTcpNoDelay(on);
}

void Duty::TcpConnection::startRead()
{
    this->loop_->runInLoop([self = shared_from_this()] { self->startReadInLoop(); });
}

void Duty::TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_.isReading()) {
        channel_.enableRead();
    }
}

void Duty::TcpConnection::stopRead()
{
    this->loop_->runInLoop([self = shared_from_this()] { self->stopReadInLoop(); });
}

void Duty::TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (channel_.isReading()) {
        channel_.disableRead();
    }
}

void Duty::TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == State::Connecting);
    state_ = State::Connected;

    auto ptr = shared_from_this();

    channel_.tie(ptr);
    channel_.enableRead();

    connectioncb_(ptr);
}

void Duty::TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    auto ptr = shared_from_this();
    if (state_ == State::Connected) {
        state_ = State::Disconnected;
        channel_.disableAll();

        connectioncb_(ptr);
    }
    channel_.remove();
}

void Duty::TcpConnection::send(const void* data, size_t len)
{
    send(std::string_view(reinterpret_cast<const char*>(data), len));
}

void Duty::TcpConnection::send(std::string_view data)
{
    if (state_ == State::Connected) {
        if (loop_->isInLoopThread())
            sendInLoop(data);
        else
            loop_->runInLoop([self = shared_from_this(), data] { self->sendInLoop(data); });
    }
}

void Duty::TcpConnection::send(Buffer* buff)
{
    if (state_ == State::Connected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buff->Peek(), buff->readable());
            buff->retrieveAll();
        } else {
            loop_->runInLoop([self = shared_from_this(), buff] {
                self->sendInLoop(buff->Peek(), buff->readable());
                buff->retrieveAll();
            });
        }
    }
}

void Duty::TcpConnection::sendInLoop(std::string_view message)
{
    loop_->assertInLoopThread();
    if (state_ == State::Connected) {
        sendInLoop(message.data(), message.size());
    }
}

void Duty::TcpConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrite { 0 };
    size_t remain { 0 };
    bool faultError { false };

    if (state_ == State::Disconnected) {
        spdlog::warn("disconnected, give up writing");
        return;
    }

    if (!channel_.isWriting() && outputBuffer_->readable() == 0) {
        nwrite = ::write(this->handler_, data, len);
        if (nwrite >= 0) {
            remain = len - nwrite;
            if (remain == 0 && writeCompletecb_)
                writeCompletecb_(shared_from_this());
        } else {
            nwrite = 0;
            if (errno != EWOULDBLOCK) {
                spdlog::error("TcpConnection::spdlogInLoop: {}", strerror(errno));
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    assert(remain <= len);
    if (!faultError && remain > 0) {
        size_t oldLen = outputBuffer_->readable();
        if (oldLen + remain >= highWaterMark_ && highWaterMarkcb_) {
            highWaterMarkcb_(shared_from_this(), oldLen + remain);
        }

        outputBuffer_->append(static_cast<const char*>(data) + nwrite, remain);
        if (!channel_.isWriting())
            channel_.enableWrite();
    }
}

Duty::TcpConnection::~TcpConnection()
{
    spdlog::info("TcpConnection::dtor[{}] at fd= {} ", name_, channel_.fd());
    assert(state_ == State::Disconnected);
}