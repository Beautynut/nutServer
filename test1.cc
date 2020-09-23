#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

string func()
{
    char str[32] = {0};
    snprintf(str,sizeof(str),"hello world\n");
    return str;
}

int main()
{
    string a = func();
    std::cout<<a;
    return 0;
}