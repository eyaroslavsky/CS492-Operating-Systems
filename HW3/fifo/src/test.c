#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <scull.h>

#define CDEV_NAME "/dev/scull"

int main(int argc, char **argv) {
	int fd1 = open(CDEV_NAME, O_WRONLY);
	int fd2 = open(CDEV_NAME, O_RDONLY);

	char *testArr[3] = {"hello", "world", "string"};

	char *buf1 = malloc(11 * sizeof(char));
	char *buf2 = malloc(11 * sizeof(char));
	char *buf3 = malloc(11 * sizeof(char));

	char **testBuf = malloc(3 * sizeof(char *));
	testBuf[0] = buf1;
	testBuf[1] = buf2;
	testBuf[2] = buf3;

	int i;
	for (i = 0; i < 3; i++) {
		write(fd1, testArr[i], i+1);
		read(fd2, testBuf[i], i+1);
		printf("buf: %s\n", testBuf[i]);
	}

	free(buf1);
	free(buf2);
	free(buf3);
	free(testBuf);

}