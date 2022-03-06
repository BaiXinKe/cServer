#ifndef HANDLER_HPP__
#define HANDLER_HPP__

namespace Duty {
#ifdef _WIN32
using Handler = SOCKET;
#else
using Handler = int;
#endif

inline constexpr Handler InvaildHandler { Handler {} };

}

#endif