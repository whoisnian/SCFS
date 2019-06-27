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
#include "script.h"
#include "debugprintf.h"

unsigned int cur_user_id = SC_GUEST_USER_ID;
unsigned int cur_group_id = SC_GUEST_GROUP_ID;
int temp_root = 0;

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
    temp_root = 1;
    if(sc_access("/.passwd", SC_F_OK) != 0)
    {
        sc_create("/.passwd", SC_REG|SC_USR_R|SC_USR_W|SC_GRP_R|SC_OTH_R, NULL);
        sc_chown("/.passwd", 0, 0, NULL);
        // username:x:uid:gid:::
        // root:x:0:0:::
        memset(tmp, 0, sizeof(tmp));
        strcpy(tmp, "root:x:0:0:::\n");
        sc_write("/.passwd", tmp, strlen(tmp), 0, NULL);
    }
    if(sc_access("/.group", SC_F_OK) != 0)
    {
        sc_create("/.group", SC_REG|SC_USR_R|SC_USR_W|SC_GRP_R|SC_OTH_R, NULL);
        sc_chown("/.group", 0, 0, NULL);
        // groupname:x:gid:
        // root:x:0:
        memset(tmp, 0, sizeof(tmp));
        strcpy(tmp, "root:x:0:\n");
        sc_write("/.group", tmp, strlen(tmp), 0, NULL);
    }
    if(sc_access("/.shadow", SC_F_OK) != 0)
    {
        sc_create("/.shadow", SC_REG|SC_USR_R|SC_USR_W, NULL);
        sc_chown("/.shadow", 0, 0, NULL);
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
        sc_chown("/.run_command", 0, 0, NULL);
    }
    if(sc_access("/tmp", SC_F_OK) != 0)
    {
        sc_mkdir("/tmp", SC_DIR|SC_USR_ALL|SC_GRP_ALL|SC_OTH_ALL);
        sc_chown("/tmp", 0, 0, NULL);
    }
    if(sc_access("/bin", SC_F_OK) != 0)
    {
        sc_mkdir("/bin", SC_DEFAULT_DIR);
        sc_chown("/bin", 0, 0, NULL);

        // 登录脚本
        sc_create("/bin/login", SC_REG|SC_R_ALL|SC_X_ALL, NULL);
        sc_chown("/bin/login", 0, 0, NULL);
        sc_write("/bin/login", sc_script_login, strlen(sc_script_login), 0, NULL);

        // 新建用户脚本
        sc_create("/bin/useradd", SC_REG|SC_R_ALL|SC_X_ALL, NULL);
        sc_chown("/bin/useradd", 0, 0, NULL);
        sc_write("/bin/useradd", sc_script_login, strlen(sc_script_useradd), 0, NULL);

        // 删除用户脚本
        sc_create("/bin/userdel", SC_REG|SC_R_ALL|SC_X_ALL, NULL);
        sc_chown("/bin/userdel", 0, 0, NULL);
        sc_write("/bin/userdel", sc_script_login, strlen(sc_script_userdel), 0, NULL);

        // 修改密码脚本
        sc_create("/bin/passwd", SC_REG|SC_R_ALL|SC_X_ALL, NULL);
        sc_chown("/bin/passwd", 0, 0, NULL);
        sc_write("/bin/passwd", sc_script_login, strlen(sc_script_passwd), 0, NULL);

        // 新建用户组脚本
        sc_create("/bin/groupadd", SC_REG|SC_R_ALL|SC_X_ALL, NULL);
        sc_chown("/bin/groupadd", 0, 0, NULL);
        sc_write("/bin/groupadd", sc_script_login, strlen(sc_script_groupadd), 0, NULL);

        // 删除用户组脚本
        sc_create("/bin/groupdel", SC_REG|SC_R_ALL|SC_X_ALL, NULL);
        sc_chown("/bin/groupdel", 0, 0, NULL);
        sc_write("/bin/groupdel", sc_script_login, strlen(sc_script_groupdel), 0, NULL);

        // 修改用户所属用户组
        sc_create("/bin/gpasswd", SC_REG|SC_R_ALL|SC_X_ALL, NULL);
        sc_chown("/bin/gpasswd", 0, 0, NULL);
        sc_write("/bin/gpasswd", sc_script_login, strlen(sc_script_gpasswd), 0, NULL);
    }
    temp_root = 0;

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
    debug_printf(debug_info, "%u %u run_command %s\n", cur_user_id, cur_group_id, cmd);
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

    temp_root = 1;
    int ret = sc_read("/.shadow", buf, 4096, 0, NULL);
    temp_root = 0;
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
                    debug_printf(debug_info, "Login as %s\n", username);
                    return 0;
                }
                subbuf2 = strtok(NULL, "\n");
            }
            return -1;
        }
        subbuf = strtok(NULL, "\n");
    }
    return -EACCES;
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
        return -EACCES;
    
    memset(buf, 0, sizeof(buf));
    temp_root = 1;
    int ret = sc_read("/.shadow", buf, 4096, 0, NULL);
    temp_root = 0;
    if(ret < 0) return ret;

    char userpass[4096];
    memset(userpass, 0, sizeof(userpass));
    strcpy(userpass, username);
    strcat(userpass, ":");
    
    char buf2[4096], *buf3, *buf4;
    memset(buf2, 0, sizeof(buf2));
    buf3 = strstr(buf, userpass);
    if(buf3 != NULL)
    {
    buf4 = strchr(buf3, '\n');
    strncpy(buf2, buf, buf3-buf+strlen(userpass));
    strcat(buf2, password);
    strcat(buf2, "\n");
    strcat(buf2, buf4+1);
    }
    else
    {
        strcpy(buf2, buf);
        strcat(buf2, userpass);
        strcat(buf2, password);
        strcat(buf2, "\n");
    }

    temp_root = 1;
    sc_write("/.shadow", buf2, 4096, 0, NULL);
    temp_root = 0;

    return 0;
}

