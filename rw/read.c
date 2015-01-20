#include <stdio.h>
#include <fcntl.h>

static const char *path_name_of_module = "/dev/ring_buffer0";

int main(int argc, char *argv[])
{
	int fd;
	long long int count;
	int i;
	char outbuf;
	if (argc < 2) {
		printf("Enter how much u wanna read as argument\n");
		return -1;
	}
	fd = open(path_name_of_module, O_RDWR);
	if (fd < 0) {
		printf("Error opening file.\n");
		return -1;
	}
	sscanf(argv[1], "%lld", &count);
	for (i = 0; i < count; i++) {
		read(fd, &outbuf, 1);
		printf("%c", outbuf);
	}
	close(fd);
	return 0;
}
