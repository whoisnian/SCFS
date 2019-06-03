#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>
#include "definition.h"

int create_image(char *filepath);
bool exist_image(char *filepath);
int write_image();
int read_image();

#endif // IMAGE_H
