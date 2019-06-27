#ifndef SCFS_H
#define SCFS_H

#define FUSE_USE_VERSION 31

#include <fuse.h>

extern unsigned int cur_user_id;
extern unsigned int cur_group_id;  
extern int temp_root;

// 初始化文件系统，相当于格式化
int init_scfs(const char *filepath);

// 打开文件系统
int open_scfs(const char *filepath);

// 关闭文件系统
int close_scfs(void);

// 运行操作命令，命令各参数以空格分割
int run_command(const char *cmd);

// 用户登录（在shadow文件中检查密码，检查通过后在passwd中获取uid和gid，更改全局变量cur_user_id和cur_group_id）
int command_login(const char *username, const char *password);

// 修改用户密码（在passwd文件中查找用户是否存在，若存在的话根据uid确定是否能修改，再去修改shadow文件中的密码）
int command_passwd(const char *username, const char *password);

// 添加用户（在passwd文件中查找用户是否存在，若不存在的话写入passwd文件新用户，并写入group文件同名用户组）
int command_useradd(const char *username);

// 删除用户（在passwd文件中查找用户是否存在，若存在的话从passwd文件删除用户，并删除shadow文件中的用户密码）
int command_userdel(const char *username);

// 添加用户组（在group文件中查找用户组是否存在，若不存在的话写入group文件新的用户组）
int command_groupadd(const char *groupname);

// 删除用户组（在group文件中查找用户组是否存在，若存在的话删除group文件中的该用户组）
int command_groupdel(const char *groupname);

// 更改用户和用户组的对应关系（在group文件中找到用户组gid，在passwd文件中找到该用户，更改其gid并写回passwd）
int command_gpasswd(const char *username, const char *groupname);

// 用于交互程序readdir的自定义filler函数，buf会被填充以/分隔的文件名
int sc_filler(void *buf, const char *name, const struct stat *stbuf, off_t off, enum fuse_fill_dir_flags flags);

// fuse接口：运行主程序时会调用该函数
void *sc_init(struct fuse_conn_info *conn, struct fuse_config *cfg);

// fuse接口：获取path对应的inode属性
int sc_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi);

// fuse接口：获取path目录下所有项
int sc_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags);

// fuse接口：打开文件
int sc_open(const char *path, struct fuse_file_info *fi);

// fuse接口：读取文件内容，返回实际读取到的字节数，负值表示出错
int sc_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

// fuse接口：向文件内写入内容，返回实际写入的字节数，负值表示出错
int sc_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

// fuse接口：在指定的path下创建权限为mode的目录
int sc_mkdir(const char *path, mode_t mode);

// fuse接口：在指定的path下创建权限为mode的文件
int sc_create(const char *path, mode_t mode, struct fuse_file_info *fi);

// fuse接口：修改path对应inode的权限为mode（该mode需要包含文件类型SC_DIR或SC_REG属性）
int sc_chmod(const char *path, mode_t mode, struct fuse_file_info *fi);

// fuse接口：通常用于move，从from_path的父级目录中删除from，在to_path的父级目录中创建to，对应的inode不变
int sc_rename(const char *from, const char *to, unsigned int flags);

// fuse接口：获取文件系统余量，df命令会用到，Dolphin调用它判断剩余空间能否移动进来新文件
int sc_statfs(const char *path, struct statvfs *stbuf);

// fuse接口：删除文件
int sc_unlink(const char *path);

// fuse接口：删除文件夹
int sc_rmdir(const char *path);

// fuse接口：判断当前用户和用户组是否对path对应iode具有mask权限，mask可取SC_R_OK，SC_W_OK，SC_X_OK，SC_F_OK
int sc_access(const char *path, int mask);

// fuse接口：修改path对应inode所属用户id和所属用户组id
int sc_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi);

/*
//int (*getattr) (const char *, struct stat *, struct fuse_file_info *fi);
int (*readlink) (const char *, char *, size_t);
int (*mknod) (const char *, mode_t, dev_t);
//int (*mkdir) (const char *, mode_t);
//int (*unlink) (const char *);
//int (*rmdir) (const char *);
int (*symlink) (const char *, const char *);
//int (*rename) (const char *, const char *, unsigned int flags);
int (*link) (const char *, const char *);
//int (*chmod) (const char *, mode_t, struct fuse_file_info *fi);
//int (*chown) (const char *, uid_t, gid_t, struct fuse_file_info *fi);
int (*truncate) (const char *, off_t, struct fuse_file_info *fi);
//int (*open) (const char *, struct fuse_file_info *);
//int (*read) (const char *, char *, size_t, off_t, struct fuse_file_info *);
//int (*write) (const char *, const char *, size_t, off_t, struct fuse_file_info *);
//int (*statfs) (const char *, struct statvfs *);
int (*flush) (const char *, struct fuse_file_info *);
int (*release) (const char *, struct fuse_file_info *);
int (*fsync) (const char *, int, struct fuse_file_info *);
int (*setxattr) (const char *, const char *, const char *, size_t, int);
int (*getxattr) (const char *, const char *, char *, size_t);
int (*listxattr) (const char *, char *, size_t);
int (*removexattr) (const char *, const char *);
int (*opendir) (const char *, struct fuse_file_info *);
//int (*readdir) (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *, enum fuse_readdir_flags);
int (*releasedir) (const char *, struct fuse_file_info *);
int (*fsyncdir) (const char *, int, struct fuse_file_info *);
//void *(*init) (struct fuse_conn_info *conn, struct fuse_config *cfg);
void (*destroy) (void *private_data);
//int (*access) (const char *, int);
//int (*create) (const char *, mode_t, struct fuse_file_info *);
int (*lock) (const char *, struct fuse_file_info *, int cmd, struct flock *);
int (*utimens) (const char *, const struct timespec tv[2], struct fuse_file_info *fi);
int (*bmap) (const char *, size_t blocksize, uint64_t *idx);
int (*ioctl) (const char *, unsigned int cmd, void *arg, struct fuse_file_info *, unsigned int flags, void *data);
int (*poll) (const char *, struct fuse_file_info *, struct fuse_pollhandle *ph, unsigned *reventsp);
int (*write_buf) (const char *, struct fuse_bufvec *buf, off_t off, struct fuse_file_info *);
int (*read_buf) (const char *, struct fuse_bufvec **bufp, size_t size, off_t off, struct fuse_file_info *);
int (*flock) (const char *, struct fuse_file_info *, int op);
int (*fallocate) (const char *, int, off_t, off_t, struct fuse_file_info *);
ssize_t (*copy_file_range) (const char *path_in,
    struct fuse_file_info *fi_in,
    off_t offset_in, const char *path_out,
    struct fuse_file_info *fi_out,
    off_t offset_out, size_t size, int flags);
*/
#endif //SCFS_H
