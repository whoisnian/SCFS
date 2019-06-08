/*************************************************************************
    > File Name: block.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月09日 星期日 03时11分14秒
 ************************************************************************/
#include "block.h"

int write_block(blockid_t blockid, const void *buf, int len)
{
    return write_image(blockid, buf, len);
}

int read_block(blockid_t blockid, void *buf, int len)
{
    return read_image(blockid, buf, len);
}