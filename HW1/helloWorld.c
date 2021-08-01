#include <linux/module.h>
#include <linux/init.h>
#include <asm/current.h>
#include <linux/sched.h>

static int hello_init(void) {
	printk(KERN_ALERT "Hello World from Edward Yaroslavsky (10439131)\n");
	return 0;
}

static void hello_exit(void) {
	printk(KERN_ALERT "Task \"%s\" (pid %i)\n", current->comm, current->pid);
}

module_init(hello_init);
module_exit(hello_exit);