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

int sc_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    (void) offset;
	(void) fi;
	(void) flags;
    int ret;
    inode_st *cur_inode = NULL;
    inodeid_t cur_inodeid;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    blockid_t cur_blockid;
    dir_st dir[15];
    blockid_t blockid[SC_BLOCK_SIZE/sizeof(blockid_t)];
    for(int i = 0;i < cur_inode->blocknum;i++)
    {
        if(i < 16)
        {
            // inode 的 blockid0 列表，共 16 个直接指向的 block
            cur_blockid = cur_inode->block_id0[i];
            ret = read_block(cur_blockid, &dir, sizeof(dir_st)*15);
            for(int j = 0;(j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15;j++)
            {
                filler(buf, dir[j].filename, NULL, 0, 0);
            }
        }
        else if(i < 2066)
        {
            // inode 的 blockid1 列表，共 2 * 1024 个直接指向的 block，加上中间的 2 个和前面的 16 个
            cur_blockid = cur_inode->block_id1[(i-16)/1025];
            ret = read_block(cur_blockid, &blockid, SC_BLOCK_SIZE);
            if((i-16)%1025 < 1) continue;
            ret = read_block(blockid[(i-16)%1025-1], &dir, sizeof(dir_st)*15);
            for(int j = 0;(j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15;j++)
            {
                filler(buf, dir[j].filename, NULL, 0, 0);
            }
        }
        else if(i < 1051667)
        {
            // inode 的 blockid2 列表，共 1 * 1024 * 1024 个直接指向的 block，加上中间的 1 + 1024 个和前面的 2066 个
            cur_blockid = cur_inode->block_id2;
            ret = read_block(cur_blockid, &blockid, SC_BLOCK_SIZE);
            ret = read_block(blockid[(i-2067)/1025], &blockid, SC_BLOCK_SIZE);
            if((i-2067)%1025 < 1) continue;
            ret = read_block(blockid[(i-2067)%1025-1], &dir, sizeof(dir_st)*15);
            for(int j = 0;(j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15;j++)
            {
                filler(buf, dir[j].filename, NULL, 0, 0);
            }
        }
        else
        {
            return -1;
        }
    }
    printf("sc_readdir %d == %s  ===  %d\n", cur_inodeid, path, cur_inode->blocknum);

    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}

int sc_open(const char *path, struct fuse_file_info *fi)
{
    (void) fi;
    int ret;
    inode_st *cur_inode = NULL;
    inodeid_t cur_inodeid;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}

int sc_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void) fi;
    int ret;
    inode_st *cur_inode = NULL;
    inodeid_t cur_inodeid;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    char temp[1000] = "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26.27.28.29.30";
    long len = cur_inode->size;
	if(offset < len)
    {
		if(offset + size > len)
			size = len - offset;
		memcpy(buf, temp + offset, size);
	}
    else
    {
		size = 0;
    }

    if(cur_inode != NULL)
        free(cur_inode);
    return size;
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