int command_useradd(const char *username)
{
    if(username == NULL) return -1;
    char buf[4096], buf2[4096], *subbuf;
    int ret;
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));

    ret = sc_read("/.passwd", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    unsigned int uid, maxuid = 2000;
    subbuf = strtok(buf, "\n");
    while(subbuf != NULL)
    {
        if(!strncmp(username, subbuf, strlen(username)))
        {
            return -1;
        }
        sscanf(strchr(subbuf, ':'), ":x:%u:%*u:::", &uid);
        if(uid > maxuid)
            maxuid = uid;
        subbuf = strtok(NULL, "\n");
    }
    maxuid++;

    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    sprintf(buf2, "%s:x:%u:%u:::\n", username, maxuid, maxuid);
    ret = sc_read("/.passwd", buf, 4096, 0, NULL);
    if(ret < 0) return ret;
    strcat(buf, buf2);
    temp_root = 1;
    sc_write("/.passwd", buf, 4096, 0, NULL);
    temp_root = 0;

    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    sprintf(buf2, "%s:x:%u:\n", username, maxuid);
    ret = sc_read("/.group", buf, 4096, 0, NULL);
    if(ret < 0) return ret;
    strcat(buf, buf2);
    temp_root = 1;
    sc_write("/.group", buf, 4096, 0, NULL);
    temp_root = 0;

    return 0;
}

