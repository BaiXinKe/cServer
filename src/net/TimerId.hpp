#ifndef TIMERID_HPP__
#define TIMERID_HPP__

#include <atomic>

namespace Duty {

class TimerId {
public:
    TimerId();
    friend bool operator<(const TimerId& left, const TimerId& right);
    friend bool operator==(const Duty::TimerId& left, const TimerId& right);

private:
    static std::atomic<uint64_t> globalId_;
    uint64_t id_;
};

inline bool operator<(const TimerId& left, const TimerId& right)
{
    return left.id_ < right.id_;
}

inline bool operator==(const Duty::TimerId& left, const TimerId& right)
{
    return left.id_ == right.id_;
}

}

#endif