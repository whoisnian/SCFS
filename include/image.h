#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>
#include "definition.h"

extern char *image_path;

int open_image(char *filepath);
int close_image(void);
bool exist_image(void);
int write_image(blockid_t blockid, const void *buf, int len);
int read_image(blockid_t blockid, void *buf, int len);

#endif // IMAGE_H
