obj-m += hello.o
obj-m += hello_param.o
obj-m += scull.o

SRC=/lib/modules/$(shell uname -r)/build

all:
	make -C $(SRC) M=$(PWD) modules

clean:
	make -C $(SRC) M=$(PWD) clean

re: clean all
