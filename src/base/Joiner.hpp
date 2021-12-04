#ifndef _cServer_JOINER__
#define _cServer_JOINER__

#include <vector>
#include <thread>

namespace cServer::base
{

    class Joiner
    {
    private:
        std::vector<std::thread> &threads_;

    public:
        Joiner(std::vector<std::thread> &threads) : threads_{threads} {}

        ~Joiner()
        {
            for (auto &t : threads_)
            {
                if (t.joinable())
                    t.join();
            }
        }
    };

}
#endif