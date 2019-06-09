/*************************************************************************
    > File Name: inode.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月07日 星期五 02时08分03秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "definition.h"
#include "image.h"
#include "superblock.h"
#include "block.h"
#include "inode.h"
#include "bitmap.h"
#include "scfs.h"

int init_scfs(const char *filepath)
{
    int ret;
    // 初始化image
    ret = open_image(filepath);
    if(ret != 0)
        return ret;

    // 初始化superblock
    ret = init_superblock();
    if(ret != 0)
        return ret;
    
    superblock_st *superblock;
    superblock = read_superblock();
    if(superblock == NULL)
        return -1;
    
    superblock->block_free = superblock->block_total - superblock->first_block - 1;
    superblock->inode_free = superblock->inode_total - 1;
    superblock->validbit = 1;
    superblock->last_write = time(NULL);

    ret = write_superblock(superblock);
    if(ret != 0)
        return ret;

    if(superblock != NULL)
        free(superblock);

    // 初始化inode
    ret = init_inode(0);
    if(ret != 0)
        return ret;

    inode_st *inode;
    inode = read_inode(0);
    if(inode == NULL)
        return -1;

    inode->privilege = SC_DEFAULT_DIR_PRIVILEGE;
    inode->user = SC_ROOT_UID;
    inode->group = SC_ROOT_GID;
    inode->size = sizeof(dir_st)*2;
    inode->blocknum = 1;
    inode->linknum = 1;
    inode->atime = time(NULL);
    inode->mtime = time(NULL);
    inode->block_id0[0] = superblock->first_block;

    // xxd test.img | grep 0080b000 可以在 0080b000 一行看到 2e (.)
    // xxd test.img | grep 0080b100 可以在 0080b000 一行看到 2e2e (..)
    dir_st dir[2] = {{0, "."}, {0, ".."}};
    ret = write_block(superblock->first_block, &dir, sizeof(dir_st)*2);
    if(ret != 0)
        return ret;

    ret = write_inode(0, inode);
    if(ret != 0)
        return ret;

    // 初始化inode bitmap
    ret = init_bitmap(superblock->first_inode_bitmap, superblock->first_block_bitmap-1);
    if(ret != 0)
        return ret;
    
    ret = write_bitmap(superblock->first_inode_bitmap, superblock->first_block_bitmap-1, 0, 1);
    if(ret != 0)
        return ret;

    // 初始化block bitmap
    ret = init_bitmap(superblock->first_block_bitmap, superblock->first_block-1);
    if(ret != 0)
        return ret;

    for(unsigned int i = 0;i <= superblock->first_block;i++)
    {
        ret = write_bitmap(superblock->first_block_bitmap, superblock->first_block-1, i, 1);
        if(ret != 0)
            return ret;
    }

    return 0;
}

int open_scfs(const char *filepath)
{
    int ret;
    // 打开image
    ret = open_image(filepath);
    if(ret != 0)
        return ret;

    // 读取superblock
    superblock_st *superblock;
    superblock = read_superblock();
    if(superblock == NULL)
        return -1;
    
    superblock->validbit = 0;
    superblock->last_mount = time(NULL);

    ret = write_superblock(superblock);

    if(superblock != NULL)
        free(superblock);

    return 0;
}

int close_scfs(void)
{
    return close_image();
}

void *sc_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    (void) conn;
    (void) cfg;
    return NULL;
}

int sc_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi)
{
    (void) fi;
    int ret;
    inode_st *inode = NULL;
    inodeid_t inodeid;

    ret = find_inode(path, &inodeid);
    if(ret != 0)
        return -ENOENT;

    inode = read_inode(inodeid);
    if(inode == NULL)
        return -ENOENT;
    
    memset(buf, 0, sizeof(struct stat));
    //buf->st_dev
    buf->st_ino = inodeid;
    buf->st_mode = sc_privilege_to_mode_t(inode->privilege);
    buf->st_nlink = inode->linknum;
    buf->st_uid = inode->user;
    buf->st_gid = inode->group;
    //buf->st_rdev
    buf->st_size = inode->size;
    buf->st_blksize = SC_BLOCK_SIZE;
    buf->st_blocks = inode->blocknum;
    buf->st_atime = inode->atime;
    buf->st_mtime = inode->mtime;
    buf->st_ctime = inode->ctime;

    if(inode != NULL)
        free(inode);
    return 0;
}

mode_t sc_privilege_to_mode_t(unsigned int privilege)
{
    mode_t mode = 0;
    if(privilege & SC_DIR)
        mode = mode | __S_IFDIR;
    else
        mode = mode | __S_IFREG;
    if(privilege & SC_S_LINK)
        mode = mode | __S_IFLNK;
    
    if(privilege & SC_USR_PRIVILEGE_R)
        mode = mode | S_IRUSR;
    if(privilege & SC_USR_PRIVILEGE_W)
        mode = mode | S_IWUSR;
    if(privilege & SC_USR_PRIVILEGE_X)
        mode = mode | S_IXUSR;

    if(privilege & SC_GRP_PRIVILEGE_R)
        mode = mode | S_IRGRP;
    if(privilege & SC_GRP_PRIVILEGE_W)
        mode = mode | S_IWGRP;
    if(privilege & SC_GRP_PRIVILEGE_X)
        mode = mode | S_IXGRP;

    if(privilege & SC_OTH_PRIVILEGE_R)
        mode = mode | S_IROTH;
    if(privilege & SC_OTH_PRIVILEGE_W)
        mode = mode | S_IWOTH;
    if(privilege & SC_OTH_PRIVILEGE_X)
        mode = mode | S_IXOTH;
    
    return mode;
}