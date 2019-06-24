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
#include "bitmap.h"
#include "inode.h"
#include "superblock.h"
#include "block.h"
#include "debugprintf.h"

int min(int a, int b)
{
    return (a < b?a:b);
}

int __inode_blockno_to_blockid(const inode_st *inode, unsigned int blockno)
{
    blockid_t cur_blockid;
    blockid_t blockid[SC_BLOCK_SIZE/sizeof(blockid_t)];
    int ret;

    if(blockno < 16)
    {
        // inode 的 blockid0 列表，共 16 个直接指向的 block
        cur_blockid = inode->block_id0[blockno];
    }
    else if(blockno < 2066)
    {
        // inode 的 blockid1 列表，共 2 * 1024 个直接指向的 block，加上中间的 2 个和前面的 16 个
        cur_blockid = inode->block_id1[(blockno-16)/1025];
        ret = read_block(cur_blockid, blockid, SC_BLOCK_SIZE);
        if(ret != 0) return -1;
        if((blockno-16)%1025 < 1) return -2;
        cur_blockid = blockid[(blockno-16)%1025-1];
    }
    else if(blockno < 1051667)
    {
        // inode 的 blockid2 列表，共 1 * 1024 * 1024 个直接指向的 block，加上中间的 1 + 1024 个和前面的 2066 个
        cur_blockid = inode->block_id2;
        ret = read_block(cur_blockid, blockid, SC_BLOCK_SIZE);
        if(ret != 0) return -1;
        ret = read_block(blockid[(blockno-2067)/1025], blockid, SC_BLOCK_SIZE);
        if(ret != 0) return -1;
        if((blockno-2067)%1025 < 1) return -2;
        cur_blockid = blockid[(blockno-2067)%1025-1];
    }
    else
    {
        return -1;
    }
    return cur_blockid;
}

int __inode_add_new_item_to_inode(inodeid_t inodeid, const char *itemname, inodeid_t *inodeidres)
{
    inode_st *inode;
    int res, ret;
    dir_st dir[15];
    blockid_t blockid[SC_BLOCK_SIZE/sizeof(blockid_t)];
    
    *inodeidres = new_inode();
    init_inode(*inodeidres);
    
    inode = read_inode(inodeid);
    if(inode == NULL)
        return -1;
    
    if(inode->size/sizeof(dir_st)%15 != 0)
    {
        // 最后一个block未占满，不需要分配新的block
        res = __inode_blockno_to_blockid(inode, inode->blocknum-1);
        if(res == -1)
            return -1;
        
        ret = read_block(res, dir, sizeof(dir_st)*15);
        if(ret != 0) return -1;
        
        strcpy(dir[inode->size/sizeof(dir_st)%15].filename, itemname);
        dir[inode->size/sizeof(dir_st)%15].inodeid = *inodeidres;
        write_block(res, dir, sizeof(dir_st)*15);

        inode->size = inode->size + sizeof(dir_st);
        write_inode(inodeid, inode);
    }
    else
    {
        // 最后一个block已占满，需要分配新的block
        blockid_t new_blockid = new_block();
        memset(dir, 0, sizeof(dir_st)*15);
        strcpy(dir[0].filename, itemname);
        dir[0].inodeid = *inodeidres;
        write_block(new_blockid, dir, sizeof(dir_st)*15);

        inode->blocknum++;
        if(inode->blocknum <= 16)
        {
            // 直接
            inode->block_id0[inode->blocknum-1] = new_blockid;
        }
        else if((inode->blocknum-17)%1025 < 1)
        {
            // 间接的中间节点
            blockid_t mid_blockid = new_block();
            inode->block_id1[(inode->blocknum-17)/1025] = mid_blockid;
            
            inode->blocknum++;
            memset(blockid, 0, sizeof(blockid));
            blockid[0] = new_blockid;
            write_block(mid_blockid, blockid, SC_BLOCK_SIZE);
        }
        else if(inode->blocknum <= 2066)
        {
            // 间接
            read_block(inode->block_id1[(inode->blocknum-17)/1025], blockid, SC_BLOCK_SIZE);
            blockid[(inode->blocknum-17)%1025-1] = new_blockid;
            write_block(inode->block_id1[(inode->blocknum-17)/1025], blockid, SC_BLOCK_SIZE);
        }
        else if(inode->blocknum == 2067)
        {
            // 双间接第一层中间节点
            blockid_t mid1_blockid = new_block();
            inode->block_id2 = mid1_blockid;
            
            inode->blocknum++;
            memset(blockid, 0, sizeof(blockid));
            blockid_t mid2_blockid = new_block();
            blockid[0] = mid2_blockid;
            write_block(mid1_blockid, blockid, SC_BLOCK_SIZE);

            inode->blocknum++;
            memset(blockid, 0, sizeof(blockid));
            blockid[0] = new_blockid;
            write_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
        }
        else if((inode->blocknum-2068)%1025 < 1)
        {
            // 双间接第二层中间节点
            read_block(inode->block_id2, blockid, SC_BLOCK_SIZE);

            blockid_t mid2_blockid = new_block();
            blockid[(inode->blocknum-2068)/1025] = mid2_blockid;
            write_block(inode->block_id2, blockid, SC_BLOCK_SIZE);

            inode->blocknum++;
            memset(blockid, 0, sizeof(blockid));
            blockid[0] = new_blockid;
            write_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
        }
        else
        {
            // 双间接
            read_block(inode->block_id2, blockid, SC_BLOCK_SIZE);
            blockid_t mid2_blockid = blockid[(inode->blocknum-2068)/1025];

            read_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
            blockid[(inode->blocknum-2068)%1025] = new_blockid;
            write_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
        }
        inode->size = inode->size + sizeof(dir_st);
        write_inode(inodeid, inode);
    }

    if(inode != NULL)
        free(inode);
    inode = NULL;
    return 0;
}

