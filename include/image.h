#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>
#include "definition.h"

// image文件路径
extern char *image_path;

// 打开image（存储image路径到image_path中进行共享），如果image不存在则会在指定路径新建image文件
int open_image(const char *filepath);

// 判断image_path指定的文件是否存在
bool exist_image(void);

// 关闭image（释放open_image()中给image_path分配的内存）
int close_image(void);

// 向image指定扇区写入数据
int write_image(sectorid_t sectorid, const void *buf, int len);

// 从image指定扇区读取数据
int read_image(sectorid_t sectorid, void *buf, int len);

#endif // IMAGE_H
