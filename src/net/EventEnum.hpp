#ifndef EVENT_ENUM_HPP__
#define EVENT_ENUM_HPP__

#include <sys/poll.h>

enum EventType {
    IN = POLLIN | POLLRDNORM | POLLPRI,
    OUT = POLLOUT | POLLWRNORM,
    ERR = POLLERR | POLLHUP
};

#endif