int __inode_add_new_block_to_inode(inodeid_t inodeid, blockid_t *blockidres)
{
    inode_st *inode;
    inode = read_inode(inodeid);
    if(inode == NULL)
        return -1;

    *blockidres = new_block();
    char data[SC_BLOCK_SIZE];
    memset(&data, 0, sizeof(data));
    write_block(*blockidres, &data, sizeof(data));

    blockid_t blockid[SC_BLOCK_SIZE/sizeof(blockid_t)];

    inode->blocknum++;
    if(inode->blocknum <= 16)
    {
        // 直接
        inode->block_id0[inode->blocknum-1] = *blockidres;
    }
    else if((inode->blocknum-17)%1025 < 1)
    {
        // 间接的中间节点
        blockid_t mid_blockid = new_block();
        inode->block_id1[(inode->blocknum-17)/1025] = mid_blockid;
        
        inode->blocknum++;
        memset(blockid, 0, sizeof(blockid));
        blockid[0] = *blockidres;
        write_block(mid_blockid, blockid, SC_BLOCK_SIZE);
    }
    else if(inode->blocknum <= 2066)
    {
        // 间接
        read_block(inode->block_id1[(inode->blocknum-17)/1025], blockid, SC_BLOCK_SIZE);
        blockid[(inode->blocknum-17)%1025-1] = *blockidres;
        write_block(inode->block_id1[(inode->blocknum-17)/1025], blockid, SC_BLOCK_SIZE);
    }
    else if(inode->blocknum == 2067)
    {
        // 双间接第一层中间节点
        blockid_t mid1_blockid = new_block();
        inode->block_id2 = mid1_blockid;
        
        inode->blocknum++;
        memset(blockid, 0, sizeof(blockid));
        blockid_t mid2_blockid = new_block();
        blockid[0] = mid2_blockid;
        write_block(mid1_blockid, blockid, SC_BLOCK_SIZE);

        inode->blocknum++;
        memset(blockid, 0, sizeof(blockid));
        blockid[0] = *blockidres;
        write_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
    }
    else if((inode->blocknum-2068)%1025 < 1)
    {
        // 双间接第二层中间节点
        read_block(inode->block_id2, blockid, SC_BLOCK_SIZE);

        blockid_t mid2_blockid = new_block();
        blockid[(inode->blocknum-2068)/1025] = mid2_blockid;
        write_block(inode->block_id2, blockid, SC_BLOCK_SIZE);

        inode->blocknum++;
        memset(blockid, 0, sizeof(blockid));
        blockid[0] = *blockidres;
        write_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
    }
    else
    {
        // 双间接
        read_block(inode->block_id2, blockid, SC_BLOCK_SIZE);
        blockid_t mid2_blockid = blockid[(inode->blocknum-2068)/1025];

        read_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
        blockid[(inode->blocknum-2068)%1025] = *blockidres;
        write_block(mid2_blockid, blockid, SC_BLOCK_SIZE);
    }
    write_inode(inodeid, inode);

    if(inode != NULL)
        free(inode);
    inode = NULL;
    return 0;
}

