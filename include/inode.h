#ifndef INODE_H
#define INODE_H

#include <time.h>
#include "definition.h"

typedef struct inode_st
{
    unsigned int privilege;         // 4    4
    unsigned int user;              // 4    8
    unsigned int group;             // 4    12
    unsigned int size;              // 4    16
    unsigned int blocknum;          // 4    20
    unsigned int flag;              // 4    24
    unsigned int linknum;           // 4    28
    time_t ctime;                   // 8    36
    time_t atime;                   // 8    44
    time_t mtime;                   // 8    52
    unsigned int block_id0[16];     // 4*16 116     直接 64K
    unsigned int block_id1[2];      // 4*2  124     间接 8M
    unsigned int block_id2;         // 4    128     双间接 4G
}inode_st;

int init_inode(inodeid_t inodeid);
int write_inode(inodeid_t inodeid, inode_st *inode);
inode_st *read_inode(inodeid_t inodeid);
int find_inode(const char *path, inodeid_t *inodeid);
void debug_inode(const inode_st *inode);

#endif // INODE_H
