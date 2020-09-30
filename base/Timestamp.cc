#include <inttypes.h>
#include "Timestamp.h"

using namespace nut;
Timestamp::Timestamp()
    :microSecondsSinceEpoch_(0)
{
}

Timestamp::Timestamp(int64_t microSecondsSinceEpochArg)
    :microSecondsSinceEpoch_(microSecondsSinceEpochArg)
{
}
Timestamp::~Timestamp()
{
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds *kMicroSecondsPerSeconds + tv.tv_usec);
}

std::string Timestamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSeconds;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSeconds;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}