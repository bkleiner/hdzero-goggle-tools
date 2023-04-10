#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SUNXI_MBR_SIZE        (16 * 1024)
#define SUNXI_BOOTLOADER_SIZE 1032192

#define SUNXI_MBR_MAGIC          "softw411"
#define SUNXI_MBR_MAX_PART_COUNT 120
#define SUNXI_MBR_COPY_NUM       4
#define SUNXI_MBR_PAGE_SIZE      512

#define SUNXI_MBR_RESERVED       \
    (SUNXI_MBR_SIZE - 32 - 4 -   \
     (SUNXI_MBR_MAX_PART_COUNT * \
      sizeof(sunxi_partition_t))) // mbr保留的空间

#define SUNXI_NOLOCK    (0)
#define SUNXI_LOCKING   (0xAA)
#define SUNXI_RELOCKING (0xA0)
#define SUNXI_UNLOCK    (0xA5)

typedef struct __attribute__((packed)) {
    unsigned int addrhi;
    unsigned int addrlo;
    unsigned int lenhi;
    unsigned int lenlo;
    char classname[16];
    char name[16];
    unsigned int user_type;
    unsigned int keydata;
    unsigned int ro;
    unsigned int sig_verify;
    unsigned int sig_erase;
    unsigned int sig_value[4];
    unsigned int sig_pubkey;
    unsigned int sig_pbumode;
    unsigned char reserved2[36];
} sunxi_partition_t;

typedef struct __attribute__((packed)) {
    unsigned int crc32;
    unsigned int version;
    char magic[8];
    unsigned int copy;
    unsigned int index;
    unsigned int part_count;
    unsigned int stamp;
    sunxi_partition_t array[SUNXI_MBR_MAX_PART_COUNT];
    unsigned int lockflag;
    unsigned char res[SUNXI_MBR_RESERVED];
} sunxi_mbr_t;

#ifdef __cplusplus
}
#endif