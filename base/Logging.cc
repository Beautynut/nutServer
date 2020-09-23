#include "Logging.h"
#include "Timestamp.h"
#include <errno.h>
#include <string.h>

namespace nut
{
__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno)
{
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

void output(const char* msg, int len)
{

}

Logger::Impl::Impl(const char *fileName, int line)
  : stream_(),
    line_(line),
    basename_(fileName)
{
    formatTime();
}

void Logger::Impl::formatTime()
{
  struct timeval tv;
  time_t time;
  char str_t[26] = {0};
  gettimeofday (&tv, NULL);
  time = tv.tv_sec;
  struct tm* p_time = localtime(&time);   
  strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
  stream_ << str_t;
}

Logger::Logger(const char *fileName, int line)
  : impl_(fileName, line)
{ 
}

Logger::~Logger()
{
  impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
  const LogStream::Buffer& buf(stream().buffer());
  output(buf.data(), buf.length());
}

}