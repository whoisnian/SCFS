/*************************************************************************
    > File Name: test_superblock.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时53分42秒
    > Compile: gcc test_superblock.c image.c superblock.c -o test_superblock -I../include
    > Run: ./test_superblock
    > Clean: rm ./test_superblock ./test.img
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definition.h"
#include "image.h"
#include "superblock.h"

int main(void)
{
    int ret;
    
    printf("\n[TEST1]\n");
    ret = open_image("test.img");
    if(ret == 0)
        printf("open_image ok\n");
    else
        printf("open_image error\n");

    printf("\n[TEST2]\n");
    ret = init_superblock();
    if(ret == 0)
        printf("init_superblock ok\n");
    else
        printf("init_superblock error\n");

    printf("\n[TEST3]\n");
    superblock_st *res;
    res = read_superblock();
    // image文件大小1024M，包含1个1024bytes的superblock，65536个128bytes的inode，和260085个4096bytes的block。
    // 此时data block区开始的第一个硬盘block编号为2059
    if(res->block_total == SC_BLOCK_COUNT&&res->first_block == 2059&&res->last_write == 0)
        printf("read_superblock1 ok\n");
    else
        printf("read_superblock1 error\n");

    printf("\n[TEST4]\n");
    res->validbit = 0;
    res->last_write = time(NULL);
    ret = write_superblock(res);
    if(ret == 0)
        printf("write_superblock ok\n");
    else
        printf("write_superblock error\n");
    free(res);

    printf("\n[TEST5]\n");
    res = read_superblock();
    if(res->validbit == 0&&res->last_write != 0)
        printf("read_superblock2 ok\n");
    else
        printf("read_superblock2 error\n");

    printf("\n========== super block ==========\n\n");
    debug_superblock(res);
    free(res);

    close_image();
    return 0;
}
