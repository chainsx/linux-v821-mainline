// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 Allwinner Technology Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/mailbox_client.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/remoteproc.h>
#include <linux/reset.h>
#include <linux/string.h>

#define RV_CFG_BOOT_ADDR		0x204
#define E907_CORE_RESET_REG		0x09c
#define E907_CORE_RESET_KEY		0xa5690000
#define E907_CORE_RESET_BIT		BIT(0)

struct v821_rproc {
	struct device *dev;
	void __iomem *cfg;
	void __iomem *mem;
	void __iomem *ccu;
	phys_addr_t mem_pa;
	resource_size_t mem_size;
	struct clk *core_clk;
	struct clk *gate_clk;
	struct clk *ts_clk;
	struct reset_control *rv_reset;
	struct reset_control *apb_reset;
	struct mbox_chan *mbox_chan;
	struct mbox_client mbox_client;
};

static void v821_rproc_core_reset(struct v821_rproc *v821, bool assert)
{
	u32 value = E907_CORE_RESET_KEY;

	if (assert)
		value |= E907_CORE_RESET_BIT;

	writel(value, v821->ccu + E907_CORE_RESET_REG);
}

static int v821_rproc_start(struct rproc *rproc)
{
	struct v821_rproc *v821 = rproc->priv;
	int ret;

	ret = reset_control_assert(v821->rv_reset);
	if (ret)
		return ret;

	ret = reset_control_assert(v821->apb_reset);
	if (ret)
		goto err_rv;

	v821_rproc_core_reset(v821, true);

	ret = clk_prepare_enable(v821->ts_clk);
	if (ret)
		goto err_apb;

	ret = clk_prepare_enable(v821->gate_clk);
	if (ret)
		goto err_ts;

	dma_wmb();
	writel(rproc->bootaddr, v821->cfg + RV_CFG_BOOT_ADDR);

	ret = reset_control_deassert(v821->apb_reset);
	if (ret)
		goto err_gate;

	ret = reset_control_deassert(v821->rv_reset);
	if (ret)
		goto err_apb_assert;

	v821_rproc_core_reset(v821, false);

	return 0;

err_apb_assert:
	reset_control_assert(v821->apb_reset);
err_gate:
	clk_disable_unprepare(v821->gate_clk);
err_ts:
	clk_disable_unprepare(v821->ts_clk);
err_apb:
	reset_control_deassert(v821->apb_reset);
err_rv:
	reset_control_deassert(v821->rv_reset);
	return ret;
}

static int v821_rproc_stop(struct rproc *rproc)
{
	struct v821_rproc *v821 = rproc->priv;

	v821_rproc_core_reset(v821, true);
	clk_disable_unprepare(v821->gate_clk);
	clk_disable_unprepare(v821->ts_clk);
	reset_control_assert(v821->apb_reset);
	reset_control_assert(v821->rv_reset);

	return 0;
}

static void v821_rproc_kick(struct rproc *rproc, int vqid)
{
	struct v821_rproc *v821 = rproc->priv;
	int ret;

	ret = mbox_send_message(v821->mbox_chan, &vqid);
	if (ret < 0)
		dev_err(v821->dev, "failed to kick virtqueue %d: %d\n",
			vqid, ret);
}

static void *v821_rproc_da_to_va(struct rproc *rproc, u64 da, size_t len,
				 bool *is_iomem)
{
	struct v821_rproc *v821 = rproc->priv;

	if (da < v821->mem_pa || len > v821->mem_size ||
	    da > v821->mem_pa + v821->mem_size - len)
		return NULL;

	if (is_iomem)
		*is_iomem = true;

	return v821->mem + da - v821->mem_pa;
}

static int v821_rproc_map_memory(struct device *dev, struct v821_rproc *v821)
{
	struct device_node *np;
	struct resource res;
	int ret;

	np = of_parse_phandle(dev->of_node, "memory-region", 0);
	if (!np)
		return dev_err_probe(dev, -EINVAL, "missing memory region\n");

	ret = of_address_to_resource(np, 0, &res);
	of_node_put(np);
	if (ret)
		return ret;

	v821->mem = devm_memremap(dev, res.start, resource_size(&res),
				  MEMREMAP_WC);
	if (IS_ERR(v821->mem))
		return PTR_ERR(v821->mem);

	v821->mem_pa = res.start;
	v821->mem_size = resource_size(&res);

	return 0;
}

static int v821_rproc_mem_alloc(struct rproc *rproc,
				struct rproc_mem_entry *mem)
{
	mem->va = ioremap_wc(mem->dma, mem->len);

	return mem->va ? 0 : -ENOMEM;
}

static int v821_rproc_mem_release(struct rproc *rproc,
				  struct rproc_mem_entry *mem)
{
	if (mem->va)
		iounmap(mem->va);

	return 0;
}

