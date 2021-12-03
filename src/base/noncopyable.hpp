#ifndef __cServer_NONCOPYABLE__
#define __cServer_NONCOPYABLE__

namespace cServer::base
{
    class noncopyable
    {
    public:
        noncopyable(noncopyable const &other) = delete;
        noncopyable &operator=(noncopyable const &other) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };
} // namespace cServer::base
#endif