// SPDX-License-Identifier: GPL-2.0-only
/*
 * v821_wlan/debug.c
 *
 * Copyright (c) 2022
 * Allwinner Technology Co., Ltd. <www.allwinner.com>
 * laumy <liumingyuan@allwinner.com>
 *
 * Debug info APIs for drivers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __XRADIO_DEBUG_H__
#define __XRADIO_DEBUG_H__

#include <linux/kernel.h>
#include <linux/printk.h>

#include "os_dep/os_intf.h"
#include "xradio_platform.h"

//#define DBG_RPMSG 1
//#define DBG_FW    1


/* Message always need to be present even in release version. */
#define XRADIO_DBG_ALWY 0x01

/* Error message to report an error, it can hardly works. */
#define XRADIO_DBG_ERROR 0x02

/* Warning message to inform us of something unnormal or
 * something very important, but it still work. */
#define XRADIO_DBG_WARN 0x04

/* Important message we need to know in unstable version. */
#define XRADIO_DBG_NIY 0x08

/* Normal message just for debug in developing stage. */
#define XRADIO_DBG_MSG 0x10

/* Trace of functions, for sequence of functions called. Normally,
 * don't set this level because there are too more print. */
#define XRADIO_DBG_TRC 0x20

#define XRADIO_DBG_LEVEL 0xFF

/* for host debuglevel*/
extern u8 dbg_common;
extern u8 dbg_cfg;
extern u8 dbg_iface;
extern u8 dbg_plat;
extern u8 dbg_queue;
extern u8 dbg_io;
extern u8 dbg_txrx;
extern u8 dbg_cmd;

static inline void data_hex_dump(const char *prefix, int width,
				 const void *buf, size_t len)
{
	print_hex_dump_debug(prefix, DUMP_PREFIX_NONE, width, 1, buf, len,
			     false);
}

struct xradio_debug_common {
	struct dentry *debugfs_phy;
};

#define xradio_printf(fmt, ...) printk(KERN_ERR fmt, ##__VA_ARGS__)

#define xradio_printk(level, fmt, ...)                                                             \
	do {                                                                                        \
		if ((level) & dbg_common & XRADIO_DBG_ALWY)                                             \
			printk(KERN_CRIT "[%s,%d][XRADIO_ALWY] " fmt, __func__, __LINE__, ##__VA_ARGS__);   \
		else if ((level) & dbg_common & XRADIO_DBG_ERROR)                                       \
			printk(KERN_ERR "[%s,%d][XRADIO_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);     \
		else if ((level) & dbg_common & XRADIO_DBG_WARN)                                        \
			printk(KERN_WARNING "[%s,%d][XRADIO_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__); \
		else if ((level) & dbg_common)                                                          \
			printk(KERN_DEBUG "[%s,%d][XRADIO] " fmt, __func__, __LINE__, ##__VA_ARGS__);       \
	} while (0)

#define cfg_printk(level, fmt, ...)                                                             \
	do {                                                                                        \
		if ((level) & dbg_cfg & XRADIO_DBG_ERROR)                                               \
			printk(KERN_ERR "[%s,%d][CFG_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);        \
		else if ((level) & dbg_cfg & XRADIO_DBG_WARN)                                           \
			printk(KERN_WARNING "[%s,%d][CFG_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__);    \
		else if ((level) & dbg_cfg)                                                             \
			printk(KERN_DEBUG "[%s,%d][CFG] " fmt, __func__, __LINE__, ##__VA_ARGS__);          \
	} while (0)

#define iface_printk(level, fmt, ...)                                                           \
	do {                                                                                        \
		if ((level) & dbg_iface & XRADIO_DBG_ERROR)                                             \
			printk(KERN_ERR "[%s,%d][IFACE_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);      \
		else if ((level) & dbg_iface & XRADIO_DBG_WARN)                                         \
			printk(KERN_WARNING "[%s,%d][IFACE_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__);  \
		else if ((level) & dbg_iface)                                                           \
			printk(KERN_DEBUG "[%s,%d][IFACE] " fmt, __func__, __LINE__, ##__VA_ARGS__);        \
	} while (0)

#define plat_printk(level, fmt, ...)                                                            \
	do {                                                                                        \
		if ((level) & dbg_plat & XRADIO_DBG_ERROR)                                              \
			printk(KERN_ERR "[%s,%d][PLAT_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);       \
		else if ((level) & dbg_plat & XRADIO_DBG_WARN)                                          \
			printk(KERN_WARNING "[%s,%d][PLAT_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__);   \
		else if ((level) & dbg_plat)                                                            \
			printk(KERN_DEBUG "[%s,%d][PLAT] " fmt, __func__, __LINE__, ##__VA_ARGS__);         \
	} while (0)

#define queue_printk(level, fmt, ...)                                                           \
	do {                                                                                        \
		if ((level) & dbg_queue & XRADIO_DBG_ERROR)                                             \
			printk(KERN_ERR "[%s,%d][QUEUE_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);      \
		else if ((level) & dbg_queue & XRADIO_DBG_WARN)                                         \
			printk(KERN_WARNING "[%s,%d][QUEUE_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__);  \
		else if ((level) & dbg_queue)                                                           \
			printk(KERN_DEBUG "[%s,%d][QUEUE] " fmt, __func__, __LINE__, ##__VA_ARGS__);        \
	} while (0)

#define io_printk(level, fmt, ...)                                                              \
	do {                                                                                        \
		if ((level) & dbg_io & XRADIO_DBG_ERROR)                                                \
			printk(KERN_ERR "[%s,%d][IO_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);         \
		else if ((level) & dbg_io & XRADIO_DBG_WARN)                                            \
			printk(KERN_WARNING "[%s,%d][IO_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__);     \
		else if ((level) & dbg_io)                                                              \
			printk(KERN_DEBUG "[%s,%d][IO] " fmt, __func__, __LINE__, ##__VA_ARGS__);           \
	} while (0)

#define txrx_printk(level, fmt, ...)                                                            \
	do {                                                                                        \
		if ((level) & dbg_txrx & XRADIO_DBG_ERROR)                                              \
			printk(KERN_ERR "[%s,%d][TXRX_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);       \
		else if ((level) & dbg_txrx & XRADIO_DBG_WARN)                                          \
			printk(KERN_WARNING "[%s,%d][TXRX_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__);   \
		else if ((level) & dbg_txrx)                                                            \
			printk(KERN_DEBUG "[%s,%d][TXRX] " fmt, __func__, __LINE__, ##__VA_ARGS__);         \
	} while (0)

#define cmd_printk(level, fmt, ...)                                                             \
	do {                                                                                        \
		if ((level) & dbg_cmd & XRADIO_DBG_ERROR)                                               \
			printk(KERN_ERR "[%s,%d][CMD_ERR] " fmt, __func__, __LINE__, ##__VA_ARGS__);       \
		else if ((level) & dbg_cmd & XRADIO_DBG_WARN)                                           \
			printk(KERN_WARNING "[%s,%d][CMD_WRN] " fmt, __func__, __LINE__, ##__VA_ARGS__);   \
		else if ((level) & dbg_cmd)                                                            \
			printk(KERN_DEBUG "[%s,%d][CMD] " fmt, __func__, __LINE__, ##__VA_ARGS__);         \
	} while (0)

int xradio_debug_init_common(struct xradio_hw *xradio_hw);
void xradio_debug_deinit_common(struct xradio_hw *xradio_hw);
#endif
