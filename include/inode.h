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
    unsigned int block_id0[16];     // block直接指向        4*16 92      直接 64K
    unsigned int block_id1[2];      // block间接指向        4*2  100     间接 8M
    unsigned int block_id2;         // block双间接指向      4    104     双间接 4G
    time_t ctime;                   // 创建或状态改变时间   8    112
    time_t atime;                   // 上次读取时间         8    120
    time_t mtime;                   // 上次修改时间         8    128
}inode_st;

// 获取inode中的第 blockno 个block的 blockid（blockno从0开始计数）
// 返回值：
//   -1 表示运行过程中出错
//   -2 表示blockno对应block为中间节点，不存储文件或文件夹实际数据
int __inode_blockno_to_blockid(const inode_st *inode, unsigned int blockno);

// 向目录inode的block中添加一项目录或文件
int __inode_add_new_item_to_inode(inodeid_t inodeid, const char *itemname, inodeid_t *inodeidres);

// 向inode中添加一块block
int __inode_add_new_block_to_inode(inodeid_t inodeid, blockid_t *blockidres);

// 修改文件类型的inode指定范围[loc_begin,loc_begin+strlen(data))指向的数据，以字节为单位，注意不要超出范围，现在暂时只能处理loc_begin为零，如果需要不为零的接口再补写
// 返回值：
//   -1 inode是目录
//   -2 空间不足
//   -3 意外错误（如开始空间足够，运行过程中空间不足）
//   -4 inode作为软链接指向不存在或不合法的路径
int __data_inode(inodeid_t inodeid, const char *data, int loc_begin);

//清空inode的blockid
void __clear_inode(inode_st* inode); 

// 根据绝对路径获取上级目录的绝对路径
int get_parent_path(const char *path, char *parent_path);

// 初始化inode
int init_inode(inodeid_t inodeid);

// 写入inode
int write_inode(inodeid_t inodeid, inode_st *inode);

// 读取inode，返回的指针在使用完毕后需要手动释放
inode_st *read_inode(inodeid_t inodeid);

// 分配一个新的inode
inodeid_t new_inode(void);

// 修改文件类型的inode指向的数据
int data_inode(inodeid_t inodeid, const char *data);

// 删除一个inode(内部判断是文件，软链接还是目录)，total表示是否递归删除所有
int delete_inode(inodeid_t inodeid, bool total);

//获取一个目录类型inode的目录
int list_inode(inodeid_t inodeid, char *list);

// 根据路径查找inode
int find_inode(const char *path, inodeid_t *inodeid);

// 根据路径创建inode
int make_inode(const char *path, inodeid_t *inodeid);

// 修改指定inode的mode
int change_inode_mode(inodeid_t inodeid, unsigned int mode);

// 查看inode调试信息
void debug_inode(const inode_st *inode);

#endif // INODE_H
