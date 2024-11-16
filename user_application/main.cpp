#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <vector>

#include <sys/ioctl.h>

#include "UsbConfig.hpp"

using namespace std;

/* Example inspired from open(2) */
int main(int argc, const char **argv)
{

	int fd;
	
	vector<unsigned char> request(8), response;

	fd = open("/dev/vusb1", O_RDWR);

	int res = ioctl(fd, 0, 0);

	while(1) {
		
		ssize_t size;
	
		size = read(fd, request.data(), 8);

		if(!size) {
			this_thread::sleep_for(2s);
			continue;
		}

		response = GetResponse(request);
	    
		size = write(fd, response.data(), response.size());
	}

	//
	//printf("Read buffer: %s\n", buf);
	close(fd);
}