#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdbool.h>
#include <time.h>

typedef struct superblock_st
{
    unsigned int block_total;           // block总量                4   4
    unsigned int block_free;            // block余量                4   8
    unsigned int inode_total;           // inode总量                4   12
    unsigned int inode_free;            // inode余量                4   16

    unsigned int block_size;            // block大小                4   20
    unsigned int inode_size;            // inode大小                4   24

    time_t create_time;                 // 创建时间                 8   32
    time_t last_mount;                  // 上次挂载时间             8   40
    time_t last_write;                  // 上次写入时间             8   48

    unsigned int first_inode;           // inode开始所在扇区        4   52
    unsigned int first_inode_bitmap;    // inode位示图开始所在扇区  4   56
    unsigned int first_block_bitmap;    // block位示图开始所在扇区  4   60
    unsigned int first_block;           // block开始所在扇区        4   64

    bool validbit;                      // 未挂载标记               1   65
}superblock_st;

// 初始化superblock
int init_superblock(void);

// 写入superblock
int write_superblock(superblock_st *superblock);

// 读取superblock，返回的指针在使用完毕后需要手动释放
superblock_st *read_superblock(void);

//读取block_free
unsigned int read_block_free();

//读取inode_free
unsigned int read_inode_free();

//写入block_free
int write_block_free(unsigned int new_block_free);

//写入inode_free
int write_inode_free(unsigned int new_inode_free);

//block_free++
int add_block_free();

//inode_free++
int add_inode_free();

//block_free--
int dec_block_free();

//inode_free--
int dec_inode_free();

// 查看superblock调试信息
void debug_superblock(const superblock_st *superblock);

#endif // SUPERBLOCK_H
