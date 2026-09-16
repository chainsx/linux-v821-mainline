// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 Allwinner Technology Co., Ltd.
 */

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mailbox_controller.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/reset.h>

#define V821_MSGBOX_CHANNELS		4
#define V821_MSGBOX_FIFO_DEPTH		8

#define MSGBOX_IRQ_ENABLE(n)		(0x20 + 0x100 * (n))
#define MSGBOX_IRQ_STATUS(n)		(0x24 + 0x100 * (n))
#define MSGBOX_MSG_STATUS(n, p)		(0x60 + 0x100 * (n) + 4 * (p))
#define MSGBOX_MSG_FIFO(n, p)		(0x70 + 0x100 * (n) + 4 * (p))

#define IRQ_ENABLE(p)			BIT(2 * (p))
#define IRQ_STATUS(p)			BIT(2 * (p))
#define MSG_COUNT_MASK			GENMASK(3, 0)

struct v821_msgbox {
	struct device *dev;
	void __iomem *local;
	void __iomem *remote;
	struct clk *clk;
	struct reset_control *reset;
	struct mbox_controller controller;
	struct mbox_chan chans[V821_MSGBOX_CHANNELS];
};

static struct v821_msgbox *to_v821_msgbox(struct mbox_chan *chan)
{
	return chan->con_priv;
}

static unsigned int v821_msgbox_channel_index(struct mbox_chan *chan)
{
	struct v821_msgbox *msgbox = to_v821_msgbox(chan);

	return chan - msgbox->chans;
}

static int v821_msgbox_startup(struct mbox_chan *chan)
{
	struct v821_msgbox *msgbox = to_v821_msgbox(chan);
	unsigned int index = v821_msgbox_channel_index(chan);
	unsigned int i;
	u32 val;

	/* Drain stale messages before enabling the receive interrupt. */
	for (i = 0; i < V821_MSGBOX_FIFO_DEPTH; i++) {
		if (!FIELD_GET(MSG_COUNT_MASK,
			       readl(msgbox->local +
				     MSGBOX_MSG_STATUS(0, index))))
			break;

		readl(msgbox->local + MSGBOX_MSG_FIFO(0, index));
	}

	val = readl(msgbox->local + MSGBOX_IRQ_STATUS(0));
	writel(val | IRQ_STATUS(index), msgbox->local + MSGBOX_IRQ_STATUS(0));

	val = readl(msgbox->local + MSGBOX_IRQ_ENABLE(0));
	writel(val | IRQ_ENABLE(index), msgbox->local + MSGBOX_IRQ_ENABLE(0));

	return 0;
}

static void v821_msgbox_shutdown(struct mbox_chan *chan)
{
	struct v821_msgbox *msgbox = to_v821_msgbox(chan);
	unsigned int index = v821_msgbox_channel_index(chan);
	u32 val;

	val = readl(msgbox->local + MSGBOX_IRQ_ENABLE(0));
	writel(val & ~IRQ_ENABLE(index), msgbox->local + MSGBOX_IRQ_ENABLE(0));
}

static int v821_msgbox_send_data(struct mbox_chan *chan, void *data)
{
	struct v821_msgbox *msgbox = to_v821_msgbox(chan);
	unsigned int index = v821_msgbox_channel_index(chan);
	u32 count;

	count = readl(msgbox->remote + MSGBOX_MSG_STATUS(0, index));
	if (FIELD_GET(MSG_COUNT_MASK, count) >= V821_MSGBOX_FIFO_DEPTH)
		return -EBUSY;

	writel(*(u32 *)data, msgbox->remote + MSGBOX_MSG_FIFO(0, index));

	return 0;
}

static bool v821_msgbox_last_tx_done(struct mbox_chan *chan)
{
	struct v821_msgbox *msgbox = to_v821_msgbox(chan);
	unsigned int index = v821_msgbox_channel_index(chan);
	u32 count;

	count = readl(msgbox->remote + MSGBOX_MSG_STATUS(0, index));

	return FIELD_GET(MSG_COUNT_MASK, count) < V821_MSGBOX_FIFO_DEPTH;
}

static bool v821_msgbox_peek_data(struct mbox_chan *chan)
{
	struct v821_msgbox *msgbox = to_v821_msgbox(chan);
	unsigned int index = v821_msgbox_channel_index(chan);
	u32 count;

	count = readl(msgbox->local + MSGBOX_MSG_STATUS(0, index));

	return FIELD_GET(MSG_COUNT_MASK, count) != 0;
}

