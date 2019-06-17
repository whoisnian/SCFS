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

int write_block(blockid_t blockid, const void *buf, int len);
int read_block(blockid_t blockid, void *buf, int len);
inodeid_t new_block(void);

#endif // BLOCK_H
