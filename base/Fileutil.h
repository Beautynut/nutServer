#ifndef SERVER_FILE_UTIL_H
#define SERVER_FILE_UTIL_H

#include "Logging.h"

#include <string>

namespace nut
{
class StringArg
{
 public:
  StringArg(const char* str)
    : str_(str)
  { }

  StringArg(const std::string& str)
    : str_(str.c_str())
  { }

  const char* c_str() const { return str_; }

 private:
  const char* str_;
};

// 对LogFile再进行一次封装
class AppendFile
{
    public:
        AppendFile(StringArg filename);
        ~AppendFile();

        int append(const char* message,size_t len);
        void flush();
    private:
        size_t write(const char* message,size_t len);
        FILE* fp_;
};

}

#endif