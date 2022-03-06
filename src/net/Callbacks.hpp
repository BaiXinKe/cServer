#ifndef CALLBACKS_HPP__
#define CALLBACKS_HPP__

#include "InetAddr.hpp"
#include "Timestamp.hpp"

#include <functional>
#include <memory>

namespace Duty {

using TimerCallback = std::function<void()>;
using NewConnectionCallback = std::function<void(int, const InetAddr&)>;

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
    Buffer* buffer, Timestamp receiveTime);

}

#endif