int __data_inode(inodeid_t inodeid, const char *data, int loc_begin)//现在loc_begin只能为0
{ // bug较多，待修改
    inode_st *cur_inode=read_inode(inodeid);
    blockid_t mid_blockid[SC_BLOCK_SIZE/sizeof(blockid_t)],mid2_blockid[SC_BLOCK_SIZE/sizeof(blockid_t)];//1024
    if((cur_inode->mode&SC_LNK)==SC_LNK)
    {
        while((cur_inode->mode&SC_LNK)==SC_LNK){//软链接的情况
            if(find_inode((char*)(cur_inode->block_id0),&inodeid)<0)
                return -4;
        }
        cur_inode=read_inode(inodeid);
    }
    if((cur_inode->mode&SC_REG)==SC_REG){//这个函数不能用于修改目录
        return -1;
    }
    int len_data=strlen(data);
    int len_block=(len_data+SC_BLOCK_SIZE-1)/SC_BLOCK_SIZE;
    if(read_block_free()<len_block+(len_block-16+1023)/1024+(len_block>2080)){//判断剩余块数是否足够
        return -2;
    }
    int i,j,now_loc = 0;
    blockid_t tmp_block,mid_block,mid2_block;
    cur_inode->mtime=time(NULL);//确定要修改，记录修改时间
    for(i=0;i<16;i++,len_block--)
    {
        tmp_block=new_block();
        if(tmp_block==-1){
            return -3;
        }      
        cur_inode->block_id0[i]=tmp_block;
        cur_inode->blocknum++;
        if(len_block>1){
            write_block(tmp_block,data+now_loc,SC_BLOCK_SIZE);
        }else{
            write_block(tmp_block,data+now_loc,len_data-now_loc);
            return 0;
        }   
        data+=now_loc;
    }
    mid_block=new_block();
    if(mid_block==-1){
            return -3;
    }
    cur_inode->blocknum++;
    cur_inode->block_id1[0]=mid_block;
    memset(mid_blockid,0,sizeof(mid_blockid));
    for(i=0;i<SC_BLOCK_SIZE/sizeof(blockid_t);i++,len_block--)//1024
    {
        tmp_block=new_block();
        if(tmp_block==-1){
            return -3;
        }
        mid_blockid[i]=tmp_block;
        cur_inode->blocknum++;
        if(len_block>1){
            write_block(tmp_block,data+now_loc,SC_BLOCK_SIZE);
        }else{
            write_block(tmp_block,data+now_loc,len_data-now_loc);
            write_block(mid_block,mid_blockid,SC_BLOCK_SIZE);
            return 0;
        }   
        data+=now_loc;  
    }
    write_block(mid_block,mid_blockid,SC_BLOCK_SIZE);
    mid_block=new_block();
    if(mid_block==-1){
            return -3;
    }
    cur_inode->blocknum++;
    cur_inode->block_id1[1]=mid_block;
    memset(mid_blockid,0,sizeof(mid_blockid));
    for(i=0;i<SC_BLOCK_SIZE/sizeof(blockid_t);i++,len_block--)//1024
    {
        tmp_block=new_block();
        if(tmp_block==-1){
            return -3;
        }
        mid_blockid[i]=tmp_block;
        cur_inode->blocknum++;
        if(len_block>1){
            write_block(tmp_block,data+now_loc,SC_BLOCK_SIZE);
        }else{
            write_block(tmp_block,data+now_loc,len_data-now_loc);
            write_block(mid_block,mid_blockid,SC_BLOCK_SIZE);
            return 0;
        }   
        data+=now_loc;  
    }
    write_block(mid_block,mid_blockid,SC_BLOCK_SIZE);
    mid_block=new_block();
    if(mid_block==-1){
            return -3;
    }
    cur_inode->blocknum++;
    cur_inode->block_id2=mid_block;
    memset(mid_blockid,0,sizeof(mid_blockid));
    for(i=0;i<SC_BLOCK_SIZE/sizeof(blockid_t);i++)
    {
        mid2_block=new_block();
        if(mid2_block==-1){
                return -3;
        }
        cur_inode->blocknum++;
        mid_blockid[i]=mid2_block;
        memset(mid2_blockid,0,sizeof(mid2_blockid));
        for(j=0;j<SC_BLOCK_SIZE/sizeof(blockid_t);j++,len_block--)
        {
            tmp_block=new_block();
            if(tmp_block==-1){
                return -3;
            }
            mid2_blockid[j]=tmp_block;
            cur_inode->blocknum++;
            if(len_block>1){
                write_block(tmp_block,data+now_loc,SC_BLOCK_SIZE);
            }else{
                write_block(tmp_block,data+now_loc,len_data-now_loc);
                write_block(mid_block,mid_blockid,SC_BLOCK_SIZE);
                write_block(mid2_block,mid2_blockid,SC_BLOCK_SIZE);
                return 0;
            }   
            data+=now_loc;
        }
        write_block(mid2_block,mid2_blockid,SC_BLOCK_SIZE);
    }
    write_block(mid_block,mid_blockid,SC_BLOCK_SIZE);
    return -3;//到这里还没保存完，说明空间不够，不过理论上不会到这里
    
    return 0;
}

