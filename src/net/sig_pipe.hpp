#ifndef SIG_PIPE_HPP__
#define SIG_PIPE_HPP__

#include <signal.h>

namespace Duty {

class SigPipeIgnore {
public:
    SigPipeIgnore()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

extern SigPipeIgnore ignore;

}

#endif