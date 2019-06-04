/*************************************************************************
    > File Name: test_image.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时53分42秒
    > Compile: gcc test_image.c image.c -o test_image -I../include
    > Run: ./test_image
    > Clean: rm ./test_image ./test.img
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "image.h"

int main(void)
{
    int ret;

    printf("\n[TEST1]\n");
    ret = write_image(0, "\0", 1);
    if(ret != 0)
        printf("write_image1 ok\n");
    else
        printf("write_image1 error\n");
    
    printf("\n[TEST2]\n");
    ret = open_image("test.img");
    if(ret == 0)
        printf("open_image ok\n");
    else
        printf("open_image error\n");

    printf("\n[TEST3]\n");
    if(exist_image())
        printf("exist_image ok\n");
    else
        printf("exist_image error\n");

    printf("\n[TEST4]\n");
    ret = write_image(0, "hello", 5);
    if(ret == 0)
        printf("write_image2 ok\n");
    else
        printf("write_image2 error\n");

    printf("\n[TEST5]\n");
    char str[10] = "\0";
    ret = read_image(0, str, 5);
    if(ret == 0&&!strcmp(str, "hello"))
        printf("read_image ok\n");
    else
        printf("read_image error\n");

    close_image();
    return 0;
}
