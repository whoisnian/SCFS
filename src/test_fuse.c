/*************************************************************************
    > File Name: test_fuse.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月08日 星期六 15时10分42秒
    > Compile: gcc test_fuse.c image.c superblock.c inode.c bitmap.c scfs.c `pkg-config fuse3 --cflags --libs` -o test_fuse -I../include
    > Run: ./test_fuse
    > Clean: rm ./test_fuse ./test.img
 ************************************************************************/

#include <stdio.h>
#include "scfs.h"

int main(int argc, char *argv[])
{
    int ret;
    
    ret = open_scfs("test.img");
    if(ret == 0)
        printf("open_scfs ok\n");
    else
        printf("open_scfs error\n");

    struct fuse_operations sc_op = {
        .init = sc_init,
        .getattr = sc_getattr,
    };

    ret = fuse_main(argc, argv, &sc_op, NULL);
    if(ret == 0)
        printf("fuse start ok\n");
    else
        printf("fuse start error %d\n", ret);

    //close_scfs();
    return 0;
}