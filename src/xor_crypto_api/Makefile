obj-m += xor_skcipher.o
obj-m += xorcipher_api_test.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
XORLIB := ../xor_crypto_lib

SYMVERS := $(dir $(PWD))xor_crypto_lib/Module.symvers

all: $(XORLIB)/Module.symvers
	$(MAKE) -C $(KDIR) M=$(PWD) KBUILD_EXTRA_SYMBOLS="$(SYMVERS)" modules

$(XORLIB)/Module.symvers:
	$(MAKE) -C $(XORLIB)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(MAKE) -C $(XORLIB) clean

load: all
	sudo insmod $(XORLIB)/xorcipher.ko
	sudo insmod xor_skcipher.ko
	sudo insmod xorcipher_api_test.ko
	sudo dmesg | tail -n 20

unload:
	-sudo rmmod xorcipher_api_test
	-sudo rmmod xor_skcipher
	-sudo rmmod xorcipher

.PHONY: all clean load unload
