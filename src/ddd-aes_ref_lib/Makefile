# SPDX-License-Identifier: GPL-2.0
#
#   make, make load, make unload     external ddd-aes-ref.ko

obj-m += ddd_aes_ref.o

KDIR ?= /lib/modules/$(shell uname -r)/build
KSRC ?= $(HOME)/kernel/linux-source-7.2.12
PWD  := $(shell pwd)

IN_TREE := lib/crypto/ddd_aes_ref.c lib/crypto/ddd_aes_ref.h

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load: all
	sudo insmod ddd_aes_ref.ko
	lsmod | grep ddd_aes_ref

unload:
	-sudo rmmod ddd_aes_ref

modinfo:
	sudo modinfo ddd_aes_ref.ko

.PHONY: all clean load unload modinfo
