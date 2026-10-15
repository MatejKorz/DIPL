obj-m += xorcipher.o
obj-m += xorcipher_test.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod xorcipher.ko
	sudo insmod xorcipher_test.ko
	sudo dmesg | tail -n 10

unload:
	-sudo rmmod xorcipher_test
	-sudo rmmod xorcipher

.PHONY: all clean load unload
