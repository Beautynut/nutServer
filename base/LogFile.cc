#include "LogFile.h"

const int fileAppendTimesLimit = 5;

using namespace nut;

Logfile::Logfile(StringArg filename)
    :count(0),
    limit(fileAppendTimesLimit),
    mutex_(new MutexLock)
{
    file_.reset(new AppendFile(filename));
}

Logfile::~Logfile()
{
}

void Logfile::append(const char* message, size_t len)
{
    MutexLockGuard lock(*mutex_);
    file_->append(message,len);
    count++;
    if(count >= limit)
    {
        file_->flush();
        count = 0;
    }
}

void Logfile::flush()
{
    MutexLockGuard lock(*mutex_);
    file_->flush();
}
     