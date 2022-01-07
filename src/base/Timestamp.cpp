#include "Timestamp.h"
#include <chrono>

namespace cServer::base {

constexpr uint64_t Timestamp::INVALID;

Timestamp Timestamp::now()
{
    using namespace std::chrono;
    auto now = system_clock::now().time_since_epoch();
    auto nowMillseconds = duration_cast<milliseconds>(now).count();
    return Timestamp(static_cast<uint64_t>(nowMillseconds));
}

Timestamp Timestamp::afterSeconds(double seconds)
{
    uint64_t millseconds = static_cast<uint64_t>(seconds * 1000);
    Timestamp now = Timestamp::now();
    now += millseconds;
    return now;
}

Timestamp::Timestamp(uint64_t ts)
    : ts_ { ts }
{
}

}