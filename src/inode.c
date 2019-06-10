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
#include "block.h"

int init_inode(inodeid_t inodeid)
{
    inode_st temp;
    temp.privilege = SC_DEFAULT_FILE_PRIVILEGE;
    temp.user = 0;
    temp.group = 0;
    temp.size = 0;
    temp.blocknum = 0;
    temp.flag = 0;
    temp.linknum = 0;
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

int find_inode(const char *path, inodeid_t *inodeid)
{
    *inodeid = 0;
    
    if(!strcmp(path, "/"))
    {
        return 0;
    }
    else if(path[0] != '/')
    {
        return -1;
    }
    
    char *temppath, *res;
    inode_st *cur_inode;
    blockid_t cur_blockid;
    dir_st dir[15];
    blockid_t blockid[SC_BLOCK_SIZE/sizeof(blockid_t)];
    int ret, ok;
    
    temppath = (char *)malloc(sizeof(char)*(strlen(path)+1));
    strcpy(temppath, path);

    res = strtok(temppath, "/");
    while(res != NULL)
    {
        //printf("debug: %d %s\n", *inodeid, res);
        ok = 1;
        cur_inode = read_inode(*inodeid);
        for(int i = 0;ok&&i < cur_inode->blocknum;i++)
        {
            if(i < 16)
            {
                // inode 的 blockid0 列表，共 16 个直接指向的 block
                cur_blockid = cur_inode->block_id0[i];
                ret = read_block(cur_blockid, &dir, sizeof(dir_st)*15);
            }
            else if(i < 2066)
            {
                // inode 的 blockid1 列表，共 2 * 1024 个直接指向的 block，加上中间的 2 个和前面的 16 个
                cur_blockid = cur_inode->block_id1[(i-16)/1025];
                ret = read_block(cur_blockid, &blockid, SC_BLOCK_SIZE);
                if((i-16)%1025 < 1) continue;
                ret = read_block(blockid[(i-16)%1025-1], &dir, sizeof(dir_st)*15);
            }
            else if(i < 1051667)
            {
                // inode 的 blockid2 列表，共 1 * 1024 * 1024 个直接指向的 block，加上中间的 1 + 1024 个和前面的 2066 个
                cur_blockid = cur_inode->block_id2;
                ret = read_block(cur_blockid, &blockid, SC_BLOCK_SIZE);
                ret = read_block(blockid[(i-2067)/1025], &blockid, SC_BLOCK_SIZE);
                if((i-2067)%1025 < 1) continue;
                ret = read_block(blockid[(i-2067)%1025-1], &dir, sizeof(dir_st)*15);
            }
            else
            {
                return -1;
            }
            
            for(int j = 0;ok&&((j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15);j++)
            {
                if(!strcmp(dir[j].filename, res))
                {
                    free(cur_inode);
                    *inodeid = dir[j].inodeid;
                    cur_inode = read_inode(dir[j].inodeid);
                    ok = 0;
                }
            }
        }
        if(ok) return -1;
        res = strtok(NULL, "/");
    }
    return 0;
}

void debug_inode(const inode_st *inode)
{
    printf("privilege       = %d\n", inode->privilege);
    printf("user            = %d\n", inode->user);
    printf("group           = %d\n", inode->group);
    printf("size            = %d\n", inode->size);
    printf("blocknum        = %d\n", inode->blocknum);
    printf("flag            = %d\n", inode->flag);
    printf("linknum         = %d\n", inode->linknum);
    printf("ctime           = %ld\n", inode->ctime);
    printf("atime           = %ld\n", inode->atime);
    printf("mtime           = %ld\n", inode->mtime);
    printf("\n");
    for(int i = 0;i < 16;i++)
        printf("block_id0[%d]%s   = %d\n", i, (i<10?" ":""), inode->block_id0[i]);
    printf("\n");
    for(int i = 0;i < 2;i++)
        printf("block_id1[%d]    = %d\n", i, inode->block_id1[i]);
    printf("\n");
    printf("block_id2       = %d\n", inode->block_id2);
}