void __clear_inode(inode_st* inode)
{
    memset(inode->block_id0, 0, sizeof(inode->block_id0));
    memset(inode->block_id1, 0, sizeof(inode->block_id1));
    inode->block_id2 = 0;
}

// 根据绝对路径获取上级目录绝对路径
int get_parent_path(const char *path, char *parent_path)
{
/*
 * [TEST1]: / parent is /
 * [TEST2]: /t1 parent is /
 * [TEST3]: /t2/ parent is /
 * [TEST4]: /t3/t4/t5 parent is /t3/t4
 * [TEST5]: /t3/t4/t5/ parent is /t3/t4
 */
    if(!strcmp(path, "/"))
    {
        strcpy(parent_path, "/");
    }
    else if(path[strlen(path)-1] == '/')
    {
        for(int i = strlen(path)-2;i > 0;i--)
        {
            if(path[i] == '/')
            {
                strncpy(parent_path, path, i<1?1:i);
                break;
            }
        }
    }
    else
    {
        for(int i = strlen(path)-1;i > 0;i--)
        {
            if(path[i] == '/')
            {
                strncpy(parent_path, path, i<1?1:i);
                break;
            }
        }
    }
    return 0;
}

int init_inode(inodeid_t inodeid)
{
    inode_st temp;
    temp.mode = SC_DEFAULT_DIR;
    temp.user = 0;
    temp.group = 0;
    temp.size = 0;
    temp.blocknum = 0;
    temp.flag = 0;
    temp.linknum = 1;
    temp.ctime = time(NULL);
    temp.atime = time(NULL);
    temp.mtime = time(NULL);
    memset(temp.block_id0, 0, sizeof(temp.block_id0));
    memset(temp.block_id1, 0, sizeof(temp.block_id1));
    temp.block_id2 = 0;
    return write_inode(inodeid, &temp);
}

