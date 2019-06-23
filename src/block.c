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
    return write_image(SC_FIRST_BLOCK_SECTOR+blockid, buf, len);
}

int read_block(blockid_t blockid, void *buf, int len)
{
    return read_image(SC_FIRST_BLOCK_SECTOR+blockid, buf, len);
}

blockid_t new_block(void)
{
    int blockid = new_bitmap(SC_FIRST_BLOCK_BITMAP_SECTOR, SC_FIRST_BLOCK_SECTOR-1);
    if(blockid<0)return -1;
    write_bitmap(SC_FIRST_BLOCK_BITMAP_SECTOR, SC_FIRST_BLOCK_SECTOR-1, blockid, 1);
    dec_block_free();
    return blockid;
}

void free_block(blockid_t blockid)
{
    int bitmap_status=read_bitmap(SC_FIRST_BLOCK_BITMAP_SECTOR,SC_FIRST_BLOCK_SECTOR-1,blockid);
    if(bitmap_status==1)
    {
        write_bitmap(SC_FIRST_BLOCK_BITMAP_SECTOR, SC_FIRST_BLOCK_SECTOR-1, blockid, 0);
        inc_block_free();
    }
}
