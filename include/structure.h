#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include "definition.h"

typedef struct superblock_fs
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
}superblock_fs;

typedef struct inode_fs
{
    int privilege;
    int user;
    int group;
    int size;
    int ctime;
    int atime;
    int mtime;
    int block_fs_num;
}inode_fs;

typedef struct block_fs
{
    char data[SC_BLOCK_SIZE];
}block_fs;

#endif // FS_H
