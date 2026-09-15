// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 Allwinner Technology Co., Ltd.
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include "ccu_common.h"
#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mp.h"
#include "ccu_reset.h"

#include <dt-bindings/clock/sun300i-v821-ccu.h>
#include <dt-bindings/reset/sun300i-v821-ccu.h>

static const struct clk_parent_data mmc_parents[] = {
	{ .fw_name = "hosc" },
	{ .fw_name = "pll-peri-192m" },
};

static const struct clk_parent_data spif_parents[] = {
	{ .fw_name = "hosc" },
	{ .fw_name = "pll-peri-384m" },
};

static const struct clk_parent_data bus_parents[] = {
	{ .fw_name = "pll-peri-192m" },
};

static SUNXI_CCU_DUALDIV_MUX_GATE(mmc0_clk, "mmc0", mmc_parents, 0x014,
				  0, 5, 16, 5, 24, 3,
				  BIT(31), 0);
static SUNXI_CCU_DUALDIV_MUX_GATE(spif_clk, "spif", spif_parents, 0x020,
				  0, 4, 16, 2, 24, 2,
				  BIT(31), 0);

static SUNXI_CCU_GATE_DATA(bus_mmc0_clk, "bus-mmc0", bus_parents,
			   0x084, BIT(20), 0);
static SUNXI_CCU_GATE_DATA(bus_uart0_clk, "bus-uart0", bus_parents,
			   0x080, BIT(15), 0);
static SUNXI_CCU_GATE_DATA(bus_spif_clk, "bus-spif", bus_parents,
			   0x084, BIT(5), 0);
static SUNXI_CCU_GATE_DATA(msgbox_clk, "msgbox", bus_parents,
			   0x07c, BIT(6), 0);
static SUNXI_CCU_GATE_DATA(e907_ts_clk, "e907-ts", bus_parents,
			   0x00c, BIT(31), 0);
static SUNXI_CCU_GATE_DATA(riscv_clk, "riscv", bus_parents,
			   0x080, BIT(0), 0);

static struct ccu_common *sun300i_v821_ccu_clks[] = {
	&mmc0_clk.common,
	&spif_clk.common,
	&bus_mmc0_clk.common,
	&bus_uart0_clk.common,
	&bus_spif_clk.common,
	&msgbox_clk.common,
	&e907_ts_clk.common,
	&riscv_clk.common,
};

static struct clk_hw_onecell_data sun300i_v821_hw_clks = {
	.hws	= {
		[CLK_MMC0]		= &mmc0_clk.common.hw,
		[CLK_SPIF]		= &spif_clk.common.hw,
		[CLK_BUS_MMC0]		= &bus_mmc0_clk.common.hw,
		[CLK_BUS_UART0]	= &bus_uart0_clk.common.hw,
		[CLK_BUS_SPIF]		= &bus_spif_clk.common.hw,
		[CLK_MSGBOX]		= &msgbox_clk.common.hw,
		[CLK_E907_TS]		= &e907_ts_clk.common.hw,
		[CLK_RISCV]		= &riscv_clk.common.hw,
	},
	.num	= CLK_RISCV + 1,
};

static const struct ccu_reset_map sun300i_v821_resets[] = {
	[RST_BUS_MMC0]		= { 0x094, BIT(20) },
	[RST_BUS_UART0]		= { 0x090, BIT(15) },
	[RST_BUS_SPIF]		= { 0x094, BIT(5) },
	[RST_BUS_RV_MSGBOX]	= { 0x090, BIT(2) },
	[RST_BUS_RV_SYS_APB]	= { 0x090, BIT(1) },
	[RST_BUS_RV]		= { 0x090, BIT(0) },
};

static const struct sunxi_ccu_desc sun300i_v821_ccu_desc = {
	.ccu_clks	= sun300i_v821_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun300i_v821_ccu_clks),
	.hw_clks	= &sun300i_v821_hw_clks,
	.resets		= sun300i_v821_resets,
	.num_resets	= ARRAY_SIZE(sun300i_v821_resets),
};

static int sun300i_v821_ccu_probe(struct platform_device *pdev)
{
	void __iomem *regs = devm_platform_ioremap_resource(pdev, 0);

	if (IS_ERR(regs))
		return PTR_ERR(regs);

	return devm_sunxi_ccu_probe(&pdev->dev, regs, &sun300i_v821_ccu_desc);
}

static const struct of_device_id sun300i_v821_ccu_ids[] = {
	{ .compatible = "allwinner,sun300i-v821-app-ccu" },
	{ }
};
MODULE_DEVICE_TABLE(of, sun300i_v821_ccu_ids);

static struct platform_driver sun300i_v821_ccu_driver = {
	.probe	= sun300i_v821_ccu_probe,
	.driver	= {
		.name			= "sun300i-v821-ccu",
		.suppress_bind_attrs	= true,
		.of_match_table		= sun300i_v821_ccu_ids,
	},
};
module_platform_driver(sun300i_v821_ccu_driver);

MODULE_IMPORT_NS("SUNXI_CCU");
MODULE_DESCRIPTION("Support for the Allwinner V821 application CCU");
MODULE_LICENSE("GPL");
