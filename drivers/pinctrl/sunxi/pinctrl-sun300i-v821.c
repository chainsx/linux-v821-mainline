// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 Allwinner Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/platform_device.h>
#include <linux/property.h>

#include "pinctrl-sunxi.h"

static const struct sunxi_desc_pin sun300i_v821_pins[] = {
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 0),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "mmc0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 0)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 1),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "mmc0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 1)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 2),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "mmc0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 2)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 3),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "mmc0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 3)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 4),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "mmc0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 4)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 5),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "mmc0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 5)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 6),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "spif"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 6)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 7),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "spif"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 7)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 8),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "spif"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 8)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 9),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "spif"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 9)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 10),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "spif"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 10)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 11),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x2, "spif"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 11)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 12),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 12)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 13),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 13)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 14),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 14)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 15),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 15)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 16),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 16)),
};

static const unsigned int sun300i_v821_irq_bank_map[] = { 2 };

static const struct sunxi_pinctrl_desc sun300i_v821_pinctrl_data = {
	.pins		= sun300i_v821_pins,
	.npins		= ARRAY_SIZE(sun300i_v821_pins),
	.gpio_ngpio	= ARRAY_SIZE(sun300i_v821_pins),
	.no_gpio_direction = true,
	.pin_base	= PC_BASE,
	.first_bank	= PC_BASE / PINS_PER_BANK,
	.irq_bank_map	= sun300i_v821_irq_bank_map,
	.irq_banks	= ARRAY_SIZE(sun300i_v821_irq_bank_map),
};

static const struct sunxi_desc_pin sun300i_v821_r_pins[] = {
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 4),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x3, "uart0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 4)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 5),
		  SUNXI_FUNCTION(0x0, "gpio_in"),
		  SUNXI_FUNCTION(0x1, "gpio_out"),
		  SUNXI_FUNCTION(0x3, "uart0"),
		  SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 5)),
};

static const unsigned int sun300i_v821_r_irq_bank_map[] = { 11 };

static const struct sunxi_pinctrl_desc sun300i_v821_r_pinctrl_data = {
	.pins		= sun300i_v821_r_pins,
	.npins		= ARRAY_SIZE(sun300i_v821_r_pins),
	.gpio_ngpio	= ARRAY_SIZE(sun300i_v821_r_pins),
	.no_gpio_direction = true,
	.irq_bank_map	= sun300i_v821_r_irq_bank_map,
	.irq_banks	= ARRAY_SIZE(sun300i_v821_r_irq_bank_map),
	.pin_base	= PL_BASE,
};

static int sun300i_v821_pinctrl_probe(struct platform_device *pdev)
{
	return sunxi_pinctrl_init_with_flags(pdev, &sun300i_v821_pinctrl_data,
					     SUNXI_PINCTRL_NEW_REG_LAYOUT);
}

static int sun300i_v821_r_pinctrl_probe(struct platform_device *pdev)
{
	return sunxi_pinctrl_init_with_flags(pdev, &sun300i_v821_r_pinctrl_data, 0);
}

static const struct of_device_id sun300i_v821_pinctrl_of_match[] = {
	{
		.compatible = "allwinner,sun300i-v821-pinctrl",
		.data = sun300i_v821_pinctrl_probe,
	}, {
		.compatible = "allwinner,sun300i-v821-r-pinctrl",
		.data = sun300i_v821_r_pinctrl_probe,
	}, {
		/* Sentinel */
	}
};
MODULE_DEVICE_TABLE(of, sun300i_v821_pinctrl_of_match);

static int sun300i_v821_pinctrl_platform_probe(struct platform_device *pdev)
{
	int (*probe)(struct platform_device *pdev);

	probe = device_get_match_data(&pdev->dev);

	return probe(pdev);
}

static struct platform_driver sun300i_v821_pinctrl_driver = {
	.probe	= sun300i_v821_pinctrl_platform_probe,
	.driver	= {
		.name		= "sun300i-v821-pinctrl",
		.of_match_table	= sun300i_v821_pinctrl_of_match,
	},
};
builtin_platform_driver(sun300i_v821_pinctrl_driver);
