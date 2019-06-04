/*************************************************************************
    > File Name: test_bitmap.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月05日 星期三 00时54分46秒
    > Compile: gcc test_bitmap.c bitmap.c -o test_bitmap -I../include
    > Run: ./test_bitmap
    > Clean: rm ./test_bitmap
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bitmap.h"

int main(void)
{
    bitmap_st tmp;
    int ret;

    init_bitmap(&tmp);

    printf("\n[TEST1]\n");
    for(int i = 0;i < SC_BLOCK_SIZE*8;i++)
    {
        ret = read_bitmap(&tmp, i);
        if(ret) break;
    }
    if(ret == 0)
        printf("read_bitmap1 ok\n");
    else
        printf("read_bitmap1 error\n");
    
    printf("\n[TEST2]\n");
    ret = write_bitmap(&tmp, 233, 1);
    if(ret == 0)
        printf("write_bitmap1 ok\n");
    else
        printf("write_bitmap1 error\n");

    printf("\n[TEST3]\n");
    ret = read_bitmap(&tmp, 233);
    if(ret == 1)
        printf("read_bitmap2 ok\n");
    else
        printf("read_bitmap2 error\n");

    printf("\n[TEST4]\n");
    write_bitmap(&tmp, 233, 0);
    ret = read_bitmap(&tmp, 233);
    if(ret == 0)
        printf("read_bitmap3 ok\n");
    else
        printf("read_bitmap3 error\n");

    return 0;
}
