#include "private_toc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    FILE *f = fopen("boot_package.fex", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(fsize);
    fread(buf, fsize, 1, f);
    fclose(f);

    sbrom_toc1_head_info_t *head = (sbrom_toc1_head_info_t *)buf;
    sbrom_toc1_item_info_t *item = (sbrom_toc1_item_info_t *)(buf + sizeof(sbrom_toc1_head_info_t));

    for (uint32_t i = 0; i < head->items_nr; i++)
    {
        char filename[255];
        strcpy(filename, item->name);
        strcat(filename, ".fex");

        printf("%s\n", filename);

        FILE *w = fopen(filename, "wb");
        fwrite(buf + item->data_offset, item->data_len, 1, w);
        fclose(w);

        item++;
    }

    free(buf);

    return 0;
}
