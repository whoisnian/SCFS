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
    temp.block_free = SC_BLOCK_COUNT;
    temp.inode_total = SC_INODE_COUNT;
    temp.inode_free = SC_INODE_COUNT;

    temp.block_size = SC_BLOCK_SIZE;
    temp.inode_size = SC_INODE_SIZE;

    temp.create_time = time(NULL);
    temp.last_mount = 0;
    temp.last_write = 0;

    temp.first_inode = SC_FIRST_INODE_SECTOR;
    temp.first_inode_bitmap = SC_FIRST_INODE_BITMAP_SECTOR;
    temp.first_block_bitmap = SC_FIRST_BLOCK_BITMAP_SECTOR;
    temp.first_block = SC_FIRST_BLOCK_SECTOR;

    temp.validbit = true;
    return write_image(0, &temp, sizeof(superblock_st));
}

int write_superblock(superblock_st *superblock)
{
    superblock->last_write=time(NULL);
    return write_image(0, superblock, sizeof(superblock_st));
}

superblock_st *read_superblock(void)
{
    superblock_st *res = (superblock_st *)malloc(sizeof(superblock_st));
    if(read_image(0, res, sizeof(superblock_st)) != 0)
        return NULL;
    return res;
}

unsigned int read_block_free()
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    return res->block_free;
}

unsigned int read_inode_free()
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    return res->inode_free;
}

int write_block_free(unsigned int new_block_free)
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    res->block_free=new_block_free;
    return write_superblock(res);
}

int write_inode_free(unsigned int new_inode_free)
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    res->inode_free=new_inode_free;
    return write_superblock(res);
}

int add_block_free()
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    res->block_free++;
    return write_superblock(res);
}

int add_inode_free()
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    res->inode_free++;
    return write_superblock(res);
}

int dec_block_free()
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    res->block_free--;
    return write_superblock(res);
}

int dec_inode_free()
{
    superblock_st *res = read_superblock();
    if(res==NULL)
        return -1;
    res->inode_free--;
    return write_superblock(res);
}

void debug_superblock(const superblock_st *superblock)
{
    printf("block_total         = %d\n", superblock->block_total);
    printf("block_free          = %d\n", superblock->block_free);
    printf("inode_total         = %d\n", superblock->inode_total);
    printf("inode_free          = %d\n", superblock->inode_free);
    printf("block_size          = %d\n", superblock->block_size);
    printf("inode_size          = %d\n", superblock->inode_size);
    printf("create_time         = %ld\n", superblock->create_time);
    printf("last_mount          = %ld\n", superblock->last_mount);
    printf("last_write          = %ld\n", superblock->last_write);
    printf("first_inode         = %d\n", superblock->first_inode);
    printf("first_inode_bitmap  = %d\n", superblock->first_inode_bitmap);
    printf("first_block_bitmap  = %d\n", superblock->first_block_bitmap);
    printf("first_block         = %d\n", superblock->first_block);
    printf("validbit            = %d\n", (superblock->validbit?1:0));
}
