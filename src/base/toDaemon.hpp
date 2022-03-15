#ifndef TO_DAEMON_HPP__
#define TO_DAEMON_HPP__

namespace Duty {
#define NO_CHDIR 01
#define NO_CLOSE_FILE 02
#define NO_REOPEN_STD_FDS 04

#define NO_UMASK0 010
#define MAX_CLOSE 8192

int to_daemon(int flags);
}

#endif