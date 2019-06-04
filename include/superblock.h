#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdbool.h>

typedef struct superblock_st
{
    int block_total;
    int block_used;
    int block_left;
    int inode_total;
    int inode_used;
    int inode_left;

    int block_size;
    int inode_size;
    bool validbit;
}superblock_st;

#endif // SUPERBLOCK_H
