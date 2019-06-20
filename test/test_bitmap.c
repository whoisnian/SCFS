/*************************************************************************
    > File Name: test_bitmap.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月05日 星期三 00时54分46秒
    > Compile: gcc test_bitmap.c ../src/debugprintf.c ../src/image.c ../src/bitmap.c -o test_bitmap -I../include
    > Run: ./test_bitmap
    > Clean: rm ./test_bitmap
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "definition.h"
#include "bitmap.h"

int main(void)
{
    bitmap_st tmp;
    int ret;

    __init_bitmap_st(&tmp);

    printf("\n[TEST1]\n");
    for(int i = 0;i < SC_BLOCK_SIZE*8;i++)
    {
        ret = __read_bitmap_st(&tmp, i);
        if(ret) break;
    }
    OK_OR_ERROR(ret == 0)
    
    printf("\n[TEST2]\n");
    ret = __write_bitmap_st(&tmp, 233, 1);
    OK_OR_ERROR(ret == 0)

    printf("\n[TEST3]\n");
    ret = __read_bitmap_st(&tmp, 233);
    printf("%d\n", ret);
    OK_OR_ERROR(ret == 1)

    printf("\n[TEST4]\n");
    __write_bitmap_st(&tmp, 233, 0);
    ret = __read_bitmap_st(&tmp, 233);
    OK_OR_ERROR(ret == 0)

    return 0;
}
