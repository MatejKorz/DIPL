# xorcipher begin
obj-$(CONFIG_CRYPTO_LIB_XORCIPHER)		+= xorcipher.o
obj-$(CONFIG_CRYPTO_LIB_XORCIPHER_KUNIT)	+= xorcipher_test.o
