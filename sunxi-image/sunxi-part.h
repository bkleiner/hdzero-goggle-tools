#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SUNXI_MAGIC          "softw411"
#define SUNXI_MAX_PART_COUNT 120
#define SUNXI_PAGE_SIZE      512

#define SUNXI_MBR_SIZE (16 * 1024)
#define SUNXI_DL_SIZE  (16 * 1024)

#define SUNXI_MBR_RESERVED (SUNXI_MBR_SIZE - 32 - 4 - (SUNXI_MAX_PART_COUNT * sizeof(sunxi_mbr_partition_t)))
#define SUNXI_DL_RESERVED  (SUNXI_DL_SIZE - 32 - (SUNXI_MAX_PART_COUNT * sizeof(sunxi_dl_partition_t)))

typedef struct __attribute__((packed)) {
    uint32_t addrhi;
    uint32_t addrlo;
    uint32_t lenhi;
    uint32_t lenlo;
    char classname[16];
    char name[16];
    uint32_t user_type;
    uint32_t keydata;
    uint32_t ro;
    uint32_t sig_verify;
    uint32_t sig_erase;
    uint32_t sig_value[4];
    uint32_t sig_pubkey;
    uint32_t sig_pbumode;
    uint8_t reserved2[36];
} sunxi_mbr_partition_t;

typedef struct __attribute__((packed)) {
    uint32_t crc32;
    uint32_t version;
    char magic[8];
    uint32_t copy;
    uint32_t index;
    uint32_t part_count;
    uint32_t stamp;
    sunxi_mbr_partition_t array[SUNXI_MAX_PART_COUNT];
    uint32_t lockflag;
    uint8_t res[SUNXI_MBR_RESERVED];
} sunxi_mbr_t;

typedef struct __attribute__((packed)) {
    char name[16];
    uint32_t addrhi;
    uint32_t addrlo;
    uint32_t lenhi;
    uint32_t lenlo;
    char dl_name[16];
    char vf_name[16];
    uint32_t encrypt;
    uint32_t verify;
} sunxi_dl_partition_t;

typedef struct __attribute__((packed)) {
    uint32_t crc32;
    uint32_t version;
    char magic[8];
    uint32_t download_count;
    uint32_t stamp[3];
    sunxi_dl_partition_t array[SUNXI_MAX_PART_COUNT];
    uint8_t res[SUNXI_DL_RESERVED];
} sunxi_dl_t;

#ifdef __cplusplus
}
#endif