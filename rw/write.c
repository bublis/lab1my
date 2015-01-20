#include <stdio.h>
#include <fcntl.h>

static const char *path_name_of_module = "/dev/ring_buffer0";

int main(int argc, char *argv[])
{
	int fd;
	int fd2;
	int rbytes;
	char buf;
	if (argc < 2) {
		printf("Enter path to file as argument\n");
		return -1;
	}
	fd2 = open(argv[1], O_RDWR);
	fd = open(path_name_of_module, O_CREAT|O_WRONLY|O_TRUNC);
	if (fd < 0 || fd2 < 0) {
		printf("Error opening file.\n");
		return -1;
	}
	rbytes = read(fd2, &buf, 1);
	do {
		write(fd, &buf, 1);
		rbytes = read(fd2, &buf, 1);
	} while (rbytes != 0);
	close(fd);
	close(fd2);
	return 0;
}
