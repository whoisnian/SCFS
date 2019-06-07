#ifndef BITMAP_H
#define BITMAP_H

#include <string.h>
#include "definition.h"

typedef struct bitmap_st
{
    char data[SC_BLOCK_SIZE];
}bitmap_st;

int init_bitmap_st(bitmap_st *bitmap);
int write_bitmap_st(bitmap_st *bitmap, int pos, int value);
int read_bitmap_st(bitmap_st *bitmap, int pos);

int init_bitmap(blockid_t inodebitmap_start, blockid_t inodebitmap_end);
int write_bitmap(blockid_t inodebitmap_start, blockid_t inodebitmap_end, int pos, int value);
int read_bitmap(blockid_t inodebitmap_start, blockid_t inodebitmap_end, int pos);

#endif // BITMAP_H