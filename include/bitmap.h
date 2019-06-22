#ifndef BITMAP_H
#define BITMAP_H

#include "definition.h"

//单个bitmap
typedef struct bitmap_st
{
    char data[SC_SECTOR_SIZE];   // 二进制数据
}bitmap_st;

// 初始化单个bitmap
int __init_bitmap_st(bitmap_st *bitmap);

// 向单个bitmap的指定位置写入值
int __write_bitmap_st(bitmap_st *bitmap, int pos, int value);

// 从单个bitmap的指定位置读取值
int __read_bitmap_st(bitmap_st *bitmap, int pos);

// 初始化指定范围内的扇区为bitmap
int init_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end);

// 向指定范围内的bitmap的指定位置写入值
int write_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end, int pos, int value);

// 从指定范围内的bitmap的指定位置读取值
int read_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end, int pos);

// 在指定范围内的bitmap分配一个未使用的位，未找到返回-1
int new_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end);

// 在指定范围内的bitmap分找到未使用的位的数量
int count_bitmap(sectorid_t inodebitmap_start, sectorid_t inodebitmap_end);

#endif // BITMAP_H
