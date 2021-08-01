#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char* msg = argv[1];
	printf("Message is first \"%s\"", msg);
    long int val = syscall(548, msg);
    printf("System call returned %s\n", msg);
    return val;
}