/*************************************************************************
    > File Name: test_fuse.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月08日 星期六 15时10分42秒
    > Compile: gcc test_fuse.c ../src/image.c ../src/debugprintf.c ../src/superblock.c ../src/inode.c ../src/bitmap.c ../src/block.c ../src/scfs.c `pkg-config fuse3 --cflags --libs` -o test_fuse -I../include
    > Run: ./test_fuse -d ./mnt
    > Clean: rm ./test_fuse /tmp/test.img
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include "definition.h"
#include "scfs.h"

int main(int argc, char *argv[])
{
    int ret;
    // 磁盘文件已存在则不初始化文件系统
    if(access("/tmp/test.img", F_OK) != 0)
    {
        ret = init_scfs("/tmp/test.img");
        OK_OR_ERROR(ret == 0)
    }

    ret = open_scfs("/tmp/test.img");
    OK_OR_ERROR(ret == 0)

    printf("\n");
    
    struct fuse_operations sc_op = {
        .init = sc_init,
        .getattr = sc_getattr,
        .readdir = sc_readdir,
        .open = sc_open,
        .read = sc_read,
        .mkdir = sc_mkdir,
        .create = sc_create,
        .write = sc_write,
        .chmod = sc_chmod,
        .rename = sc_rename,
        .statfs = sc_statfs,
        .unlink = sc_unlink,
        .rmdir = sc_rmdir,
        .access = sc_access,
    };

    ret = fuse_main(argc, argv, &sc_op, NULL);
    OK_OR_ERROR(ret == 0)

    close_scfs();
    return 0;
}
