obj-m := ring_buffer.o 
KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
ccflags-y := -Wall

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
