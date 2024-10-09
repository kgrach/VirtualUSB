#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>

#define BUF_SIZE 4096

using namespace std;

/* Example inspired from open(2) */
int main(int argc, const char **argv)
{
	char buf[BUF_SIZE];
	int fd;

	fd = open("/dev/vusb1", O_RDWR);

	while(1){
		
		ssize_t size;
		size = read(fd, buf, sizeof(ssize_t));

		if(!size) {
			this_thread::sleep_for(2s);
			continue;
		}

		size = *(ssize_t*)&buf[0];

		size = read(fd, buf, size);
	    
		//ssize_t len = write(fd, "Hello,", 6);
		
		break;
	}

	//
	//printf("Read buffer: %s\n", buf);
	close(fd);
}