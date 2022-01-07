#ifndef _CSERVER_TIMESTAMP_H_
#define _CSERVER_TIMESTAMP_H_

#include <cstdint>

namespace cServer::base {

class Timestamp {
private:
    uint64_t ts_;

public:
    static constexpr uint64_t INVALID { 0 };

    Timestamp(uint64_t ts = INVALID);
    ~Timestamp() = default;

    static Timestamp now();
    static Timestamp afterSeconds(double second);

    inline bool operator<(Timestamp other);
    inline bool operator<=(Timestamp other);
    inline Timestamp operator+(Timestamp other);
    inline Timestamp operator+(double seconds);
    inline Timestamp operator+(uint64_t millseconds);

    inline Timestamp& operator+=(Timestamp other);
    inline Timestamp& operator+=(double seconds);
    inline Timestamp& operator+=(uint64_t millseconds);

    bool isInvalid() const
    {
        return this->ts_ == INVALID;
    }
};

inline bool
Timestamp::operator<(Timestamp other)
{
    return this->ts_ < other.ts_;
}

inline bool
Timestamp::operator<=(Timestamp other)
{
    return this->ts_ <= other.ts_;
}

inline Timestamp
Timestamp::operator+(Timestamp other)
{
    return Timestamp(this->ts_ + other.ts_);
}

inline Timestamp
Timestamp::operator+(double seconds)
{
    uint64_t millseconds = static_cast<uint64_t>(seconds * 1000);
    return Timestamp(this->ts_ + millseconds);
}

inline Timestamp
Timestamp::operator+(uint64_t millseconds)
{
    return Timestamp(this->ts_ + millseconds);
}

inline Timestamp&
Timestamp::operator+=(Timestamp other)
{
    this->ts_ += other.ts_;
    return *this;
}

inline Timestamp&
Timestamp::operator+=(double seconds)
{
    uint64_t millseconds = static_cast<uint64_t>(seconds * 1000);
    this->ts_ += millseconds;
    return *this;
}

inline Timestamp&
Timestamp::operator+=(uint64_t millseconds)
{
    this->ts_ += millseconds;
    return *this;
}

}

#endif