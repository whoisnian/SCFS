#ifndef INODE_H
#define INODE_H

typedef struct inode_st
{
    int privilege;
    int user;
    int group;
    int size;
    int ctime;
    int atime;
    int mtime;
    int block_id;
}inode_st;

#endif // INODE_H
