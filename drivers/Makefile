obj-m += scull.o
obj-m += simple_module.o
obj-m += kernel_probes.o
obj-m += netfilter.o

SRC=/lib/modules/$(shell uname -r)/build

all:
	make -C $(SRC) M=$(PWD) modules

clean:
	make -C $(SRC) M=$(PWD) clean

re: clean all
