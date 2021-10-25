#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>



static int dev;


int main(void) {
	char buff[40];
	int len;

	unsigned char buf;

	dev = open("/dev/ActPhoto_Module", O_RDWR);

	if(dev < 0) {
		printf( "Device Open ERROR!\n");
		exit(1);
	}

	buf = 1;

    while(1) {

		read(dev, buff, 40);
		//write(dev, &buf, 0);

		if(buff[0] != '\0')
			printf("%s .... \n", buff);
	}

	close(dev);

	return 0;

}