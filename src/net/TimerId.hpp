#pragma once

#include <atomic>
#include <inttypes.h>

class TimerId {
public:
    TimerId();

    ~TimerId() = default;

    inline bool operator==(const TimerId& other) const;
    inline bool operator<(const TimerId& other) const;

private:
    static std::atomic<int64_t> globalId;

    int64_t id_;
};

inline bool
TimerId::operator==(const TimerId& other) const
{
    return this->id_ == other.id_;
}

inline bool
TimerId::operator<(const TimerId& other) const
{
    return this->id_ < other.id_;
}
