/*************************************************************************
    > File Name: image.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: Mon Jun  3 10:18:14 2019
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "image.h"

char *image_path = NULL;

int open_image(char *filepath)
{
    image_path = (char *)malloc(strlen(filepath)*sizeof(char));
    strcpy(image_path, filepath);
    if(exist_image())
        return 0;

    int fd = open(image_path, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
    {
        printf("Unable to open image '%s'.\n", image_path);
        return -1;
    }

    if(lseek(fd, SC_IMAGE_SIZE-1, SEEK_SET) == -1)
    {
        printf("Unable to create image '%s'.\n", image_path);
        return -1;
    }
    write(fd, "\0", 1);
    close(fd);
    return 0;
}

int close_image(void)
{
    free(image_path);
    image_path = NULL;
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

int write_image(blockid_t blockid, const void *buf, int len)
{
    if(blockid >= SC_BLOCK_COUNT)
    {
        printf("Invalid blockid '%u'.\n", blockid);
        return -1;
    }

    if(len < 0||len > SC_BLOCK_SIZE)
    {
        printf("Invalid len '%d'.\n", len);
        return -1;
    }

    if(!exist_image())
    {
        printf("Image '%s' not found.\n", image_path);
        return -1;
    }

    int fd = open(image_path, O_WRONLY);
    if(fd == -1)
    {
        printf("Unable to open image '%s'.\n", image_path);
        return -1;
    }

    if(lseek(fd, blockid*SC_BLOCK_SIZE, SEEK_SET) == -1)
    {
        printf("Unable to seek image '%s'.\n", image_path);
        return -1;
    }

    write(fd, buf, len);
    close(fd);
    return 0;
}

int read_image(blockid_t blockid, void *buf, int len)
{
    if(blockid >= SC_BLOCK_COUNT)
    {
        printf("Invalid blockid '%u'.\n", blockid);
        return -1;
    }

    if(len < 0||len > SC_BLOCK_SIZE)
    {
        printf("Invalid len '%d'.\n", len);
        return -1;
    }

    if(!exist_image())
    {
        printf("Image '%s' not found.\n", image_path);
        return -1;
    }

    int fd = open(image_path, O_RDONLY);
    if(fd == -1)
    {
        printf("Unable to open image '%s'.\n", image_path);
        return -1;
    }

    if(lseek(fd, blockid*SC_BLOCK_SIZE, SEEK_SET) == -1)
    {
        printf("Unable to seek image '%s'.\n", image_path);
        return -1;
    }

    read(fd, buf, len);
    close(fd);
    return 0;
}