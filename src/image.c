/*************************************************************************
    > File Name: image.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: 2019年06月03日 星期一 10时18分14秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "image.h"
#include "debugprintf.h"

char *image_path = NULL;

int open_image(const char *filepath)
{
    image_path = (char *)malloc(sizeof(char)*(strlen(filepath)+1));
    strcpy(image_path, filepath);

    // image_path下文件已存在，该函数结束
    if(exist_image())
        return 0;

    // image文件不存在，新建文件
    int fd = open(image_path, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
    {
        debug_printf(debug_error, "Unable to open image '%s'.\n", image_path);
        return -1;
    }

    // 设置image文件大小
    if(lseek(fd, SC_IMAGE_SIZE-1, SEEK_SET) == -1)
    {
        debug_printf(debug_error, "Unable to create image '%s'.\n", image_path);
        return -1;
    }

    write(fd, "\0", 1);
    close(fd);
    return 0;
}

bool exist_image(void)
{
    if(image_path == NULL)
        return false;
    if(access(image_path, F_OK) != 0)
        return false;
    return true;
}

int close_image(void)
{
    // 释放内存
    if(image_path != NULL)
        free(image_path);
    image_path = NULL;
    return 0;
}

int write_image(sectorid_t sectorid, const void *buf, int len)
{
    if(sectorid >= SC_SECTOR_COUNT)
    {
        debug_printf(debug_error, "Write to image with invalid sectorid '%u'.\n", sectorid);
        return -1;
    }

    if(len < 0||len > SC_SECTOR_SIZE)
    {
        debug_printf(debug_error, "Write to image with invalid len '%d'.\n", len);
        return -1;
    }

    if(!exist_image())
    {
        debug_printf(debug_error, "Image '%s' not found.\n", image_path);
        return -1;
    }

    int fd = open(image_path, O_WRONLY);
    if(fd == -1)
    {
        debug_printf(debug_error, "Unable to open image '%s'.\n", image_path);
        return -1;
    }

    if(lseek(fd, sectorid*SC_SECTOR_SIZE, SEEK_SET) == -1)
    {
        debug_printf(debug_error, "Unable to seek image '%s'.\n", image_path);
        return -1;
    }

    write(fd, buf, len);
    close(fd);
    return 0;
}

int read_image(sectorid_t sectorid, void *buf, int len)
{
    if(sectorid >= SC_SECTOR_COUNT)
    {
        debug_printf(debug_error, "Write to image with invalid sectorid '%u'.\n", sectorid);
        return -1;
    }

    if(len < 0||len > SC_SECTOR_SIZE)
    {
        debug_printf(debug_error, "Write to image with invalid len '%d'.\n", len);
        return -1;
    }

    if(!exist_image())
    {
        debug_printf(debug_error, "Image '%s' not found.\n", image_path);
        return -1;
    }

    int fd = open(image_path, O_RDONLY);
    if(fd == -1)
    {
        debug_printf(debug_error, "Unable to open image '%s'.\n", image_path);
        return -1;
    }

    if(lseek(fd, sectorid*SC_SECTOR_SIZE, SEEK_SET) == -1)
    {
        debug_printf(debug_error, "Unable to seek image '%s'.\n", image_path);
        return -1;
    }

    read(fd, buf, len);
    close(fd);
    return 0;
}
