KDIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
TARGET = alloc_continuous_memory_dma_pl

CROSS_COMPILE = arm-linux-gnueabihf-
KDIR_ARM = /home/naydan/work/repo_xilinx/linux-xlnx
ARCH = arm
DEFCONFIG = xilinx_zynq_defconfig

obj-m := $(TARGET).o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

arm_clean_build: distclean arm_distclean
	# Выполнение конфигурации ядра
	$(MAKE) -C $(KDIR_ARM) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $(DEFCONFIG) -j$(shell nproc)

	$(MAKE) -C $(KDIR_ARM) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -j$(shell nproc)
	
	# Сборка модуля
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR_ARM) M=$(PWD) modules -j$(shell nproc)

arm_build: distclean
	# Выполнение конфигурации ядра
	$(MAKE) -C $(KDIR_ARM) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $(DEFCONFIG) -j$(shell nproc)

	$(MAKE) -C $(KDIR_ARM) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -j$(shell nproc)
	
	# Сборка модуля
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR_ARM) M=$(PWD) modules -j$(shell nproc)


distclean: clean
	rm -f *.ko *.symvers *.mod

clean:
	rm -f *.o .*.cmd .*.flags *.mod.c *.order .*.o.d
	rm -f .*.*.cmd *~ *.*~ TODO.*
	rm -rf .tmp* .tmp_versions


arm_distclean:
	# Очистка ядра
	$(MAKE) -C $(KDIR_ARM) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	$(MAKE) -C $(KDIR_ARM) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) distclean
