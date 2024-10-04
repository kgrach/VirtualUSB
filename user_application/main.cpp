#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 256

/* Example inspired from open(2) */
int main(int argc, const char **argv)
{
	char buf[BUF_SIZE];
	int fd;

	fd = open("/dev/vusb1", O_RDWR);

	//ssize_t len = write(fd, "Hello,", 6);

	//read(fd, buf, BUF_SIZE);
	//printf("Read buffer: %s\n", buf);
	close(fd);
}