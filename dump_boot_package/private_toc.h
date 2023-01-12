/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __toc_v2_h
#define __toc_v2_h

#include <stdint.h>

// 增加安全启动下，toc1的头部数据结构
typedef struct sbrom_toc1_head_info
{
	char name[16];	// user can modify
	uint32_t magic; // must equal TOC_U32_MAGIC
	uint32_t add_sum;

	uint32_t serial_num; // user can modify
	uint32_t status;	 // user can modify,such as TOC_MAIN_INFO_STATUS_ENCRYP_NOT_USED

	uint32_t items_nr; // total entry number
	uint32_t valid_len;
	uint32_t version_main; // only one byte
	uint32_t version_sub;  // two bytes
	uint32_t reserved[3];  // reserved for future

	uint32_t end;
} sbrom_toc1_head_info_t;

typedef struct sbrom_toc1_item_info
{
	char name[64]; // such as ITEM_NAME_SBROMSW_CERTIF
	uint32_t data_offset;
	uint32_t data_len;
	uint32_t encrypt;  // 0: no aes   //1: aes
	uint32_t type;	   // 0: normal file, dont care  1: key certif  2: sign certif 3: bin file
	uint32_t run_addr; // if it is a bin file, then run on this address; if not, it should be 0
	uint32_t index;	   // if it is a bin file, this value shows the index to run; if not
	// if it is a certif file, it should equal to the bin file index
	// that they are in the same group
	// it should be 0 when it anyother data type
	uint32_t reserved[69]; // reserved for future;
	uint32_t end;
} sbrom_toc1_item_info_t;

#define ITEM_SCP_NAME "scp"
#define ITEM_MONITOR_NAME "monitor"
#define ITEM_UBOOT_NAME "u-boot"
#define ITEM_LOGO_NAME "logo"
#define ITEM_DTB_NAME "dtb"
#define ITEM_SOCCFG_NAME "soc-cfg"
#define ITEM_BDCFG_NAME "board-cfg"
#define ITEM_SHUTDOWNCHARGE_LOGO_NAME "shutdowncharge"
#define ITEM_ANDROIDCHARGE_LOGO_NAME "androidcharge"

#endif //  ifndef __toc_h

/* end of toc.h */
