#ifndef BLOCK_H
#define BLOCK_H

#include "image.h"
#include "definition.h"

// 目录项
typedef struct dir_st
{
    inodeid_t inodeid;              // 目录所在inode
    char filename[SC_NAME_MAX+1];   // 目录名称
}dir_st;

// 向指定block写入数据
int write_block(blockid_t blockid, const void *buf, int len);

// 从指定block读取数据
int read_block(blockid_t blockid, void *buf, int len);

// 分配一个新的blockid，未找到返回-1
blockid_t new_block(void);

//删除一个block，请务必检查blockid为正数，为负数可能会导致误删
void free_block(blockid_t blockid);

#endif // BLOCK_H
