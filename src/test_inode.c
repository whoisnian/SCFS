/*************************************************************************
    > File Name: test_inode.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时53分42秒
    > Compile: gcc test_inode.c image.c inode.c -o test_inode -I../include
    > Run: ./test_inode
    > Clean: rm ./test_inode ./test.img
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definition.h"
#include "image.h"
#include "inode.h"

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
    ret = init_inode(0);
    if(ret == 0)
        printf("init_inode ok\n");
    else
        printf("init_inode error\n");

    printf("\n[TEST3]\n");
    inode_st *res;
    res = read_inode(0);
    if(res->privilege == DEFAULT_FILE_PRIVILEGE&&res->user == 0&&res->atime == 0)
        printf("read_inode1 ok\n");
    else
        printf("read_inode1 error\n");

    printf("\n[TEST4]\n");
    res->privilege = res->privilege&(~SC_USR_PRIVILEGE_ALL);
    res->atime = time(NULL);
    res->mtime = time(NULL);
    init_inode(1);
    ret = write_inode(1, res);
    if(ret == 0)
        printf("write_inode ok\n");
    else
        printf("write_inode error\n");
    free(res);

    printf("\n[TEST5]\n");
    res = read_inode(1);
    if(res->privilege == SC_GRP_PRIVILEGE_R|SC_OTH_PRIVILEGE_R&&res->mtime != 0)
        printf("read_inode2 ok\n");
    else
        printf("read_inode2 error\n");

    printf("\n============ inode 1 ============\n\n");
    debug_inode(res);
    free(res);

    close_image();
    return 0;
}
