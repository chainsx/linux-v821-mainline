// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Allwinner Technology Co., Ltd.
 */

#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

#include "debug.h"
#include "xr_version.h"

u8 dbg_common = XRADIO_DBG_ALWY | XRADIO_DBG_ERROR;
u8 dbg_cfg = XRADIO_DBG_ERROR;
u8 dbg_iface = XRADIO_DBG_ERROR;
u8 dbg_plat = XRADIO_DBG_ERROR;
u8 dbg_queue = XRADIO_DBG_ERROR;
u8 dbg_io = XRADIO_DBG_ERROR;
u8 dbg_txrx = XRADIO_DBG_ERROR;
u8 dbg_cmd = XRADIO_DBG_ERROR;

static int xradio_version_show(struct seq_file *seq, void *unused)
{
	seq_printf(seq, "%s\n", XRADIO_VERSION);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(xradio_version);

int xradio_debug_init_common(struct xradio_hw *xradio_hw)
{
	struct xradio_debug_common *debug;
	struct dentry *root;

	debug = kzalloc_obj(*debug);
	if (!debug)
		return -ENOMEM;

	root = debugfs_create_dir("xradio", NULL);
	if (IS_ERR(root)) {
		kfree(debug);
		return PTR_ERR(root);
	}

	debugfs_create_file("version", 0400, root, NULL,
			    &xradio_version_fops);

	debug->debugfs_phy = root;
	xradio_hw->debug = debug;

	return 0;
}

void xradio_debug_deinit_common(struct xradio_hw *xradio_hw)
{
	struct xradio_debug_common *debug = xradio_hw->debug;

	if (!debug)
		return;

	debugfs_remove_recursive(debug->debugfs_phy);
	kfree(debug);
	xradio_hw->debug = NULL;
}
