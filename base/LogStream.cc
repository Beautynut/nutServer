#include "LogStream.h"

using namespace nut;

//zero两边对称，因为余数可能为负数
const char digits[] = "9876543210123456789";
//十六进制时使用
const char digitsHex[] = "0123456789ABCDEF";
const char* zero = digits + 9;

// From muduo
// Efficient Integer to String Conversions, by Matthew Wilson.
template <typename T>
size_t convert(char buf[], T value) {
  T i = value;
  char* p = buf;

  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

//转为十六进制字符串
size_t convertHex(char buf[], uintptr_t value)
{
    uintptr_t i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template class FixedBuffer<SmallBuffer>;
template class FixedBuffer<LargeBuffer>;
template <typename T>
void LogStream::formatInteger(T v) 
{
    if (buffer_.avail() >= kMaxNumericSize) 
    {
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(short v) 
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) 
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) 
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) 
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) 
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) 
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) 
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) 
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(double v) 
{
    if (buffer_.avail() >= kMaxNumericSize) 
    {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(long double v) 
{
    if (buffer_.avail() >= kMaxNumericSize) 
    {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
        buffer_.add(len);
    }
  return *this;
}