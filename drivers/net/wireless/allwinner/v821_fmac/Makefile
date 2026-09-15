# SPDX-License-Identifier: GPL-2.0

v821_fmac-y := low_cmd.o xradio_main.o xradio_platform.o queue.o \
	       xradio_txrx.o xradio_firmware.o debug.o up_cmd.o \
	       os_dep/os_intf.o platform/xradio_rpbuf.o \
	       platform/xradio_rpmsg.o

ccflags-y += -I$(srctree)/$(src) \
	     -I$(srctree)/$(src)/include \
	     -I$(srctree)/$(src)/os_dep \
	     -I$(srctree)/$(src)/platform \
	     -DXRADIO_RPBUF_SUPPORT -DSTA_AP_COEXIST

obj-$(CONFIG_V821_FMAC_WLAN) += v821_fmac.o
