/*************************************************************************
    > File Name: block.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月09日 星期日 03时11分14秒
 ************************************************************************/
#include "block.h"
#include "bitmap.h"
#include "superblock.h"

int write_block(blockid_t blockid, const void *buf, int len)
{
    return write_image(blockid, buf, len);
}

int read_block(blockid_t blockid, void *buf, int len)
{
    return read_image(blockid, buf, len);
}

blockid_t new_block(void)
{
    superblock_st *superblock = read_superblock();
    int blockid = new_bitmap(superblock->first_block_bitmap, superblock->first_block-1);
    write_bitmap(superblock->first_block_bitmap, superblock->first_block-1, blockid, 1);
    return blockid;
}
