/*************************************************************************
    > File Name: image.c
    > Author: nian
    > Blog: https://whoisnian.com
    > Mail: zhuchangbao1998@gmail.com
    > Created Time: Mon Jun  3 10:18:14 2019
 ************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include "image.h"

int create_image(char *filepath)
{
    if(access(filepath, F_OK) == 0)
    {
        printf("Image '%s' already exists.\n", filepath);
        return -1;
    }

    FILE *fp = fopen(filepath, "w");
    if(fp == NULL)
    {
        printf("Unable to open image '%s'.\n", filepath);
        return -1;
    }

    if(fseek(fp, SC_IMAGE_SIZE-1, SEEK_SET) == -1)
    {
        printf("Unable to create image '%s'.\n", filepath);
        return -1;
    }
    fputc('\0', fp);
    fclose(fp);
    return 0;
}

bool exist_image(char *filepath)
{
    if(access(filepath, F_OK) == 0)
        return true;
    else
        return false;
}
