#ifndef EVENT_ENUM_HPP__
#define EVENT_ENUM_HPP__

#include <sys/epoll.h>

enum EventType {
    IN = EPOLLIN | EPOLLPRI,
    OUT = EPOLLOUT
};

#endif