#include "Error.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>

std::string chauncy::errMsg(std::string_view name)
{
    static char msg[256] {};
    int sizeName = snprintf(msg, sizeof(msg), "%s: ", name.data());
    strerror_r(errno, msg + sizeName, sizeof(msg) - sizeName);
    return std::string(msg);
}