int command_userdel(const char *username)
{
    if(username == NULL) return -1;
    if(!strcmp(username, "root")) return -1;
    char buf[4096], buf2[4096], *buf3, *buf4;
    int ret;
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));

    ret = sc_read("/.passwd", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    char userpass[4096];
    memset(userpass, 0, sizeof(userpass));
    strcpy(userpass, username);
    strcat(userpass, ":");

    buf3 = strstr(buf, userpass);
    if(buf3 == NULL) return -1;
    buf4 = strchr(buf3, '\n');

    strncpy(buf2, buf, buf3-buf);
    strcat(buf2, buf4+1);
    temp_root = 1;
    sc_write("/.passwd", buf2, 4096, 0, NULL);
    temp_root = 0;

    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    temp_root = 1;
    ret = sc_read("/.shadow", buf, 4096, 0, NULL);
    temp_root = 0;
    if(ret < 0) return ret;

    buf3 = strstr(buf, userpass);
    if(buf3 == NULL) return -1;
    buf4 = strchr(buf3, '\n');

    strncpy(buf2, buf, buf3-buf);
    strcat(buf2, buf4+1);

    temp_root = 1;
    sc_write("/.shadow", buf2, 4096, 0, NULL);
    temp_root = 0;
    return 0;
}

int command_groupadd(const char *groupname)
{
    if(groupname == NULL) return -1;
    char buf[4096], buf2[4096], *subbuf;
    int ret;
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));

    ret = sc_read("/.group", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    unsigned int gid, maxgid = 3000;
    subbuf = strtok(buf, "\n");
    while(subbuf != NULL)
    {
        if(!strncmp(groupname, subbuf, strlen(groupname)))
        {
            return -1;
        }
        sscanf(strchr(subbuf, ':'), ":x:%u:", &gid);
        if(gid > maxgid)
            maxgid = gid;
        subbuf = strtok(NULL, "\n");
    }
    maxgid++;

    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    sprintf(buf2, "%s:x:%u:\n", groupname, maxgid);
    ret = sc_read("/.group", buf, 4096, 0, NULL);
    if(ret < 0) return ret;
    strcat(buf, buf2);
    temp_root = 1;
    sc_write("/.group", buf, 4096, 0, NULL);
    temp_root = 0;
    return 0;
}

int command_groupdel(const char *groupname)
{
    if(groupname == NULL) return -1;
    if(!strcmp(groupname, "root")) return -1;
    char buf[4096], buf2[4096], *buf3, *buf4;
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));

    int ret = sc_read("/.group", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    char userpass[4096];
    memset(userpass, 0, sizeof(userpass));
    strcpy(userpass, groupname);
    strcat(userpass, ":");

    buf3 = strstr(buf, userpass);
    if(buf3 == NULL) return -1;
    buf4 = strchr(buf3, '\n');

    strncpy(buf2, buf, buf3-buf);
    strcat(buf2, buf4+1);
    temp_root = 1;
    sc_write("/.group", buf2, 4096, 0, NULL);
    temp_root = 0;
    return 0;
}

