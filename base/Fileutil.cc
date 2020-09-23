#include <stdio.h>
#include "Fileutil.h"

using namespace nut;

AppendFile::AppendFile(StringArg filename)
    :fp_(::fopen(filename.c_str(),"ae"))
{
}

AppendFile::~AppendFile()
{
    ::fclose(fp_);
}

int AppendFile::append(const char* message, size_t len)
{
    size_t n = write(message, len);
    size_t remain = len - n;
    while (remain > 0)
    {
        size_t x = write(message + n, remain);
        if (x == 0)
        {
            int err = ferror(fp_);
            if (err)
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
            }
            break;
        }
        n += x;
        remain = len - n; // remain -= x
    }
}

void AppendFile::flush()
{
    ::fflush(fp_);
}

size_t AppendFile::write(const char* message,size_t len)
{
    return ::fwrite_unlocked(message,1,len,fp_);
}