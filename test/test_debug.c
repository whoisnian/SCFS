/*************************************************************************
    > File Name: test_debug.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月23日 星期日 01时42分17秒
    > Compile: gcc test_debug.c ../src/debugprintf.c ../src/image.c ../src/superblock.c ../src/inode.c ../src/bitmap.c ../src/block.c ../src/scfs.c `pkg-config fuse3 --cflags --libs` -o test_debug -I../include
    > Run: ./test_debug
    > Clean: rm ./test_debug
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definition.h"
#include "scfs.h"
#include "superblock.h"
#include "inode.h"
#include "block.h"

int main(int argc, char *argv[])
{
    char image_path[4096] = "/tmp/test.img";
    if(argc == 2)
    {
        strcpy(image_path, argv[1]);
    }
    else if(argc > 2)
    {
        printf("Too many arguments.\n");
        return -1;
    }
    
    int ret = open_scfs(image_path);
    if(ret != 0) return ret;

    char command[100];
    int id, ok = 1;
    char data[SC_BLOCK_SIZE];
    dir_st dir[15];

    while(ok)
    {
        printf("\n\033[1;32;40m--->>\033[0m ");
        scanf("%s", command);
        if(!strcmp(command, "q"))
        {
            // q: 退出 quit
            ok = 0;
        }
        else if(!strcmp(command, "s"))
        {
            // s: 查看超级块 superblock
            printf("超级块信息：\n");
            superblock_st *superblock = read_superblock();
            debug_superblock(superblock);
            free(superblock);
        }
        else if(!strcmp(command, "i"))
        {
            // i: 查看索引节点 inode
            scanf("%d", &id);
            printf("索引节点信息：\n");
            inode_st *inode = read_inode(id);
            debug_inode(inode);
            free(inode);
        }
        else if(!strcmp(command, "b"))
        {
            // b: 查看数据块 block
            scanf("%d", &id);
            printf("数据块信息：\n");
            read_block(id, data, SC_BLOCK_SIZE);
            for(int i = 0;i < SC_BLOCK_SIZE/16;i++)
            {
                printf("%04x: ", i*16);
                for(int j = 0;j < 8;j++)
                    printf("%02x%02x ", data[i*16+j*2], data[i*16+j*2+1]);
                printf("  ");
                for(int j = 0;j < 8;j++)
                    printf("%c%c ", data[i*16+j*2]>=32?data[i*16+j*2]:46, data[i*16+j*2+1]>=32?data[i*16+j*2+1]:46);
                printf("\n");
            }
        }
        else if(!strcmp(command, "d"))
        {
            // d: 查看目录所在数据块 dir
            scanf("%d", &id);
            printf("目录块信息：\n");
            read_block(id, dir, sizeof(dir_st)*15);
            for(int i = 0;i < 15;i++)
                printf("%2u %s\n", dir[i].inodeid, dir[i].filename);
        }
        else if((!strcmp(command, "h"))||(!strcmp(command, "?")))
        {
            // h/?: 帮助 help
            printf(" s        查看超级块信息\n");
            printf(" i <id>   查看索引节点信息\n");
            printf(" b <id>   查看数据块信息\n");
            printf(" d <id>   查看目录块信息\n");
            printf(" q        退出\n");
            printf(" h/?      查看帮助信息\n");
        }
    }

    close_scfs();
    return 0;
}