int command_gpasswd(const char *username, const char *groupname)
{
    if(username == NULL||groupname == NULL) return -1;
    char buf[4096], buf2[4096], *subbuf;
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));

    int ret = sc_read("/.group", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    unsigned int gid = 1;
    subbuf = strtok(buf, "\n");
    while(subbuf != NULL)
    {
        if(!strncmp(groupname, subbuf, strlen(groupname)))
        {
            sscanf(strchr(subbuf, ':'), ":x:%u:", &gid);
            break;
        }
        subbuf = strtok(NULL, "\n");
    }
    if(gid == 1) return -1;

    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    ret = sc_read("/.passwd", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    unsigned int uid = 1, gid2 = 1;
    subbuf = strtok(buf, "\n");
    while(subbuf != NULL)
    {
        if(!strncmp(username, subbuf, strlen(username)))
        {
            sscanf(strchr(subbuf, ':'), ":x:%u:%u:::", &uid, &gid2);
            break;
        }
        subbuf = strtok(NULL, "\n");
    }
    if(uid == 1) return -1;
    if(gid2 == gid) return 0;

    memset(buf, 0, sizeof(buf));
    ret = sc_read("/.passwd", buf, 4096, 0, NULL);
    if(ret < 0) return ret;

    char userpass[4096];
    memset(userpass, 0, sizeof(userpass));
    strcpy(userpass, username);
    strcat(userpass, ":");
    
    char *buf3, *buf4;
    memset(buf2, 0, sizeof(buf2));
    buf3 = strstr(buf, userpass);
    if(buf3 == NULL) return -1;
    buf4 = strchr(buf3, '\n');
    strncpy(buf2, buf, buf3-buf+strlen(userpass));
    sprintf(userpass, "x:%u:%u:::\n", uid, gid);
    strcat(buf2, userpass);
    strcat(buf2, buf4+1);

    temp_root = 1;
    sc_write("/.passwd", buf2, 4096, 0, NULL);
    temp_root = 0;
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
    debug_printf(debug_info, "Call sc_getattr(path %s, buf, fi)\n", path);
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
    debug_printf(debug_info, "Call sc_readdir(path %s, buf, filler, offset, fi, flags)\n", path);
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
    
    if(cur_user_id != 0&&temp_root != 1)
    {
        unsigned int mask = SC_R_OK;
        if(cur_user_id == cur_inode->user)
            mask = mask << 6;
        else if(cur_group_id == cur_inode->group)
            mask = mask << 3;
        if((cur_inode->mode&mask)!=mask)
            return -EACCES;
    }

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
    debug_printf(debug_info, "Call sc_open(path %s, fi)\n", path);
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
    debug_printf(debug_info, "Call sc_read(path %s, buf, size %u, offset %u, fi)\n", path, size, offset);
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

    if(cur_user_id != 0&&temp_root != 1)
    {
        unsigned int mask = SC_R_OK;
        if(cur_user_id == cur_inode->user)
            mask = mask << 6;
        else if(cur_group_id == cur_inode->group)
            mask = mask << 3;
        if((cur_inode->mode&mask)!=mask)
            return -EACCES;
    }

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
    debug_printf(debug_info, "Call sc_write(path %s, buf, size %u, offset %u, fi)\n", path, size, offset);
    int ret;
    if(!strcmp(path, "/.run_command"))
    {
        char cmd[4096];
        memset(cmd, 0, sizeof(cmd));
        strncpy(cmd, buf, size);
        ret = run_command(cmd);
        if(ret == 0)
            return size;
        else
            return ret;
    }

    (void) fi;
    int res;
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

    if(cur_user_id != 0&&temp_root != 1)
    {
        unsigned int mask = SC_W_OK;
        if(cur_user_id == cur_inode->user)
            mask = mask << 6;
        else if(cur_group_id == cur_inode->group)
            mask = mask << 3;
        if((cur_inode->mode&mask)!=mask)
            return -EACCES;
    }

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
    debug_printf(debug_info, "Call sc_mkdir(path %s, mode %o)\n", path, mode);
    int ret;
    inodeid_t inodeid;

    ret = find_inode(path, &inodeid);
    if(ret == 0)
        return -EEXIST;

    char check_parent_path[SC_PATH_MAX];
    memset(check_parent_path, 0, sizeof(check_parent_path));
    get_parent_path(path, check_parent_path);
    if(sc_access(check_parent_path, SC_W_OK) != 0)
        return -EACCES;

    ret = make_inode(path, &inodeid);
    if(ret != 0)
        return ret;

    inode_st *inode = read_inode(inodeid);
    inode->mode = SC_DIR|mode;
    inode->user = cur_user_id;
    inode->group = cur_group_id;

    ret = write_inode(inodeid, inode);
    if(ret != 0)
        return ret;
    if(inode != NULL)
        free(inode);
    return 0;
}

int sc_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    debug_printf(debug_info, "Call sc_create(path %s, mode %o, fi)\n", path, mode);
    (void) fi;
    int ret;

    char check_parent_path[SC_PATH_MAX];
    memset(check_parent_path, 0, sizeof(check_parent_path));
    get_parent_path(path, check_parent_path);
    if(sc_access(check_parent_path, SC_W_OK) != 0)
        return -EACCES;

    inodeid_t inodeid;
    ret = make_inode(path, &inodeid);
    if(ret != 0)
        return ret;
    
    inode_st *inode = read_inode(inodeid);
    inode->mode = SC_REG|mode;
    inode->user = cur_user_id;
    inode->group = cur_group_id;

    ret = write_inode(inodeid, inode);
    if(ret != 0)
        return ret;
    if(inode != NULL)
        free(inode);
    return 0;
}

