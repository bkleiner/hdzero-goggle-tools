#pragma once

#include "sunxi-part.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    sunxi_mbr_t mbr;
    FILE *fp;
} sunxi_image_t;

int sunxi_image_open(sunxi_image_t *img, const char *filename);
void sunxi_image_close(sunxi_image_t *img);

sunxi_partition_t *sunxi_image_find_part(sunxi_image_t *img, const char *name);
int sunxi_image_patch_part(sunxi_image_t *img, const char *name, const char *filename);

#ifdef __cplusplus
}
#endif