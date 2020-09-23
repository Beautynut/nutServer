#ifndef SERVER_TIMESTAMP_H
#define SERVER_TIMESTAMP_H

#include <sys/time.h>
#include <string>

namespace nut
{
class Timestamp
{
    public:
        Timestamp();
        explicit Timestamp(int64_t microSecondsSinceEpochArg);
        ~Timestamp();

        Timestamp now();

        std::string Timestamp::toString() const;

        int64_t microSecondsSinceEpoch(){ return microSecondsSinceEpoch_; }

        time_t secondsSinceEpoch() const
        { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSeconds); }

        bool operator==(Timestamp& rhs)
        { return this->microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch(); }

        bool operator<(Timestamp& rhs)
        { return this->microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch(); }
        
        static const int kMicroSecondsPerSeconds = 1000 * 1000;
    private:
        int64_t microSecondsSinceEpoch_;
};

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSeconds);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}

#endif