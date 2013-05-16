# cross-compile module makefile

DRIVERNAME=hrt

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
	scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $(DRIVERNAME).ko root@192.168.10.112:/home/root

clean:
	rm -rf *~ *.ko *.o *.mod.c modules.order Module.symvers .hrt* .tmp_versions

endif

