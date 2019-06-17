/*************************************************************************
    > File Name: bitmap.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月05日 星期三 00时26分43秒
 ************************************************************************/
#include <stdio.h>
#include "image.h"
#include "bitmap.h"

int init_bitmap_st(bitmap_st *bitmap)
{
    memset(bitmap->data, 0, sizeof(bitmap->data));
    return 0;
}

int write_bitmap_st(bitmap_st *bitmap, int pos, int value)
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

int read_bitmap_st(bitmap_st *bitmap, int pos)
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

int init_bitmap(blockid_t inodebitmap_start, blockid_t inodebitmap_end)
{
    bitmap_st indoebitmap;
    init_bitmap_st(&indoebitmap);

    int ret;
    for(unsigned int i = inodebitmap_start;i <= inodebitmap_end;i++)
    {
        ret = write_image(i, &indoebitmap, sizeof(bitmap_st));
        if(ret != 0)
            return ret;
    }
    return 0;
}

int write_bitmap(blockid_t inodebitmap_start, blockid_t inodebitmap_end, int pos, int value)
{
    if(pos < 0||pos >= (inodebitmap_end-inodebitmap_start+1)*SC_BLOCK_SIZE*8)
    {
        printf("Invalid pos '%d'.\n", pos);
        return -1;
    }

    if(value != 0&&value != 1)
    {
        printf("Invalid value '%d'.\n", value);
        return -1;
    }

    int ret;
    bitmap_st indoebitmap;
    ret = read_image(inodebitmap_start+(pos/(SC_BLOCK_SIZE*8)), &indoebitmap, sizeof(bitmap_st));
    if(ret != 0)
        return ret;

    ret = write_bitmap_st(&indoebitmap, pos%(SC_BLOCK_SIZE*8), value);
    if(ret != 0)
        return ret;
    
    return write_image(inodebitmap_start+(pos/(SC_BLOCK_SIZE*8)), &indoebitmap, sizeof(bitmap_st));
}

int read_bitmap(blockid_t inodebitmap_start, blockid_t inodebitmap_end, int pos)
{
    if(pos < 0||pos >= (inodebitmap_end-inodebitmap_start+1)*SC_BLOCK_SIZE*8)
    {
        printf("Invalid pos '%d'.\n", pos);
        return -1;
    }

    int ret;
    bitmap_st indoebitmap;
    ret = read_image(inodebitmap_start+(pos/(SC_BLOCK_SIZE*8)), &indoebitmap, sizeof(bitmap_st));
    if(ret != 0)
        return ret;

    return read_bitmap_st(&indoebitmap, pos%(SC_BLOCK_SIZE*8));
}

int new_bitmap(blockid_t inodebitmap_start, blockid_t inodebitmap_end)
{
    int pos;
    for(pos = 0;pos < (inodebitmap_end-inodebitmap_start+1)*SC_BLOCK_SIZE*8;pos++)
    {
        int ret = read_bitmap(inodebitmap_start, inodebitmap_end, pos);
        if(ret != 0)
            return ret;
    }
    return -1;
}
