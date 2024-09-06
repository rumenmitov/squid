#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int, char **)
{
	static char buf[] = "hello world\n";
	int fd = open("/squid-cache/test", O_RDWR| O_CREAT);
	if (fd < 0) {
		printf("Error: could not open file\n");
		return -1;
	}
	int count = read(fd, buf, sizeof(buf) - 1);
	printf("Read %d bytes: %s\n", count, buf);

	int err = write(fd, buf, sizeof(buf) - 1);
	printf("Write returned %d\n", err);
	return 0;
}
