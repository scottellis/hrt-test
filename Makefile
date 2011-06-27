# cross-compile module makefile

ifneq ($(KERNELRELEASE),)
    obj-m := hrt.o
else
    PWD := $(shell pwd)

default:
ifeq ($(strip $(KERNELDIR)),)
	$(error "KERNELDIR is undefined!")
else
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 
endif

install:
	scp hrt.ko root@tide:/home/root

clean:
	rm -rf *~ *.ko *.o *.mod.c modules.order Module.symvers .hrt* .tmp_versions

endif

