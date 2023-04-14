#pragma once

#include <string>

#define MEM_ALIGN(offset, size) (((offset) + ((size)-1)) & -(size))

static std::string extract_string(const char *str, size_t max_len) {
    size_t len = max_len;
    for (size_t i = 0; i < max_len; i++) {
        if (str[i] == 0) {
            len = i;
            break;
        }
    }
    return std::string(str, str + len);
}

static void insert_string(char *dst, const std::string &str, size_t max_len) {
    for (size_t i = 0; i < max_len; i++) {
        if (i < str.size()) {
            dst[i] = str[i];
        } else {
            dst[i] = 0;
        }
    }
}

static uint32_t crc32_calc(uint8_t *buf, uint32_t size) {
    uint32_t table[256];
    for (size_t i = 0; i < 256; i++) {
        uint32_t val = i;
        for (size_t j = 0; j < 8; j++) {
            if (val & 1) {
                val = (val >> 1) ^ 0xEDB88320;
            } else {
                val >>= 1;
            }
        }
        table[i] = val;
    }

    uint32_t crc = 0xffffffff;
    for (size_t i = 0; i < size; i++) {
        crc = table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }
    return crc ^ 0xffffffff;
}