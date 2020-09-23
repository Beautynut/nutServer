#ifndef SERVER_LOGFILE_H
#define SREVER_LOGFILE_H

#include <memory>
#include "Fileutil.h"
#include "Mutex.h"
namespace nut
{
class Logfile
{
    public:
        Logfile(StringArg filename);
        ~Logfile();

        void append(const char* message, size_t len);
        void flush();

    private:
        int count;
        const int limit;

        std::unique_ptr<AppendFile> file_;
        std::unique_ptr<MutexLock> mutex_;
    
};
}

#endif