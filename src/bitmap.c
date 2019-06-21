/*************************************************************************
    > File Name: bitmap.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月05日 星期三 00时26分43秒
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "bitmap.h"
#include "debugprintf.h"

int __init_bitmap_st(bitmap_st *bitmap)
{
    memset(bitmap->data, 0, sizeof(bitmap->data));
    return 0;
}

int __write_bitmap_st(bitmap_st *bitmap, int pos, int value)
{
    if(pos < 0||pos >= sizeof(bitmap->data)*8)
    {
        debug_printf(debug_error, "Write to single bitmap with invalid pos '%d'.\n", pos);
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
    else
    {
        debug_printf(debug_error, "Write to single bitmap with invalid value '%d'.\n", value);
        return -1;
    }
    
    return 0;
}

int __read_bitmap_st(bitmap_st *bitmap, int pos)
{
    if(pos < 0||pos >= sizeof(bitmap->data)*8)
    {
        debug_printf(debug_error, "Write to single bitmap with invalid pos '%d'.\n", pos);
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

int init_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end)
{
    bitmap_st inodebitmap;
    __init_bitmap_st(&inodebitmap);

    int ret;
    for(unsigned int i = inodebitmap_start;i <= inodebitmap_end;i++)
    {
        ret = write_image(i, &inodebitmap, sizeof(bitmap_st));
        if(ret != 0)
            return ret;
    }
    return 0;
}

int write_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end, int pos, int value)
{
    if(pos < 0||pos >= (inodebitmap_end-inodebitmap_start+1)*SC_SECTOR_SIZE*8)
    {
        debug_printf(debug_error, "Write to bitmap with invalid pos '%d'.\n", pos);
        return -1;
    }

    if(value != 0&&value != 1)
    {
        debug_printf(debug_error, "Write to bitmap with invalid value '%d'.\n", value);
        return -1;
    }

    int ret;
    bitmap_st inodebitmap;
    ret = read_image(inodebitmap_start+(pos/(SC_SECTOR_SIZE*8)), &inodebitmap, sizeof(bitmap_st));
    if(ret != 0)
        return ret;

    ret = __write_bitmap_st(&inodebitmap, pos%(SC_SECTOR_SIZE*8), value);
    if(ret != 0)
        return ret;
    
    return write_image(inodebitmap_start+(pos/(SC_SECTOR_SIZE*8)), &inodebitmap, sizeof(bitmap_st));
}

int read_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end, int pos)
{
    if(pos < 0||pos >= (inodebitmap_end-inodebitmap_start+1)*SC_SECTOR_SIZE*8)
    {
        debug_printf(debug_error, "Write to bitmap with invalid pos '%d'.\n", pos);
        return -1;
    }

    int ret;
    bitmap_st inodebitmap;
    ret = read_image(inodebitmap_start+(pos/(SC_SECTOR_SIZE*8)), &inodebitmap, sizeof(bitmap_st));
    if(ret != 0)
        return ret;

    return __read_bitmap_st(&inodebitmap, pos%(SC_SECTOR_SIZE*8));
}

int new_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end)
{
    // todo: 使用 unsigned long 优化：一次比较 unsigned long 长度的位，发现空位时再详细寻找
    // 假设1G大小，块大小为4K，则共2^30/2^12=2^18=262144个块，每个块对应一位，unsigned long 长度为64位，则共相当于262144/64=4096个无符号长整形
    int pos, ret;
    for(pos = 0;pos < (inodebitmap_end-inodebitmap_start+1)*SC_SECTOR_SIZE*8;pos++)
    {
        ret = read_bitmap(inodebitmap_start, inodebitmap_end, pos);
        if(ret == 0)
            return pos;
    }
    return -1;
}
