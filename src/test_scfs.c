/*************************************************************************
    > File Name: test_scfs.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时53分42秒
    > Compile: gcc test_scfs.c image.c superblock.c inode.c bitmap.c scfs.c `pkg-config fuse3 --cflags --libs` -o test_scfs -I../include
    > Run: ./test_scfs
    > Clean: rm ./test_scfs ./test.img
 ************************************************************************/

#include <stdio.h>
#include "inode.h"
#include "scfs.h"

int main(void)
{
    int ret;
    
    printf("\n[TEST1]\n");
    ret = init_scfs("test.img");
    if(ret == 0)
        printf("init_scfs ok\n");
    else
        printf("init_scfs error\n");

    printf("\n[TEST2]\n");
    ret = open_scfs("test.img");
    if(ret == 0)
        printf("open_scfs ok\n");
    else
        printf("open_scfs error\n");

    close_scfs();
    return 0;
}