int write_inode(inodeid_t inodeid, inode_st *inode)
{
    // 读取inode所在扇区
    inode_st inodes[SC_SECTOR_SIZE/SC_INODE_SIZE];
    if(read_image(1+inodeid/(SC_SECTOR_SIZE/SC_INODE_SIZE), inodes, SC_SECTOR_SIZE) != 0)
        return -1;
    
    // 修改扇区内容，再写入
    inodes[inodeid%(SC_SECTOR_SIZE/SC_INODE_SIZE)] = *inode;
    return write_image(1+inodeid/(SC_SECTOR_SIZE/SC_INODE_SIZE), inodes, SC_SECTOR_SIZE);
}

inode_st *read_inode(inodeid_t inodeid)
{
    // 读取inode所在扇区
    inode_st inodes[SC_SECTOR_SIZE/SC_INODE_SIZE];
    if(read_image(1+inodeid/(SC_SECTOR_SIZE/SC_INODE_SIZE), inodes, SC_SECTOR_SIZE) != 0)
        return NULL;

    // 在得到的扇区内容中获取请求的inode
    inode_st *res = (inode_st *)malloc(sizeof(inode_st));
    *res = inodes[inodeid%(SC_SECTOR_SIZE/SC_INODE_SIZE)];
    return res;
}

inodeid_t new_inode(void)
{
    int inodeid = new_bitmap(SC_FIRST_INODE_BITMAP_SECTOR, SC_FIRST_BLOCK_BITMAP_SECTOR-1);
    if(inodeid==-1)return -1;
    write_bitmap(SC_FIRST_INODE_BITMAP_SECTOR, SC_FIRST_BLOCK_BITMAP_SECTOR-1, inodeid, 1);
    dec_inode_free();
    return inodeid;
}

int data_inode(inodeid_t inodeid, const char *data)
{
    return __data_inode(inodeid,data,0);
}

