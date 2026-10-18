obj-m += test_crypto_headers.o

.PHONY: all install clean

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

install: all
	mkdir -p /lib/modules/$(shell uname -r)/extra
	cp test_crypto_headers.ko /lib/modules/$(shell uname -r)/extra/
	depmod -a
	modprobe test_crypto_headers

clean:
	modprobe -r test_crypto_headers
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
