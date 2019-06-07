#ifndef BLOCK_H
#define BLOCK_H

#include <string.h>
#include "image.h"
#include "definition.h"

typedef struct dir_st
{
    inodeid_t inodeid;
    char filename[SC_NAME_MAX];
}dir_st;

int write_block(blockid_t blockid, const void *buf, int len)
{
    return write_image(blockid, buf, len);
}

int read_block(blockid_t blockid, void *buf, int len)
{
    return read_image(blockid, buf, len);
}

#endif // BLOCK_H