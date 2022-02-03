#include "TimerId.hpp"

std::atomic<int64_t> TimerId::globalId { 1 };

TimerId::TimerId()
    : id_ { globalId.fetch_add(1) }
{
}