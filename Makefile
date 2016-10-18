all:
	make -C drivers
	make -C userspace
clean:
	make -C drivers clean
	make -C userspace clean
