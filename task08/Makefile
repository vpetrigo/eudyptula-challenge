obj-m += hello_module.o
KDIR ?= /lib/modules/$(shell uname -r)/build
ccflags-y += -std=gnu99

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
