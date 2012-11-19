
LINUX ?= /lib/modules/$(shell uname -r)/build

ccflags-y += -I$M/include

obj-m = fmc.o
obj-m += fmc-trivial.o
obj-m += fmc-write-eeprom.o

fmc-y = fmc-core.o
fmc-y += fmc-match.o
fmc-y += fmc-sdb.o
fmc-y += fru-parse.o

all modules:
	$(MAKE) -C $(LINUX) M=$(shell /bin/pwd) modules

install modules_install:
	$(MAKE) -C $(LINUX) M=$(shell /bin/pwd) modules_install

# be able to run the "clean" rule even if $(LINUX) is not valid
clean:
	rm -rf *.o *~  .*.cmd *.ko *.mod.c .tmp_versions Module.symvers \
		Module.markers modules.order