int delete_inode(inodeid_t inodeid, int option)
{
    blockid_t mid_blockid[SC_BLOCK_SIZE/sizeof(blockid_t)];
    inode_st *cur_inode = read_inode(inodeid);

    // 如果指向该inode的硬链接数大于1，则修改硬链接数，不删除该inode
    if(cur_inode->linknum > 1)
    {
        cur_inode->linknum--;
        write_inode(inodeid, cur_inode);
        if(cur_inode != NULL)
            free(cur_inode);
        return -2;
    }

    if((cur_inode->mode & SC_DIR) == SC_DIR)
    {
        // 该inode为目录，根据option执行动作
        if(cur_inode->blocknum > 0&&option == 1)
        {
            // 目录非空，且指定递归删除
            dir_st dir[15];
            for(int i = 0;i < cur_inode->blocknum;i++)
            {
                int res = __inode_blockno_to_blockid(cur_inode, i);
                if(res == -1)
                    return -1;
                else if(res == -2)
                    continue;
                
                int ret = read_block(res, dir, sizeof(dir_st)*15);
                if(ret != 0) return -1;

                for(int j = 0;(j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15;j++)
                {
                    if(strcmp(dir[j].filename, ".")&&strcmp(dir[j].filename, ".."))
                        delete_inode(dir[j].inodeid, 1);
                }
            }
            free_inode(inodeid);
        }
        else if(cur_inode->blocknum == 0)
        {
            // 目录为空
            free_inode(inodeid);
        }
        else
        {
            // 目录非空，且未指定递归删除
            if(cur_inode != NULL)
                free(cur_inode);
            return -1;
        }
    }
    else
    {
        // 该inode为文件，释放所有block
        for(int i = cur_inode->blocknum-1;i >= 0;i--)
        {
            int res = __inode_blockno_to_blockid(cur_inode, i);
            if(res >= 0)
                free_block(res);
        }
        if(cur_inode->blocknum > 16)
            free_block(cur_inode->block_id1[0]);
        if(cur_inode->blocknum > 1041)
            free_block(cur_inode->block_id1[1]);
        if(cur_inode->blocknum > 2066)
        {
            blockid_t blockid = cur_inode->block_id2;
            read_block(blockid, mid_blockid, SC_BLOCK_SIZE);
            for(int j = (cur_inode->blocknum-2068)/1025;j >= 0;j--)
                free_block(mid_blockid[j]);
            free_block(cur_inode->block_id2);
        }
        free_inode(inodeid);
    }

    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}

void free_inode(inodeid_t inodeid)
{
    int bitmap_status=read_bitmap(SC_FIRST_INODE_BITMAP_SECTOR, SC_FIRST_BLOCK_BITMAP_SECTOR-1, inodeid);
    if(bitmap_status == 1)
    {
        write_bitmap(SC_FIRST_INODE_BITMAP_SECTOR, SC_FIRST_BLOCK_BITMAP_SECTOR-1, inodeid, 0);
        inc_inode_free();
    }
}

int find_inode(const char *path, inodeid_t *inodeid)
{
    *inodeid = 0;
    
    if(!strcmp(path, "/"))
        return 0;
    else if(path[0] != '/')
        return -1;
    
    char *oripath, *subpath;
    inode_st *cur_inode;
    dir_st dir[15];
    int ret, res, ok;
    
    oripath = (char *)malloc(sizeof(char)*(strlen(path)+1));
    strcpy(oripath, path);

    subpath = strtok(oripath, "/");
    while(subpath != NULL)
    {
        ok = 1;
        cur_inode = read_inode(*inodeid);
        for(int i = 0;ok&&i < cur_inode->blocknum;i++)
        {
            res = __inode_blockno_to_blockid(cur_inode, i);
            if(res == -1)
                return -1;
            else if(res == -2)
                continue;
            
            ret = read_block(res, dir, sizeof(dir_st)*15);
            if(ret != 0) return -1;
            for(int j = 0;ok&&((j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15);j++)
            {
                if(!strcmp(dir[j].filename, subpath))
                {
                    free(cur_inode);
                    *inodeid = dir[j].inodeid;
                    cur_inode = read_inode(dir[j].inodeid);
                    ok = 0;
                }
            }
        }
        if(ok) return -1;
        subpath = strtok(NULL, "/");
    }
    return 0;
}

int make_inode(const char *path, inodeid_t *inodeid)
{
    *inodeid = 0;

    if(!strcmp(path, "/"))
        return 0;
    
    char *oripath, *subpath;
    inode_st *cur_inode;
    inodeid_t inodeidres;
    dir_st dir[15];
    int res, ret, ok;
    
    oripath = (char *)malloc(sizeof(char)*(strlen(path)+1));
    strcpy(oripath, path);

    subpath = strtok(oripath, "/");
    while(subpath != NULL)
    {
        ok = 1;
        cur_inode = read_inode(*inodeid);
        for(int i = 0;ok&&i < cur_inode->blocknum;i++)
        {
            res = __inode_blockno_to_blockid(cur_inode, i);
            if(res == -1)
                return -1;
            else if(res == -2)
                continue;
            
            ret = read_block(res, dir, sizeof(dir_st)*15);
            if(ret != 0) return -1;
            for(int j = 0;ok&&((j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15);j++)
            {
                if(!strcmp(dir[j].filename, subpath))
                {
                    free(cur_inode);
                    *inodeid = dir[j].inodeid;
                    cur_inode = read_inode(dir[j].inodeid);
                    ok = 0;
                }
            }
        }

        // 未找到，在当前inode下新建
        if(ok)
        {
            ret = __inode_add_new_item_to_inode(*inodeid, subpath, &inodeidres);
            if(ret != 0) return -1;
            *inodeid = inodeidres;
        }
        subpath = strtok(NULL, "/");
    }

    if(oripath != NULL)
        free(oripath);
    return 0;
}

int change_inode_mode(inodeid_t inodeid, unsigned int mode)
{
    int ret = 0;
    inode_st *inode;
    inode = read_inode(inodeid);
    inode->mode = mode;

    ret = write_inode(inodeid, inode);
    free(inode);

    return ret;
}

void debug_inode(const inode_st *inode)
{
    printf("mode            = %o\n", inode->mode);
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
