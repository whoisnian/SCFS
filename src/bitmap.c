/*************************************************************************
    > File Name: bitmap.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月05日 星期三 00时26分43秒
 ************************************************************************/
#include <stdio.h>
#include "bitmap.h"

int init_bitmap(bitmap_st *bitmap)
{
    memset(bitmap->data, 0, sizeof(bitmap->data));
    return 0;
}

int write_bitmap(bitmap_st *bitmap, int pos, int value)
{
    if(pos < 0||pos >= sizeof(bitmap->data)*8)
    {
        printf("Invalid pos '%d'.\n", pos);
        return -1;
    }

    if(value != 0&&value != 1)
    {
        printf("Invalid value '%d'.\n", value);
        return -1;
    }

    char ch;
    if(value == 1)
    {
        ch = (1 << (pos%8));
        bitmap->data[pos/8] = bitmap->data[pos/8] | ch;
    }
    else if(value == 0)
    {
        ch = (1 << (pos%8));
        ch = ~ch;
        bitmap->data[pos/8] = bitmap->data[pos/8] & ch;
    }
    return 0;
}

int read_bitmap(bitmap_st *bitmap, int pos)
{
    if(pos < 0||pos >= sizeof(bitmap->data)*8)
    {
        printf("Invalid pos '%d'.\n", pos);
        return -1;
    }

    char ch;
    ch = (1 << (pos%8));
    int res = bitmap->data[pos/8] & ch;

    if(res == 0)
        return 0;
    else
        return 1;
}