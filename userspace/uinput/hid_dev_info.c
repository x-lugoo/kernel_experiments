#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/hiddev.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <linux/msdos_fs.h>
#include <sys/ioctl.h>

const char *show_bustype(unsigned int bt)
{
	if (bt & BUS_USB)
		return "USB";
	else if (bt & BUS_BLUETOOTH)
		return "Bluetooth";
	else if (bt & BUS_VIRTUAL)
		return "Virtual";
	else
		return "Unkown";
}

int main(int argc, char **argv)
{
	int fd, ver;
	char *dev_path = "/dev/hidraw0";
	char raw_name[255];
	char phys_name[255];
	struct hiddev_devinfo hdi;

	if (argc != 1)
		dev_path = argv[1];

	fd = open(dev_path, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

	if (ioctl(fd, HIDIOCGVERSION, &ver) == -1) {
		perror("ioctl ver");
		exit(1);
	}
	printf("Device %s, version %d\n", dev_path, ver);

	if (ioctl(fd, HIDIOCGRAWNAME(sizeof(raw_name)), &raw_name) == -1) {
		perror("ioctl raw name");
		exit(1);
	}
	printf("Device raw name: %s\n", raw_name);

	if (ioctl(fd, HIDIOCGRAWPHYS(sizeof(phys_name)), &phys_name) == -1) {
		perror("ioctl raw phys");
		exit(1);
	}
	printf("Device raw physical: %s\n", phys_name);

	if (ioctl(fd, HIDIOCGRAWINFO, &hdi) == -1) {
		perror("ioctl raw info");
		exit(1);
	}
	printf("Bus type: %s\n", show_bustype(hdi.bustype));
	printf("Vendor: %04x\n", hdi.vendor);
	printf("Product: %04x\n", hdi.product);

	close(fd);

	return 0;
}
