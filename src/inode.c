/*************************************************************************
    > File Name: inode.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月07日 星期五 00时25分26秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "inode.h"

int init_inode(inodeid_t inodeid)
{
    inode_st temp;
    temp.privilege = SC_DEFAULT_FILE_PRIVILEGE;
    temp.user = 0;
    temp.group = 0;
    temp.size = 0;
    temp.flag = 0;
    temp.link = 0;
    temp.ctime = time(NULL);
    temp.atime = 0;
    temp.mtime = 0;
    memset(temp.block_id0, 0, sizeof(temp.block_id0));
    memset(temp.block_id1, 0, sizeof(temp.block_id1));
    temp.block_id2 = 0;
    return write_image(1+inodeid, &temp, sizeof(inode_st));
}

int write_inode(inodeid_t inodeid, inode_st *inode)
{
    return write_image(1+inodeid, inode, sizeof(inode_st));
}

inode_st *read_inode(inodeid_t inodeid)
{
    inode_st *res = (inode_st *)malloc(sizeof(inode_st));
    if(read_image(1+inodeid, res, sizeof(inode_st)) != 0)
        return NULL;
    return res;
}

void debug_inode(const inode_st *inode)
{
    printf("privilege       = %d\n", inode->privilege);
    printf("user            = %d\n", inode->user);
    printf("group           = %d\n", inode->group);
    printf("size            = %d\n", inode->size);
    printf("flag            = %d\n", inode->flag);
    printf("ctime           = %ld\n", inode->ctime);
    printf("atime           = %ld\n", inode->atime);
    printf("mtime           = %ld\n", inode->mtime);
    printf("\n");
    for(int i = 0;i < 16;i++)
        printf("block_id0[%d]%s   = %d\n", i, (i<10?" ":""), inode->block_id0[i]);
    printf("\n");
    for(int i = 0;i < 3;i++)
        printf("block_id1[%d]    = %d\n", i, inode->block_id1[i]);
    printf("\n");
    printf("block_id2       = %d\n", inode->block_id2);
}