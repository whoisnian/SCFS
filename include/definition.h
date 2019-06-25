#ifndef DEFINITION_H
#define DEFINITION_H

// 文件系统底层存储结构
#define SC_IMAGE_SIZE (1<<30)                           // 文件系统image总大小（1g = 2^30byte）
#define SC_SECTOR_SIZE 4096                             // 文件系统image单个sector大小（byte）
#define SC_SECTOR_COUNT (SC_IMAGE_SIZE/SC_BLOCK_SIZE)   // sector个数（262144）
#define SC_SUPERBLOCK_SIZE 1024                         // superblock大小（byte）
#define SC_INODE_SIZE 128                               // inode大小（byte）
#define SC_INODE_COUNT (1<<16)                          // inode个数（65536）
#define SC_BLOCK_SIZE 4096                              // block大小（byte）
#define SC_BLOCK_COUNT (SC_SECTOR_COUNT-1-2048-2-8)     // block个数（260085）

// SC_BLOCK_COUNT
// block大小和扇区大小相等，所以：
// block个数 = 扇区数 - 超级块占的扇区数 - inode表占的扇区数 - inode位示图占的扇区数 - block位示图占的扇区数

// 文件系统底层结构位置信息
#define SC_FIRST_INODE_SECTOR 1                                                                         // inode开始所在扇区
#define SC_FIRST_INODE_BITMAP_SECTOR (1+SC_INODE_COUNT*SC_INODE_SIZE/SC_SECTOR_SIZE)                    // inode位示图开始所在扇区
#define SC_FIRST_BLOCK_BITMAP_SECTOR (SC_FIRST_INODE_BITMAP_SECTOR+SC_INODE_COUNT/8/SC_SECTOR_SIZE)     // block位示图开始所在扇区
#define SC_FIRST_BLOCK_SECTOR (SC_FIRST_BLOCK_BITMAP_SECTOR+SC_SECTOR_COUNT/8/SC_SECTOR_SIZE)           // block开始所在扇区

typedef unsigned int sectorid_t;
typedef unsigned int inodeid_t;
typedef unsigned int blockid_t;

// 文件属性，参考 /usr/include/linux/limits.h 和 man useradd
#define SC_USER_MAX 32                                  // 用户名长度限制
#define SC_NAME_MAX 255                                 // 文件名长度限制
#define SC_PATH_MAX 4096                                // 绝对路径长度限制

// 权限设置，参考 /usr/include/bits/stat.h 和 /usr/include/sys/stat.h
#define SC_DIR 0040000      // 目录
#define SC_LNK 0120000      // 软链接
#define SC_REG 0100000      // 文件
#define SC_USR_R 0400       // 用户读
#define SC_USR_W 0200       // 用户写
#define SC_USR_X 0100       // 用户执行
#define SC_GRP_R 040        // 用户组读
#define SC_GRP_W 020        // 用户组写
#define SC_GRP_X 010        // 用户组执行
#define SC_OTH_R 04         // 其它读
#define SC_OTH_W 02         // 其它写
#define SC_OTH_X 01         // 其它执行

#define SC_USR_ALL (SC_USR_R|SC_USR_W|SC_USR_X)
#define SC_GRP_ALL (SC_GRP_R|SC_GRP_W|SC_GRP_X)
#define SC_OTH_ALL (SC_OTH_R|SC_OTH_W|SC_OTH_X)
#define SC_DEFAULT_DIR (SC_DIR|SC_USR_ALL|SC_GRP_R|SC_GRP_X|SC_OTH_R|SC_OTH_X)
#define SC_DEFAULT_FILE (SC_REG|SC_USR_R|SC_USR_W|SC_GRP_R|SC_OTH_R)

#define SC_R_OK 4       // 测试是否具有读取权限
#define SC_W_OK 2       // 测试是否具有写入权限
#define SC_X_OK 1       // 测试是否具有执行权限
#define SC_F_OK 0       // 测试是否存在

// 根用户
#define SC_ROOT_UID 0
#define SC_ROOT_GID 0
#define SC_ROOT_USERNAME "root"
#define SC_ROOT_PASSWORD "toor"

typedef unsigned int userid_t;
typedef unsigned int groupid_t;

// TEST结果输出，statement为真时输出OK，否则输出ERROR
#define OK_OR_ERROR(statement) \
if(statement) \
    printf("\033[1;32;40mOK\033[0m"); \
else \
    printf("\033[1;31;40mERROR\033[0m");

#endif // DEFINITION_H
