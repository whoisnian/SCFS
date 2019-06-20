#ifndef INODE_H
#define INODE_H

#include <time.h>
#include "definition.h"

typedef struct inode_st
{
    unsigned int mode;              // 权限控制             4    4
    unsigned int user;              // 所属用户             4    8
    unsigned int group;             // 所属用户组           4    12
    unsigned int size;              // 文件或文件夹实际大小 4    16
    unsigned int blocknum;          // 占用block个数        4    20
    unsigned int flag;              // 文件标志位           4    24
    unsigned int linknum;           // 链接数               4    28
    time_t ctime;                   // 创建或状态改变时间   8    36
    time_t atime;                   // 上次读取时间         8    44
    time_t mtime;                   // 上次修改时间         8    52
    unsigned int block_id0[16];     // block直接指向        4*16 116     直接 64K
    unsigned int block_id1[2];      // block间接指向        4*2  124     间接 8M
    unsigned int block_id2;         // block双间接指向      4    128     双间接 4G
}inode_st;

// 获取inode中的第 blockno 个block的 blockid
// 返回值：
//   -1 表示运行过程中出错
//   -2 表示blockno对应block为中间节点，不存储文件或文件夹实际数据
int __inode_blockno_to_blockid(const inode_st *inode, unsigned int blockno);

// 向inode的block中添加一项
int __inode_add_new_item_to_inode(inodeid_t inodeid, const char *itemname, inodeid_t *inodeidres);

// 初始化inode
int init_inode(inodeid_t inodeid);

// 写入inode
int write_inode(inodeid_t inodeid, inode_st *inode);

// 读取inode，返回的指针在使用完毕后需要手动释放
inode_st *read_inode(inodeid_t inodeid);

// 分配一个新的inode
inodeid_t new_inode(void);

// 根据路径查找inode
int find_inode(const char *path, inodeid_t *inodeid);

// 根据路径创建inode
int make_inode(const char *path, inodeid_t *inodeid);

// 查看inode调试信息
void debug_inode(const inode_st *inode);

#endif // INODE_H