static int v821_rproc_prepare(struct rproc *rproc)
{
	struct device *dev = rproc->dev.parent;
	struct device_node *np = dev->of_node;
	struct rproc_mem_entry *mem;
	struct resource res;
	unsigned int region = 0;
	unsigned int vdev_index = 0;
	const char *name_end;
	u64 da;
	int ret;

	while (1) {
		ret = of_reserved_mem_region_to_resource(np, region++, &res);
		if (ret == -ENODEV)
			break;
		if (ret)
			return ret;

		/* The firmware region is mapped separately for ELF loading. */
		if (strstarts(res.name, "e907"))
			continue;

		da = res.start;
		name_end = strchrnul(res.name, '@');

		if (strstarts(res.name, "vdev0buffer")) {
			mem = rproc_of_resm_mem_entry_init(&rproc->dev,
							   region - 1,
							   resource_size(&res),
							   da,
							   "vdev%dbuffer",
							   vdev_index++);
		} else {
			mem = rproc_mem_entry_init(&rproc->dev, NULL,
						   res.start,
						   resource_size(&res), da,
						   v821_rproc_mem_alloc,
						   v821_rproc_mem_release,
						   "%.*s",
						   (int)(name_end -
							 res.name),
						   res.name);
		}
		if (!mem)
			return -ENOMEM;

		rproc_add_carveout(rproc, mem);
	}

	return 0;
}

static const struct rproc_ops v821_rproc_ops = {
	.prepare	= v821_rproc_prepare,
	.start		= v821_rproc_start,
	.stop		= v821_rproc_stop,
	.kick		= v821_rproc_kick,
	.da_to_va	= v821_rproc_da_to_va,
};

static int v821_rproc_map_ccu(struct device *dev, struct v821_rproc *v821)
{
	struct device_node *np;
	struct resource res;
	int ret;

	np = of_parse_phandle(dev->of_node, "allwinner,ccu", 0);
	if (!np)
		return dev_err_probe(dev, -EINVAL, "missing CCU phandle\n");

	ret = of_address_to_resource(np, 0, &res);
	of_node_put(np);
	if (ret)
		return ret;

	v821->ccu = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(v821->ccu))
		return PTR_ERR(v821->ccu);

	return 0;
}

static int v821_rproc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct v821_rproc *v821;
	struct rproc *rproc;
	const char *firmware;
	int ret;

	ret = of_property_read_string(dev->of_node, "firmware-name", &firmware);
	if (ret)
		return dev_err_probe(dev, ret, "missing firmware name\n");

	rproc = devm_rproc_alloc(dev, "e907", &v821_rproc_ops, firmware,
				 sizeof(*v821));
	if (!rproc)
		return -ENOMEM;

	v821 = rproc->priv;
	v821->dev = dev;
	rproc->auto_boot = true;
	platform_set_drvdata(pdev, rproc);

	v821->cfg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(v821->cfg))
		return PTR_ERR(v821->cfg);

	ret = v821_rproc_map_ccu(dev, v821);
	if (ret)
		return ret;

	v821->core_clk = devm_clk_get_enabled(dev, "core");
	v821->gate_clk = devm_clk_get(dev, "gate");
	v821->ts_clk = devm_clk_get(dev, "ts");
	if (IS_ERR(v821->core_clk))
		return PTR_ERR(v821->core_clk);
	if (IS_ERR(v821->gate_clk))
		return PTR_ERR(v821->gate_clk);
	if (IS_ERR(v821->ts_clk))
		return PTR_ERR(v821->ts_clk);

	v821->rv_reset = devm_reset_control_get_exclusive(dev, "rv");
	v821->apb_reset = devm_reset_control_get_exclusive(dev, "apb");
	if (IS_ERR(v821->rv_reset))
		return PTR_ERR(v821->rv_reset);
	if (IS_ERR(v821->apb_reset))
		return PTR_ERR(v821->apb_reset);

	v821->mbox_client.dev = dev;
	v821->mbox_client.tx_block = false;
	v821->mbox_chan = mbox_request_channel_byname(&v821->mbox_client,
						      "tx");
	if (IS_ERR(v821->mbox_chan))
		return PTR_ERR(v821->mbox_chan);

	ret = v821_rproc_map_memory(dev, v821);
	if (ret)
		goto err_mbox;

	ret = devm_rproc_add(dev, rproc);
	if (ret)
		goto err_mbox;

	return 0;

err_mbox:
	mbox_free_channel(v821->mbox_chan);
	return ret;
}

static void v821_rproc_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);
	struct v821_rproc *v821 = rproc->priv;

	mbox_free_channel(v821->mbox_chan);
}

static const struct of_device_id v821_rproc_of_match[] = {
	{ .compatible = "allwinner,sun300i-v821-rproc" },
	{ }
};
MODULE_DEVICE_TABLE(of, v821_rproc_of_match);

static struct platform_driver v821_rproc_driver = {
	.probe	= v821_rproc_probe,
	.remove	= v821_rproc_remove,
	.driver	= {
		.name		= "sun300i-v821-rproc",
		.of_match_table	= v821_rproc_of_match,
	},
};
module_platform_driver(v821_rproc_driver);

MODULE_DESCRIPTION("Allwinner V821 E907 remoteproc driver");
MODULE_LICENSE("GPL");
