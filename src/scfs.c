/*************************************************************************
    > File Name: inode.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月07日 星期五 02时08分03秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "definition.h"
#include "image.h"
#include "superblock.h"
#include "block.h"
#include "inode.h"
#include "bitmap.h"
#include "scfs.h"
#include "debugprintf.h"

unsigned int cur_user_id = SC_GUEST_USER_ID;
unsigned int cur_group_id = SC_GUEST_GROUP_ID;

int init_scfs(const char *filepath)
{
    int ret;
    // 初始化image
    ret = open_image(filepath);
    if(ret != 0)
        return ret;

    // 初始化superblock
    ret = init_superblock();
    if(ret != 0)
        return ret;
    
    superblock_st *superblock;
    superblock = read_superblock();
    if(superblock == NULL)
        return -1;
    
    superblock->block_free = superblock->block_total - superblock->first_block - 1;
    superblock->inode_free = superblock->inode_total - 1;
    superblock->validbit = 1;
    superblock->last_write = time(NULL);

    ret = write_superblock(superblock);
    if(ret != 0)
        return ret;

    if(superblock != NULL)
        free(superblock);

    // 初始化inode
    ret = init_inode(0);
    if(ret != 0)
        return ret;

    inode_st *inode;
    inode = read_inode(0);
    if(inode == NULL)
        return -1;

    inode->mode = SC_DEFAULT_DIR;
    inode->user = 0;
    inode->group = 0;
    inode->size = sizeof(dir_st);
    inode->blocknum = 1;
    inode->linknum = 1;
    inode->atime = time(NULL);
    inode->mtime = time(NULL);
    inode->block_id0[0] = 0;

    // xxd test.img | grep 0080b000 可以在 0080b000 一行看到 2e (.)
    dir_st dir = {0, "."};
    ret = write_block(0, &dir, sizeof(dir_st));
    if(ret != 0)
        return ret;

    ret = write_inode(0, inode);
    if(ret != 0)
        return ret;

    // 初始化inode bitmap
    ret = init_bitmap(superblock->first_inode_bitmap, superblock->first_block_bitmap-1);
    if(ret != 0)
        return ret;
    
    ret = write_bitmap(superblock->first_inode_bitmap, superblock->first_block_bitmap-1, 0, 1);
    if(ret != 0)
        return ret;

    // 初始化block bitmap
    ret = init_bitmap(superblock->first_block_bitmap, superblock->first_block-1);
    if(ret != 0)
        return ret;

    ret = write_bitmap(superblock->first_block_bitmap, superblock->first_block-1, 0, 1);
    if(ret != 0)
        return ret;
    
    // 初始化用户认证相关文件
    char tmp[128];
    if(sc_access("/.passwd", SC_F_OK) != 0)
    {
        sc_create("/.passwd", SC_REG|SC_USR_R|SC_USR_W|SC_GRP_R|SC_OTH_R, NULL);
        // username:x:uid:gid:::
        // root:x:0:0:::
        memset(tmp, 0, sizeof(tmp));
        strcpy(tmp, "root:x:0:0:::\n");
        sc_write("/.passwd", tmp, strlen(tmp), 0, NULL);
    }
    if(sc_access("/.group", SC_F_OK) != 0)
    {
        sc_create("/.group", SC_REG|SC_USR_R|SC_USR_W|SC_GRP_R|SC_OTH_R, NULL);
        // groupname:x:gid:
        // root:x:0:
        memset(tmp, 0, sizeof(tmp));
        strcpy(tmp, "root:x:0:\n");
        sc_write("/.group", tmp, strlen(tmp), 0, NULL);
    }
    if(sc_access("/.shadow", SC_F_OK) != 0)
    {
        sc_create("/.shadow", SC_REG|SC_USR_R|SC_USR_W, NULL);
        // username:password
        // root:toor
        memset(tmp, 0, sizeof(tmp));
        strcpy(tmp, "root:");
        strcat(tmp, SC_ROOT_PASSWORD);
        strcat(tmp, "\n");
        sc_write("/.shadow", tmp, strlen(tmp), 0, NULL);
    }
    if(sc_access("/.run_command", SC_F_OK) != 0)
    {
        sc_create("/.run_command", SC_REG|SC_USR_W|SC_GRP_W|SC_OTH_W, NULL);
    }

    return 0;
}

int open_scfs(const char *filepath)
{
    int ret;
    // 打开image
    ret = open_image(filepath);
    if(ret != 0)
        return ret;

    // 读取superblock
    superblock_st *superblock;
    superblock = read_superblock();
    if(superblock == NULL)
        return -1;
    
    superblock->validbit = 0;
    superblock->last_mount = time(NULL);

    ret = write_superblock(superblock);

    if(superblock != NULL)
        free(superblock);

    return 0;
}

int close_scfs(void)
{
    return close_image();
}

int run_command(const char *cmd)
{
    debug_printf(debug_info, "run_command %s\n", cmd);
    if(cmd == NULL||strlen(cmd) <= 1)
        return -1;
    char *oricmd, *subcmd;
    char *username, *password, *groupname;
    oricmd = (char *)malloc(sizeof(char)*(strlen(cmd)+1));
    strcpy(oricmd, cmd);

    subcmd = strtok(oricmd, " ");
    if(subcmd != NULL)
    {
        if(!strcmp(subcmd, "login"))
        {
            // 登录
            // login username password
            username = strtok(NULL, " ");
            password = strtok(NULL, " ");
            return command_login(username, password);
        }
        else if(!strcmp(subcmd, "passwd"))
        {
            // 修改密码
            // passwd username password
            username = strtok(NULL, " ");
            password = strtok(NULL, " ");
            return command_passwd(username, password);
        }
        else if(!strcmp(subcmd, "useradd"))
        {
            // 新建用户
            // useradd username
            username = strtok(NULL, " ");
            return command_useradd(username);
        }
        else if(!strcmp(subcmd, "userdel"))
        {
            // 删除用户
            // userdel username
            username = strtok(NULL, " ");
            return command_userdel(username);
        }
        else if(!strcmp(subcmd, "groupadd"))
        {
            // 新建组
            // groupadd groupname
            groupname = strtok(NULL, " ");
            return command_groupadd(groupname);
        }
        else if(!strcmp(subcmd, "groupdel"))
        {
            // 删除组
            // groupdel groupname
            groupname = strtok(NULL, " ");
            return command_groupdel(groupname);
        }
        else if(!strcmp(subcmd, "gpasswd"))
        {
            // 修改用户所属组
            // gpasswd username groupname
            username = strtok(NULL, " ");
            groupname = strtok(NULL, " ");
            return command_gpasswd(username, groupname);
        }
    }
    return -1;
}

int command_login(const char *username, const char *password)
{
    if(username == NULL||password == NULL) return -1;
    char buf[4096], *subbuf, userpass[4096];
    char buf2[4096], *subbuf2;
    memset(buf, 0, sizeof(buf));
    memset(userpass, 0, sizeof(userpass));

    strcpy(userpass, username);
    strcat(userpass, ":");
    strcat(userpass, password);

    int ret = sc_read("/.shadow", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    subbuf = strtok(buf, "\n");
    while(subbuf != NULL)
    {
        if(!strcmp(userpass, subbuf))
        {
            memset(buf2, 0, sizeof(buf2));
            sc_read("/.passwd", buf2, 4096, 0, NULL);
            subbuf2 = strtok(buf2, "\n");
            while(subbuf2 != NULL)
            {
                if(!strncmp(username, subbuf2, strlen(username)))
                {
                    unsigned int uid = SC_GUEST_USER_ID, gid=SC_GUEST_GROUP_ID;
                    char formatstr[4096];
                    strcpy(formatstr, username);
                    strcat(formatstr, ":x:%u:%u:::");
                    sscanf(subbuf2, formatstr, &uid, &gid);
                    cur_user_id = uid;
                    cur_group_id = gid;
                    return strlen(username)+strlen(password)+7;
                }
                subbuf2 = strtok(NULL, "\n");
            }
            return -1;
        }
        subbuf = strtok(NULL, "\n");
    }
    return -1;
}

int command_passwd(const char *username, const char *password)
{
    if(username == NULL||password == NULL) return -1;
    char buf[4096], *subbuf;
    memset(buf, 0, sizeof(buf));

    unsigned int uid = SC_GUEST_USER_ID;
    if(cur_user_id != 0)
    {
        int ret = sc_read("/.passwd", buf, 4096, 0, NULL);
        if(ret < 0) return ret;

        subbuf = strtok(buf, "\n");
        while(subbuf != NULL)
        {
            if(!strncmp(username, subbuf, strlen(username)))
            {
                char formatstr[4096];
                strcpy(formatstr, username);
                strcat(formatstr, ":x:%u:%*u:::");
                sscanf(subbuf, formatstr, &uid);
                break;
            }
            subbuf = strtok(NULL, "\n");
        }
    }

    if(cur_user_id != uid&&cur_user_id != 0)
        return -1;
    
    memset(buf, 0, sizeof(buf));
    int ret = sc_read("/.shadow", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    char userpass[4096];
    memset(userpass, 0, sizeof(userpass));
    strcpy(userpass, username);
    strcat(userpass, ":");
    strcat(userpass, password);

    // todo 修改buf

    sc_write("/.shadow", buf, 4096, 0, NULL);

    return strlen(username)+strlen(password)+8;
}

int command_useradd(const char *username)
{
    if(username == NULL) return -1;
    return 0;
}

int command_userdel(const char *username)
{
    if(username == NULL) return -1;
    return 0;
}

int command_groupadd(const char *groupname)
{
    if(groupname == NULL) return -1;
    return 0;
}

int command_groupdel(const char *groupname)
{
    if(groupname == NULL) return -1;
    return 0;
}

int command_gpasswd(const char *username, const char *groupname)
{
    if(username == NULL||groupname == NULL) return -1;
    return 0;
}

int sc_filler(void *buf, const char *name, const struct stat *stbuf, off_t off, enum fuse_fill_dir_flags flags)
{
    if(strcmp(name, ".")&&strcmp(name, ".."))
    {
        strcat(buf, "/");
        strcat(buf, name);
    }
    return 0;
}

void *sc_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    debug_printf(debug_info, "Call sc_init()\n");
    (void) conn;
    (void) cfg;
    return NULL;
}

int sc_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi)
{
    (void) fi;
    int ret;
    inode_st *inode = NULL;
    inodeid_t inodeid;

    ret = find_inode(path, &inodeid);
    if(ret != 0)
        return -ENOENT;

    inode = read_inode(inodeid);
    if(inode == NULL)
        return -ENOENT;

    memset(buf, 0, sizeof(struct stat));
    //buf->st_dev
    buf->st_ino = inodeid;
    buf->st_mode = inode->mode;
    buf->st_nlink = inode->linknum;
    buf->st_uid = inode->user;
    buf->st_gid = inode->group;
    //buf->st_rdev
    buf->st_size = inode->size;
    buf->st_blksize = SC_BLOCK_SIZE;
    buf->st_blocks = inode->blocknum;
    buf->st_atime = inode->atime;
    buf->st_mtime = inode->mtime;
    buf->st_ctime = inode->ctime;

    if(inode != NULL)
        free(inode);
    return 0;
}

int sc_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    (void) offset;
	(void) fi;
	(void) flags;
    int ret, res;
    inode_st *cur_inode = NULL;
    inodeid_t cur_inodeid;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    dir_st dir[15];
    for(int i = 0;i < cur_inode->blocknum;i++)
    {
        res = __inode_blockno_to_blockid(cur_inode, i);
        if(res == -1)
            return -1;
        else if(res == -2)
            continue;
        
        ret = read_block(res, dir, sizeof(dir_st)*15);
        if(ret != 0) return -1;

        for(int j = 0;(j < 15&&i < cur_inode->blocknum-1)||j < cur_inode->size/sizeof(dir_st)%15;j++)
        {
            if(strcmp(dir[j].filename, ".")&&strcmp(dir[j].filename, ".."))
                filler(buf, dir[j].filename, NULL, 0, 0);
        }
    }

    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}

int sc_open(const char *path, struct fuse_file_info *fi)
{
    (void) fi;
    int ret;
    inode_st *cur_inode = NULL;
    inodeid_t cur_inodeid;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}

int sc_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void) fi;
    int ret, res;
    size_t ret_size = 0;
    inode_st *cur_inode = NULL;
    inodeid_t cur_inodeid;
    char data[SC_BLOCK_SIZE];

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    long len = cur_inode->size;
	if(offset < len)
    {
		if(offset + size > len)
			size = len - offset;
		ret_size = size;

        // 计算到offset包含中间节点一共多少个block
        int num = offset/SC_BLOCK_SIZE;
        int i;
        if(num < 16)
            i = num;
        else if(num < 2064)
            i = num + (num-16)/1024 + 1;
        else if(num < 1050640)
            i = num + (num-2064)/1024 + 4;
        else
            return -EFAULT;
        
        for(;i < cur_inode->blocknum;i++)
        {
            res = __inode_blockno_to_blockid(cur_inode, i);
            if(res == -1)
                return -1;
            else if(res == -2)
                continue;
            
            ret = read_block(res, data, SC_BLOCK_SIZE);
            if(ret != 0) return -1;

            if(size > 0)
            {
                if(size < SC_BLOCK_SIZE-(offset%SC_BLOCK_SIZE))
                {
                    memcpy(buf+(ret_size-size), data+(offset%SC_BLOCK_SIZE), size);
                    size = 0;
                }
                else
                {
                    memcpy(buf+(ret_size-size), data+(offset%SC_BLOCK_SIZE), SC_BLOCK_SIZE-(offset%SC_BLOCK_SIZE));
                    size = size-SC_BLOCK_SIZE+(offset%SC_BLOCK_SIZE);
                }
                offset = 0;
            }
            else
            {
                break;
            }
        }
	}
    else
    {
		size = 0;
    }

    if(cur_inode != NULL)
        free(cur_inode);
    if(size > 0)
        ret_size -= size;
    return ret_size;
}

int sc_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    if(!strcmp(path, "/.run_command"))
        return run_command(buf);

    (void) fi;
    int ret, res;
    size_t ret_size = size;
    inode_st *cur_inode = NULL;
    inodeid_t cur_inodeid;
    blockid_t blockid;
    char data[SC_BLOCK_SIZE];

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    // 分配足够的block
    long len = cur_inode->size;
    for(int i = ((offset+size+SC_BLOCK_SIZE-1)/SC_BLOCK_SIZE)-((len+SC_BLOCK_SIZE-1)/SC_BLOCK_SIZE);i > 0;i--)
    {
        ret = __inode_add_new_block_to_inode(cur_inodeid, &blockid);
        if(ret != 0)
            return ret;
    }

    if(cur_inode != NULL)
        free(cur_inode);
    cur_inode = NULL;
    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    if(offset+size > len)
        cur_inode->size = offset+size;

    // 计算到offset包含中间节点一共多少个block
    int num = offset/SC_BLOCK_SIZE;
    int i;
    if(num < 16)
        i = num;
    else if(num < 2064)
        i = num + (num-16)/1024 + 1;
    else if(num < 1050640)
        i = num + (num-2064)/1024 + 4;
    else
        return -EFAULT;

    for(;i < cur_inode->blocknum;i++)
    {
        res = __inode_blockno_to_blockid(cur_inode, i);
        if(res == -1)
            return -1;
        else if(res == -2)
            continue;
        
        ret = read_block(res, data, SC_BLOCK_SIZE);
        if(ret != 0) return -1;

        if(size > 0)
        {
            if(size<SC_BLOCK_SIZE-(offset%SC_BLOCK_SIZE))
            {
                memcpy(data+(offset%SC_BLOCK_SIZE), buf+(ret_size-size), size);
                size = 0;
            }
            else
            {
                memcpy(data+(offset%SC_BLOCK_SIZE), buf+(ret_size-size), SC_BLOCK_SIZE-(offset%SC_BLOCK_SIZE));
                size = size-SC_BLOCK_SIZE+(offset%SC_BLOCK_SIZE);
            }
            offset = 0;
        }
        else
        {
            break;
        }
        write_block(res, data, SC_BLOCK_SIZE);
    }
    write_inode(cur_inodeid, cur_inode);
    if(cur_inode != NULL)
        free(cur_inode);
    return ret_size;
}

int sc_mkdir(const char *path, mode_t mode)
{
    int ret;
    inodeid_t inodeid;

    ret = find_inode(path, &inodeid);
    if(ret == 0)
        return -EEXIST;

    ret = make_inode(path, &inodeid);
    if(ret != 0)
        return ret;

    ret = change_inode_mode(inodeid, mode|SC_DIR);
    if(ret != 0)
        return ret;
    return 0;
}

int sc_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    (void) fi;
    int ret;
    inodeid_t inodeid;
    ret = make_inode(path, &inodeid);
    if(ret != 0)
        return ret;
    
    ret = change_inode_mode(inodeid, mode|SC_REG);
    if(ret != 0)
        return ret;
    return 0;
}

int sc_chmod(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    (void) fi;
    inodeid_t cur_inodeid;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    ret = change_inode_mode(cur_inodeid, mode);
    if(ret != 0)
        return ret;
    return 0;
}

int sc_rename(const char *from, const char *to, unsigned int flags)
{
    (void) flags;
    inode_st *cur_inode;
    inodeid_t cur_inodeid, to_parent_inodeid;
    int ret, res;

    // 检查from路径和to路径是否存在
    ret = find_inode(to, &cur_inodeid);
    if(ret == 0)
        return -EEXIST;
    
    char to_parent_path[SC_PATH_MAX];
    memset(to_parent_path, 0, sizeof(to_parent_path));
    get_parent_path(to, to_parent_path);
    ret = find_inode(to_parent_path, &to_parent_inodeid);
    if(ret != 0)
        return -ENOENT;
    
    ret = find_inode(from, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    // 从from路径的父级目录中删除
    ret = delete_from_parent(from);
    if(ret != 0)
        return ret;

    // 向to路径的父级目录中加入cur_inodeid
    inodeid_t inodeidres;
    dir_st dir[15];
    char filename[SC_PATH_MAX];
    strcpy(filename, to+strlen(to_parent_path)+(to_parent_path[strlen(to_parent_path)-1]=='/'?0:1));
    if(filename[strlen(filename)-1] == '/')
        filename[strlen(filename)-1] = '\0';
    __inode_add_new_item_to_inode(to_parent_inodeid, filename, &inodeidres);

    cur_inode = read_inode(to_parent_inodeid);
    int ok = 1;
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
            if(!strcmp(dir[j].filename, filename))
            {
                dir[j].inodeid = cur_inodeid;
                write_block(res, dir, sizeof(dir_st)*15);
                free_inode(inodeidres);
                ok = 0;
            }
        }
    }

    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}

int sc_statfs(const char *path, struct statvfs *stbuf)
{
    (void) path;

    superblock_st *superblock = read_superblock();
    stbuf->f_blocks = superblock->block_total;
    stbuf->f_bsize = superblock->block_size;
    stbuf->f_bfree = superblock->block_free;
    stbuf->f_bavail = superblock->block_free;
    stbuf->f_files = superblock->inode_total;
    stbuf->f_ffree = superblock->inode_free;
    stbuf->f_favail = superblock->inode_free;
    stbuf->f_namemax = SC_NAME_MAX;
    if(superblock != NULL)
        free(superblock);
    return 0;
}

int sc_unlink(const char *path)
{
    debug_printf(debug_info, "unlink %s\n", path);
    inodeid_t cur_inodeid;
    inode_st *cur_inode;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -1;

    if((cur_inode->mode&SC_DIR)==SC_DIR)
    {
        ret = -EISDIR;
    }
    else
    {
        ret = delete_from_parent(path);
        if(ret != 0) return ret;
        ret = delete_inode(cur_inodeid, 0);
    }

    if(cur_inode != NULL)
        free(cur_inode);
    return ret;
}

int sc_rmdir(const char *path)
{
    debug_printf(debug_info, "rmdir %s\n", path);
    inodeid_t cur_inodeid;
    inode_st *cur_inode;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -1;

    if((cur_inode->mode&SC_DIR)!=SC_DIR)
    {
        ret = -ENOTDIR;
    }
    else
    {
        ret = delete_from_parent(path);
        if(ret != 0) return ret;
        ret = delete_inode(cur_inodeid, 0);
    }

    if(cur_inode != NULL)
        free(cur_inode);
    return ret;
}

int sc_access(const char *path, int mask)
{
    debug_printf(debug_info, "Call access(path: %s, mask: %d)\n", path, mask);
    inodeid_t cur_inodeid;
    inode_st *cur_inode;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -1;
    
    if(cur_user_id == cur_inode->user)
        mask = mask << 6;
    else if(cur_group_id == cur_inode->group)
        mask = mask << 3;

    ret = -1;
    if((cur_inode->mode&mask)==mask)
        ret = 0;

    return ret;
}
