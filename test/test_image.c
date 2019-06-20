/*************************************************************************
    > File Name: test_image.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时53分42秒
    > Compile: gcc test_image.c ../src/debugprintf.c ../src/image.c -o test_image -I../include
    > Run: ./test_image
    > Clean: rm ./test_image ./test.img
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "definition.h"
#include "image.h"

int main(void)
{
    int ret;

    printf("\n[TEST1]\n");
    ret = write_image(0, "\0", 1);
    OK_OR_ERROR(ret != 0)
    
    printf("\n[TEST2]\n");
    ret = open_image("test.img");
    OK_OR_ERROR(ret == 0)

    printf("\n[TEST3]\n");
    OK_OR_ERROR(exist_image())

    printf("\n[TEST4]\n");
    ret = write_image(0, "hello", 5);
    OK_OR_ERROR(ret == 0)

    printf("\n[TEST5]\n");
    char str[10] = "\0";
    ret = read_image(0, str, 5);
    OK_OR_ERROR(ret == 0&&!strcmp(str, "hello"))

    close_image();
    return 0;
}
