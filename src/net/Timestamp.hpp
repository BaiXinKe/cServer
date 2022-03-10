#ifndef TIMESTAMP_HPP__
#define TIMESTAMP_HPP__

#include <cassert>
#include <chrono>

namespace Duty {
using Timestamp = std::chrono::system_clock::time_point;

inline Timestamp now()
{
    return std::chrono::system_clock::now();
}

inline int getDiffTimestampsMillseconds(Timestamp low, Timestamp high)
{
    assert(low.time_since_epoch() < high.time_since_epoch());
    return std::chrono::duration_cast<std::chrono::milliseconds>(high - low).count();
}

inline int getDiffAtNow(Timestamp ts)
{
    return getDiffTimestampsMillseconds(now(), ts);
}

}

#endif