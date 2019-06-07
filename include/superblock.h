#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdbool.h>
#include <time.h>

typedef struct superblock_st
{
    unsigned int block_total;           // 4    4
    unsigned int block_free;            // 4    8
    unsigned int inode_total;           // 4    12
    unsigned int inode_free;            // 4    16

    unsigned int block_size;            // 4    20
    unsigned int inode_size;            // 4    24

    time_t create_time;                 // 8    32
    time_t last_mount;                  // 8    40
    time_t last_write;                  // 8    48

    unsigned int first_inode;           // 4    52
    unsigned int first_inode_bitmap;    // 4    56
    unsigned int first_block_bitmap;    // 4    60
    unsigned int first_block;           // 4    64

    bool validbit;                      // 1    65
}superblock_st;

int init_superblock(void);
int write_superblock(superblock_st *superblock);
superblock_st *read_superblock(void);
void debug_superblock(const superblock_st *superblock);

#endif // SUPERBLOCK_H
