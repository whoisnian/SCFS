/*************************************************************************
    > File Name: superblock.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月06日 星期四 01时02分26秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "superblock.h"

int init_superblock(void)
{
    superblock_st temp;
    temp.block_total = SC_BLOCK_COUNT;
    temp.block_left = SC_BLOCK_COUNT;
    temp.inode_total = SC_INODE_COUNT;
    temp.inode_left = SC_INODE_COUNT;

    temp.block_size = SC_BLOCK_SIZE;
    temp.inode_size = SC_INODE_SIZE;

    temp.create_time = time(NULL);
    temp.last_mount = 0;
    temp.last_write = 0;

    temp.first_inode = 1;
    temp.first_inode_bitmap = 1+temp.inode_total*temp.inode_size/temp.block_size;
    temp.first_block_bitmap = temp.first_inode_bitmap+temp.inode_total/8/temp.block_size;
    temp.first_block = temp.first_block_bitmap+temp.block_total/8/temp.block_size;

    temp.validbit = 1;
    return write_image(0, &temp, sizeof(superblock_st));
}

int write_superblock(superblock_st *superblock)
{
    return write_image(0, superblock, sizeof(superblock_st));
}

superblock_st *read_superblock(void)
{
    superblock_st *res = (superblock_st *)malloc(sizeof(superblock_st));
    if(read_image(0, res, sizeof(superblock_st)) != 0)
        return NULL;
    return res;
}

void debug_superblock(const superblock_st *superblock)
{
    printf("block_total         = %d\n", superblock->block_total);
    printf("block_left          = %d\n", superblock->block_left);
    printf("inode_total         = %d\n", superblock->inode_total);
    printf("inode_left          = %d\n", superblock->inode_left);
    printf("block_size          = %d\n", superblock->block_size);
    printf("inode_size          = %d\n", superblock->inode_size);
    printf("create_time         = %d\n", superblock->create_time);
    printf("last_mount          = %ld\n", superblock->last_mount);
    printf("last_write          = %ld\n", superblock->last_write);
    printf("first_inode         = %d\n", superblock->first_inode);
    printf("first_inode_bitmap  = %d\n", superblock->first_inode_bitmap);
    printf("first_block_bitmap  = %d\n", superblock->first_block_bitmap);
    printf("first_block         = %d\n", superblock->first_block);
    printf("validbit            = %d\n", (superblock->validbit?1:0));
}