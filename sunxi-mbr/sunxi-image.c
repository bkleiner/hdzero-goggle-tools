#include "sunxi-image.h"

#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE (1 * 1024 * 1024)

int sunxi_image_open(sunxi_image_t *img, const char *filename) {
    img->fp = fopen(filename, "r+b");
    if (!img->fp) {
        return -1;
    }

    fseek(img->fp, SUNXI_BOOTLOADER_SIZE, SEEK_SET);
    fread(&img->mbr, SUNXI_MBR_SIZE, 1, img->fp);

    if (strncmp(img->mbr.magic, SUNXI_MBR_MAGIC, 8) != 0) {
        return -1;
    }

    return 0;
}

void sunxi_image_close(sunxi_image_t *img) {
    fclose(img->fp);
}

sunxi_partition_t *sunxi_image_find_part(sunxi_image_t *img, const char *name) {
    for (size_t i = 0; i < img->mbr.part_count; i++) {
        if (strcmp(img->mbr.array[i].name, name) == 0) {
            return &img->mbr.array[i];
        }
    }
    return NULL;
}

int sunxi_image_patch_part(sunxi_image_t *img, const char *name, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    size_t patch_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    sunxi_partition_t *part = sunxi_image_find_part(img, name);
    if (!part) {
        fclose(fp);
        return -1;
    }

    size_t part_size = part->lenlo * SUNXI_MBR_PAGE_SIZE;
    if (patch_file_size > part_size) {
        fclose(fp);
        return -1;
    }

    size_t offset = SUNXI_BOOTLOADER_SIZE + part->addrlo * SUNXI_MBR_PAGE_SIZE;
    fseek(img->fp, offset, SEEK_SET);

    size_t written = 0;
    char buf[BUFFER_SIZE];
    while (written < patch_file_size) {
        size_t read = fread(buf, 1, BUFFER_SIZE, fp);
        if (read == 0) {
            break;
        }

        fwrite(buf, read, 1, img->fp);
        written += read;
    }

    fclose(fp);
    return 0;
}