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
typedef unsigned int inodeid_t;

// 文件属性
#define SC_USER_MAX 32                                  // 用户名长度限制
#define SC_NAME_MAX 256                                 // 文件名长度限制
#define SC_PATH_MAX 4096                                // 路径长度限制

// inode权限设置
#define SC_DIR (1<<10)
#define SC_USR_PRIVILEGE_R (1<<9)
#define SC_USR_PRIVILEGE_W (1<<8)
#define SC_USR_PRIVILEGE_X (1<<7)
#define SC_GRP_PRIVILEGE_R (1<<6)
#define SC_GRP_PRIVILEGE_W (1<<5)
#define SC_GRP_PRIVILEGE_X (1<<4)
#define SC_OTH_PRIVILEGE_R (1<<3)
#define SC_OTH_PRIVILEGE_W (1<<2)
#define SC_OTH_PRIVILEGE_X (1<<1)
#define SC_S_LINK (1)

#define SC_USR_PRIVILEGE_ALL (SC_USR_PRIVILEGE_R|SC_USR_PRIVILEGE_W|SC_USR_PRIVILEGE_X)
#define SC_GRP_PRIVILEGE_ALL (SC_GRP_PRIVILEGE_R|SC_GRP_PRIVILEGE_W|SC_GRP_PRIVILEGE_X)
#define SC_OTH_PRIVILEGE_ALL (SC_OTH_PRIVILEGE_R|SC_OTH_PRIVILEGE_W|SC_OTH_PRIVILEGE_X)
#define DEFAULT_DIR_PRIVILEGE (SC_DIR|SC_USR_PRIVILEGE_ALL|SC_GRP_PRIVILEGE_R|SC_GRP_PRIVILEGE_X|SC_OTH_PRIVILEGE_R|SC_OTH_PRIVILEGE_X)
#define DEFAULT_FILE_PRIVILEGE (SC_USR_PRIVILEGE_R|SC_USR_PRIVILEGE_W|SC_GRP_PRIVILEGE_R|SC_OTH_PRIVILEGE_R)

#endif // DEFINITION_H
