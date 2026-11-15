# SPDX-License-Identifier: GPL-2.0
#
#   make, make load, make unload     external xorcipher.ko (no test module)
#   make kunit                       KUnit tests in a UML kernel via kunit.py

obj-m += xorcipher.o

KDIR ?= /lib/modules/$(shell uname -r)/build
KSRC ?= $(HOME)/kernel/linux-source-7.2.12
PWD  := $(shell pwd)

IN_TREE := lib/crypto/xorcipher.c lib/crypto/xorcipher.h lib/crypto/xorcipher_test.c

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load: all
	sudo insmod xorcipher.ko
	lsmod | grep xorcipher

unload:
	-sudo rmmod xorcipher

modinfo:
	modinfo xorcipher.ko

# kunit.py builds its own kernel into $(KSRC)/.kunit and runs the suites
# as a userspace process; the running kernel and the tree's root .config
# are untouched.  Recommended way to run the tests.

kunit-deps:
	sudo apt install -y build-essential bc bison flex libelf-dev libssl-dev \
		dwarves python3 dpkg-dev

KVER ?= 7.2.12-1~bpo13+1
kunit-source:
	@test ! -d $(KSRC) || { echo "$(KSRC) already exists"; exit 1; }
	sudo apt install -y -t trixie-backports linux-source-7.2
	mkdir -p $(dir $(KSRC))
	cd $(dir $(KSRC)) && tar xf /usr/src/linux-source-7.2.tar.xz && \
		mv linux-source-7.2 $(notdir $(KSRC))

kunit-tree-check:
	@test ! -e $(KSRC)/.config -a ! -d $(KSRC)/include/config \
		-a ! -d $(KSRC)/arch/x86/include/generated \
		-a ! -d $(KSRC)/arch/um/include/generated || { \
		echo "$(KSRC) is not clean (leftover .config, include/config/"; \
		echo "or arch/*/include/generated/ from an in-tree build)."; \
		echo "Run: make kunit-tree-clean"; \
		exit 1; }

kunit-tree-clean:
	$(MAKE) -C $(KSRC) mrproper

kunit-install:
	cp xorcipher.c xorcipher.h xorcipher_test.c $(KSRC)/lib/crypto/
	cp .kunitconfig $(KSRC)/
	cd $(KSRC) && \
	if ! grep -q 'xorcipher begin' lib/crypto/Kconfig; then \
		echo >> lib/crypto/Kconfig && \
		cat $(PWD)/xorcipher.Kconfig >> lib/crypto/Kconfig; \
	fi && \
	if ! grep -q 'xorcipher begin' lib/crypto/Makefile; then \
		echo >> lib/crypto/Makefile && \
		cat $(PWD)/xorcipher.mk >> lib/crypto/Makefile; \
	fi

kunit: kunit-tree-check kunit-install
	cd $(KSRC) && ./tools/testing/kunit/kunit.py run \
		--kunitconfig=.kunitconfig 'xorcipher*'

kunit-clean:
	cd $(KSRC) && sed -i '/xorcipher begin/,$$d' lib/crypto/Kconfig lib/crypto/Makefile
	rm -f $(addprefix $(KSRC)/,$(IN_TREE)) $(KSRC)/.kunitconfig

kunit-distclean: kunit-clean
	rm -rf $(KSRC)/.kunit

.PHONY: all clean load unload modinfo kunit-deps kunit-source \
	kunit-tree-check kunit-tree-clean kunit-install kunit kunit-clean \
	kunit-distclean
