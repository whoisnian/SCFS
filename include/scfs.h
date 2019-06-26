#ifndef SCFS_H
#define SCFS_H

#define FUSE_USE_VERSION 31

#include <fuse.h>

extern unsigned int cur_user_id;
extern unsigned int cur_group_id;  

int init_scfs(const char *filepath);
int open_scfs(const char *filepath);
int close_scfs(void);

// 返回值为0表示成功，非0表示失败
int run_command(const char *cmd);
int command_login(const char *username, const char *password);
int command_passwd(const char *username, const char *password);
int command_useradd(const char *username);
int command_userdel(const char *username);
int command_groupadd(const char *groupname);
int command_groupdel(const char *groupname);
int command_gpasswd(const char *username, const char *groupname);

int sc_filler(void *buf, const char *name, const struct stat *stbuf, off_t off, enum fuse_fill_dir_flags flags);

void *sc_init(struct fuse_conn_info *conn, struct fuse_config *cfg);
int sc_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi);
int sc_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags);
int sc_open(const char *path, struct fuse_file_info *fi);
int sc_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sc_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sc_mkdir(const char *path, mode_t mode);
int sc_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int sc_chmod(const char *path, mode_t mode, struct fuse_file_info *fi);
int sc_rename(const char *from, const char *to, unsigned int flags);
int sc_statfs(const char *path, struct statvfs *stbuf);
int sc_unlink(const char *path);
int sc_rmdir(const char *path);
int sc_access(const char *path, int mask);

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
int (*chown) (const char *, uid_t, gid_t, struct fuse_file_info *fi);
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
