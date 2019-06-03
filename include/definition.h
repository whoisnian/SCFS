#ifndef DEFINITION_H
#define DEFINITION_H

// 文件系统底层存储结构
#define SC_IMAGE_SIZE (1<<30)                           // 文件系统image总大小（1g = 2^30byte）
#define SC_SUPERBLOCK_SIZE 1024                         // superblock大小（byte）
#define SC_INODE_SIZE 128                               // inode大小（byte）
#define SC_INODE_COUNT (1<<16)                          // inode个数（65536）
#define SC_BLOCK_SIZE 4096                              // 文件系统image单个block大小（byte）
#define SC_BLOCK_COUNT (SC_IMAGE_SIZE/SC_BLOCK_SIZE)    // block个数（262144）
typedef unsigned int blockid_t;

// 文件属性
#define SC_USER_MAX 32                                  // 用户名长度限制
#define SC_GROUP_MAX 32                                 // 用户组长度限制
#define SC_NAME_MAX 256                                 // 文件名长度限制
#define SC_PATH_MAX 4096                                // 路径长度限制

#endif // DEFINITION_H
