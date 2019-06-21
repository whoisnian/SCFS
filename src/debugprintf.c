/*************************************************************************
    > File Name: debugprintf.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月19日 星期三 01时32分14秒
 ************************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "debugprintf.h"

void debug_printf(DebugType type, const char *format, ...)
{
    time_t timer;
    time(&timer);
    char nowTimeStr[20];
    strftime(nowTimeStr, 20, "%Y-%m-%d %H:%M:%S", localtime(&timer));

    if(type == debug_info)
        printf("%s \033[1;32;40m[I]\033[0m ", nowTimeStr);
    else if(type == debug_error)
        printf("%s \033[1;31;40m[E]\033[0m ", nowTimeStr);
    else if(type == debug_warn)
        printf("%s \033[1;33;40m[W]\033[0m ", nowTimeStr);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
