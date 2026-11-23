# SPDX-License-Identifier: GPL-2.0
#
#   make, make load, make unload     external ddd-aes-ref.ko
#   make kunit                       kunit tests in a uml kernel via kunit.py

obj-m += ddd_aes_ref.o

KDIR ?= /lib/modules/$(shell uname -r)/build
KSRC ?= $(HOME)/kernel/linux-source-7.2.12
PWD  := $(shell pwd)

IN_TREE := lib/crypto/ddd_aes_ref.c lib/crypto/ddd_aes_ref.h lib/crypto/tests/ddd_aes_ref_kunit.c

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
	@test -d $(KSRC)/lib/crypto/tests || { \
		echo "$(KSRC) has no lib/crypto/tests/ (needs >= 6.15 / 7.x)"; exit 1; }
	cp ddd_aes_ref.c ddd_aes_ref.h $(KSRC)/lib/crypto/
	cp ddd_aes_ref_kunit.c $(KSRC)/lib/crypto/tests/
	cp .kunitconfig $(KSRC)/
	cd $(KSRC) && \
	if ! grep -q 'ddd_aes_ref begin' lib/crypto/Kconfig; then \
		echo >> lib/crypto/Kconfig && cat $(PWD)/ddd_aes_ref.Kconfig >> lib/crypto/Kconfig; \
	fi && \
	if ! grep -q 'ddd_aes_ref begin' lib/crypto/Makefile; then \
		echo >> lib/crypto/Makefile && cat $(PWD)/ddd_aes_ref.mk >> lib/crypto/Makefile; \
	fi && \
	if ! grep -q 'ddd_aes_ref kunit begin' lib/crypto/tests/Kconfig; then \
		echo >> lib/crypto/tests/Kconfig && cat $(PWD)/ddd_aes_ref_kunit.Kconfig >> lib/crypto/tests/Kconfig; \
	fi && \
	if ! grep -q 'ddd_aes_ref kunit begin' lib/crypto/tests/Makefile; then \
		echo >> lib/crypto/tests/Makefile && cat $(PWD)/ddd_aes_ref_kunit.mk >> lib/crypto/tests/Makefile; \
	fi

kunit: kunit-tree-check kunit-install
	cd $(KSRC) && ./tools/testing/kunit/kunit.py run \
		--kunitconfig=.kunitconfig 'ddd_aes_ref*'

kunit-clean:
	cd $(KSRC) && sed -i '/ddd_aes_ref begin/,/ddd_aes_ref end/d; /ddd_aes_ref kunit begin/,/ddd_aes_ref kunit end/d' \
		lib/crypto/Kconfig lib/crypto/Makefile lib/crypto/tests/Kconfig lib/crypto/tests/Makefile
	rm -f $(addprefix $(KSRC)/,$(IN_TREE)) $(KSRC)/.kunitconfig

kunit-distclean: kunit-clean
	rm -rf $(KSRC)/.kunit

.PHONY: all clean load unload modinfo kunit-deps kunit-source \
	kunit-tree-check kunit-tree-clean kunit-install kunit kunit-clean \
	kunit-distclean
