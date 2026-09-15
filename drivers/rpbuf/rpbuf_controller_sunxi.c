// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 *
 * (C) Copyright 2020-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Junyan Lin <junyanlin@allwinnertech.com>
 *
 * Allwinner RPBuf driver.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/dma-mapping.h>

#include "rpbuf_internal.h"

#define SUNXI_RPBUF_CONTROLLER_VERSION "1.1.0"

struct sunxi_rpbuf_controller_priv {
	struct device *rproc;
};

static int sunxi_rpbuf_controller_alloc_payload_memory(struct rpbuf_controller *controller,
						       struct rpbuf_buffer *buffer,
						       void *priv)
{
	struct device *dev = controller->dev;
	void *va;
	dma_addr_t pa;
	int ret;

	va = dma_alloc_coherent(dev, buffer->len, &pa, GFP_KERNEL);
	if (IS_ERR_OR_NULL(va)) {
		dev_err(dev, "dma_alloc_coherent for len %d failed\n", buffer->len);
		ret = -ENOMEM;
		goto err_out;
	}
	dev_dbg(dev, "allocate payload memory: va 0x%pK, pa %pad, len %d\n",
		va, &pa, buffer->len);

	buffer->va = va;
	buffer->pa = (phys_addr_t)pa;
	buffer->da = (u64)pa;

	return 0;

err_out:
	return ret;
}

static void sunxi_rpbuf_controller_free_payload_memory(struct rpbuf_controller *controller,
						       struct rpbuf_buffer *buffer,
						       void *priv)
{
	struct device *dev = controller->dev;
	void *va = NULL;
	dma_addr_t pa = 0;

	va = buffer->va;
	pa = buffer->pa;

	dev_dbg(dev, "free payload memory: va %pK, pa %pad, len %d\n",
		va, &pa, buffer->len);
	dma_free_coherent(dev, buffer->len, va, pa);
}

static struct rpbuf_controller_ops sunxi_rpbuf_controller_ops = {
	.alloc_payload_memory = sunxi_rpbuf_controller_alloc_payload_memory,
	.free_payload_memory = sunxi_rpbuf_controller_free_payload_memory,
};

static int sunxi_rpbuf_controller_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct sunxi_rpbuf_controller_priv *chip;
	struct rpbuf_controller *controller;
	struct device_node *rproc_np;
	struct device *rproc;
	struct device_node *tmp_np;
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	rproc_np = of_parse_phandle(np, "remoteproc", 0);
	if (!rproc_np) {
		dev_err(dev, "no \"remoteproc\" node specified\n");
		return -EINVAL;
	}

	/* We assume that the remoteproc is always a platform device */
	rproc = bus_find_device_by_of_node(&platform_bus_type, rproc_np);
	if (!rproc) {
		dev_err(dev, "failed to get remoteproc device\n");
		ret = -EINVAL;
		goto err_put_rproc_node;
	}
	of_node_put(rproc_np);
	chip->rproc = rproc;

	tmp_np = of_parse_phandle(np, "memory-region", 0);
	if (tmp_np) {
		of_node_put(tmp_np);
		ret = of_reserved_mem_device_init(dev);
		if (ret < 0) {
			dev_err(dev, "failed to get reserved memory (ret: %d)\n", ret);
			goto err_put_rproc_device;
		}
	}

	controller = rpbuf_create_controller(dev, &sunxi_rpbuf_controller_ops, chip);
	if (!controller) {
		dev_err(dev, "rpbuf_create_controller failed\n");
		ret = -ENOMEM;
		goto err_release_reserved_mem;
	}

	/*
	 * Linux can only be MASTER until we ensure the translation from
	 * buffer->pa to buffer->va is correct.
	 * We use the remoteproc device as rpbuf link token.
	 */
	ret = rpbuf_register_controller(controller, (void *)rproc, RPBUF_ROLE_MASTER);
	if (ret < 0) {
		dev_err(dev, "rpbuf_register_controller failed\n");
		goto err_destroy_controller;
	}

	/*
	 * We must set rpbuf_controller as the device drvdata, to ensure that it
	 * can be found by rpbuf_get_controller_by_of_node().
	 */
	dev_set_drvdata(dev, controller);

	return 0;
err_destroy_controller:
	rpbuf_destroy_controller(controller);
err_release_reserved_mem:
	of_reserved_mem_device_release(dev);
err_put_rproc_device:
	put_device(rproc);
err_put_rproc_node:
	of_node_put(rproc_np);
	return ret;
}

static void sunxi_rpbuf_controller_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rpbuf_controller *controller = dev_get_drvdata(dev);
	struct sunxi_rpbuf_controller_priv *chip = controller->priv;

	if (IS_ERR_OR_NULL(controller)) {
		dev_err(dev, "invalid rpbuf_controller ptr\n");
		return;
	}

	dev_set_drvdata(dev, NULL);

	rpbuf_unregister_controller(controller);
	rpbuf_destroy_controller(controller);
	put_device(chip->rproc);
}

static const struct of_device_id sunxi_rpbuf_controller_ids[] = {
	{ .compatible = "allwinner,rpbuf-controller" },
	{}
};

static struct platform_driver sunxi_rpbuf_controller_driver = {
	.probe	= sunxi_rpbuf_controller_probe,
	.remove	= sunxi_rpbuf_controller_remove,
	.driver	= {
		.owner = THIS_MODULE,
		.name = "sunxi-rpbuf-controller",
		.of_match_table = sunxi_rpbuf_controller_ids,
	},
};

module_platform_driver(sunxi_rpbuf_controller_driver);
MODULE_DESCRIPTION("Allwinner RPBuf controller driver");
MODULE_AUTHOR("Junyan Lin <junyanlin@allwinnertech.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(SUNXI_RPBUF_CONTROLLER_VERSION);