int sc_chmod(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    debug_printf(debug_info, "Call sc_chmod(path %s, mode %o, fi)\n", path, mode);
    (void) fi;
    inodeid_t cur_inodeid;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;
    
    inode_st *cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    if(cur_user_id != 0&&temp_root != 1)
    {
        unsigned int mask = SC_W_OK;
        if(cur_user_id == cur_inode->user)
            mask = mask << 6;
        else if(cur_group_id == cur_inode->group)
            mask = mask << 3;
        if((cur_inode->mode&mask)!=mask)
            return -EACCES;
    }

    cur_inode->mode = mode;
    cur_inode->user = cur_user_id;
    cur_inode->group = cur_group_id;

    ret = write_inode(cur_inodeid, cur_inode);
    if(ret != 0)
        return ret;
    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}

int sc_rename(const char *from, const char *to, unsigned int flags)
{
    debug_printf(debug_info, "Call sc_rename(from %s, to %s, flags)\n", from, to);
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

    char check_parent_path[SC_PATH_MAX];
    memset(check_parent_path, 0, sizeof(check_parent_path));
    get_parent_path(from, check_parent_path);
    if(sc_access(check_parent_path, SC_W_OK) != 0)
        return -EACCES;
    if(sc_access(to_parent_path, SC_W_OK) != 0)
        return -EACCES;
    
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
    debug_printf(debug_info, "Call sc_statfs(path %s, mstbuf)\n", path);
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
    debug_printf(debug_info, "Call sc_unlink(path %s)\n", path);
    inodeid_t cur_inodeid;
    inode_st *cur_inode;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    char check_parent_path[SC_PATH_MAX];
    memset(check_parent_path, 0, sizeof(check_parent_path));
    get_parent_path(path, check_parent_path);
    if(sc_access(check_parent_path, SC_W_OK) != 0)
        return -EACCES;

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
    debug_printf(debug_info, "Call sc_rmdir(path %s)\n", path);
    inodeid_t cur_inodeid;
    inode_st *cur_inode;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;

    char check_parent_path[SC_PATH_MAX];
    memset(check_parent_path, 0, sizeof(check_parent_path));
    get_parent_path(path, check_parent_path);
    if(sc_access(check_parent_path, SC_W_OK) != 0)
        return -EACCES;

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
    debug_printf(debug_info, "Call sc_access(path %s, mask %d)\n", path, mask);
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
    if(ret == -1&&(cur_user_id == 0||temp_root == 1)&&mask != 0)
        ret = 0;
    return ret;
}

int sc_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    debug_printf(debug_info, "Call sc_chown(path %s, uid %u, gid %u, fi)\n", path, uid, gid);
    (void) fi;
    inodeid_t cur_inodeid;
    int ret;

    ret = find_inode(path, &cur_inodeid);
    if(ret != 0)
        return -ENOENT;
    
    inode_st *cur_inode = read_inode(cur_inodeid);
    if(cur_inode == NULL)
        return -ENOENT;

    if(cur_user_id != 0&&temp_root != 1)
    {
        unsigned int mask = SC_W_OK;
        if(cur_user_id == cur_inode->user)
            mask = mask << 6;
        else if(cur_group_id == cur_inode->group)
            mask = mask << 3;
        if((cur_inode->mode&mask)!=mask)
            return -EACCES;
    }

    cur_inode->user = uid;
    cur_inode->group = gid;

    write_inode(cur_inodeid, cur_inode);

    if(cur_inode != NULL)
        free(cur_inode);
    return 0;
}