static const struct mbox_chan_ops v821_msgbox_chan_ops = {
	.startup		= v821_msgbox_startup,
	.send_data		= v821_msgbox_send_data,
	.shutdown		= v821_msgbox_shutdown,
	.last_tx_done		= v821_msgbox_last_tx_done,
	.peek_data		= v821_msgbox_peek_data,
};

static struct mbox_chan *v821_msgbox_xlate(struct mbox_controller *controller,
					   const struct of_phandle_args *args)
{
	struct v821_msgbox *msgbox = container_of(controller, typeof(*msgbox),
						  controller);

	if (args->args_count != 1 || args->args[0] >= V821_MSGBOX_CHANNELS)
		return ERR_PTR(-EINVAL);

	return &msgbox->chans[args->args[0]];
}

static irqreturn_t v821_msgbox_irq(int irq, void *dev_id)
{
	struct v821_msgbox *msgbox = dev_id;
	irqreturn_t ret = IRQ_NONE;
	u32 enabled, pending;
	unsigned int i;

	enabled = readl(msgbox->local + MSGBOX_IRQ_ENABLE(0));
	pending = readl(msgbox->local + MSGBOX_IRQ_STATUS(0));

	for (i = 0; i < V821_MSGBOX_CHANNELS; i++) {
		u32 status;
		u32 msg;

		if (!(enabled & IRQ_ENABLE(i)) || !(pending & IRQ_STATUS(i)))
			continue;

		do {
			status = readl(msgbox->local + MSGBOX_MSG_STATUS(0, i));
			if (!FIELD_GET(MSG_COUNT_MASK, status))
				break;

			msg = readl(msgbox->local + MSGBOX_MSG_FIFO(0, i));
			mbox_chan_received_data(&msgbox->chans[i], &msg);
		} while (true);

		writel(IRQ_STATUS(i), msgbox->local + MSGBOX_IRQ_STATUS(0));
		ret = IRQ_HANDLED;
	}

	return ret;
}

static int v821_msgbox_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct v821_msgbox *msgbox;
	int irq;
	int ret;
	int i;

	msgbox = devm_kzalloc(dev, sizeof(*msgbox), GFP_KERNEL);
	if (!msgbox)
		return -ENOMEM;

	msgbox->dev = dev;
	msgbox->local = devm_platform_ioremap_resource_byname(pdev, "local");
	msgbox->remote = devm_platform_ioremap_resource_byname(pdev, "remote");
	if (IS_ERR(msgbox->local))
		return PTR_ERR(msgbox->local);
	if (IS_ERR(msgbox->remote))
		return PTR_ERR(msgbox->remote);

	msgbox->clk = devm_clk_get_enabled(dev, NULL);
	if (IS_ERR(msgbox->clk))
		return PTR_ERR(msgbox->clk);

	msgbox->reset = devm_reset_control_get_optional_exclusive(dev, NULL);
	if (IS_ERR(msgbox->reset))
		return PTR_ERR(msgbox->reset);

	ret = reset_control_deassert(msgbox->reset);
	if (ret)
		return ret;

	irq = platform_get_irq_byname(pdev, "local");
	if (irq < 0)
		return irq;

	ret = devm_request_irq(dev, irq, v821_msgbox_irq, 0,
			       dev_name(dev), msgbox);
	if (ret)
		return ret;

	for (i = 0; i < V821_MSGBOX_CHANNELS; i++)
		msgbox->chans[i].con_priv = msgbox;

	msgbox->controller.dev = dev;
	msgbox->controller.ops = &v821_msgbox_chan_ops;
	msgbox->controller.chans = msgbox->chans;
	msgbox->controller.num_chans = V821_MSGBOX_CHANNELS;
	msgbox->controller.txdone_poll = true;
	msgbox->controller.txpoll_period = 1;
	msgbox->controller.of_xlate = v821_msgbox_xlate;

	return devm_mbox_controller_register(dev, &msgbox->controller);
}

static const struct of_device_id v821_msgbox_of_match[] = {
	{ .compatible = "allwinner,sun300i-v821-msgbox" },
	{ }
};
MODULE_DEVICE_TABLE(of, v821_msgbox_of_match);

static struct platform_driver v821_msgbox_driver = {
	.probe	= v821_msgbox_probe,
	.driver	= {
		.name		= "sun300i-v821-msgbox",
		.of_match_table	= v821_msgbox_of_match,
	},
};
module_platform_driver(v821_msgbox_driver);

MODULE_DESCRIPTION("Allwinner V821 msgbox driver");
MODULE_LICENSE("GPL");
