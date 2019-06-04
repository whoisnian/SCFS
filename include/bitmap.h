#ifndef BITMAP_H
#define BITMAP_H

#include <string.h>
#include "definition.h"

typedef struct bitmap_st
{
    char data[SC_BLOCK_SIZE];
}bitmap_st;

int init_bitmap(bitmap_st *bitmap);
int write_bitmap(bitmap_st *bitmap, int pos, int value);
int read_bitmap(bitmap_st *bitmap, int pos);

#endif // BITMAP_H