#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/msdos_fs.h>
#include <linux/fs.h>

int main(int argc, char **argv)
{
	int ret, fd;
	struct fat_boot_sector fbs;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <path to disk>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	ret = read(fd, &fbs, sizeof(fbs));
	if (ret != sizeof(fbs)) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	printf("system_id: %s\nSectors per Cluster: %d\n"
			, fbs.system_id
			, fbs.sec_per_clus);

	/* 8 == FAT32, 4 == FAT16 */
	if (fbs.sec_per_clus == 8) {
		printf("Label %.11s\nFS type: %s\n"
				, fbs.fat32.vol_label
				, fbs.fat32.fs_type);
	} else {
		printf("Label %.11s\nFS type: %s\n"
				, fbs.fat16.vol_label
				, fbs.fat16.fs_type);
	}

	if (close(fd)) {
		perror("close");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
