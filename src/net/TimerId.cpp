#include "TimerId.hpp"

std::atomic<uint64_t> Duty::TimerId::globalId_ { 0 };

Duty::TimerId::TimerId()
    : id_ { globalId_.fetch_add(1) }
{
}
