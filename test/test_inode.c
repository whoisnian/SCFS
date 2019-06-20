/*************************************************************************
    > File Name: test_inode.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时53分42秒
    > Compile: gcc test_inode.c ../src/debugprintf.c ../src/bitmap.c ../src/image.c ../src/inode.c -o test_inode -I../include
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
    OK_OR_ERROR(ret == 0)

    printf("\n[TEST2]\n");
    ret = init_inode(0);
    OK_OR_ERROR(ret == 0)

    printf("\n[TEST3]\n");
    inode_st *res;
    res = read_inode(0);
    OK_OR_ERROR(res->mode == SC_DEFAULT_FILE&&res->user == 0&&res->atime == 0)

    printf("\n[TEST4]\n");
    res->mode = res->mode&(~SC_USR_ALL);
    res->atime = time(NULL);
    res->mtime = time(NULL);
    init_inode(1);
    ret = write_inode(1, res);
    OK_OR_ERROR(ret == 0)
    free(res);

    printf("\n[TEST5]\n");
    res = read_inode(1);
    OK_OR_ERROR(res->mode == (SC_GRP_R|SC_OTH_R)&&res->mtime != 0)

    printf("\n============ inode 1 ============\n\n");
    debug_inode(res);
    free(res);

    close_image();
    return 0;
}
