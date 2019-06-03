/*************************************************************************
    > File Name: test_image.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时53分42秒
    > Compile: gcc test_image.c image.c -o test_image -I../include
 ************************************************************************/
#include<stdio.h>
#include"image.h"

int main(void)
{
    int ret = create_image("test.img");
    if(ret != 0)
        printf("create_image error\n");
    else
        printf("create_image ok\n");

    if(!exist_image("test.img"))
        printf("exist_image error\n");
    else
        printf("exist_image ok\n");

    return 0;
}
