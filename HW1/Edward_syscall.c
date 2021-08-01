#include <linux/kernal.h>
#include <linux/syscalls.h>
#include <linux/printk.h>

SYSCALL_DEFINE1(Edward_syscall, char *, msg) {
	char buf[256];
	long len = strncpy_from_user(buf, msg, sizeof(buf));
	
	if (len > 128) {
		return -1;
	}

	int counter = 0;
	int i;
	for (i = 0; i < len; i++) {
		if (buf[i] == 'x') {
			buf[i] = 'y';
			counter++;
		}
	}
	buf[i] = '\0';

	printk(KERN_INFO "New string is \"%s\"\n", buf);
	return counter;
}