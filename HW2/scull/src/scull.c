#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "scull.h"

#define CDEV_NAME "/dev/scull"

/* Quantum command line option */
static int g_quantum;

static void usage(const char *cmd)
{
	printf("Usage: %s <command>\n"
	       "Commands:\n"
	       "  R          Reset quantum\n"
	       "  S <int>    Set quantum\n"
	       "  T <int>    Tell quantum\n"
	       "  G          Get quantum\n"
	       "  Q          Qeuery quantum\n"
	       "  X <int>    Exchange quantum\n"
	       "  H <int>    Shift quantum\n"
	       "  h          Print this message\n"
	       "  K 		 Return task_info struct\n"
	       "  p <int>	 Create n processes\n"
	       "  t <int>	 Create n threads\n",
	       cmd);
}

typedef int cmd_t;

static cmd_t parse_arguments(int argc, const char **argv)
{
	cmd_t cmd;

	if (argc < 2) {
		fprintf(stderr, "%s: Invalid number of arguments\n", argv[0]);
		cmd = -1;
		goto ret;
	}

	/* Parse command and optional int argument */
	cmd = argv[1][0];
	switch (cmd) {
	case 'S':
	case 'T':
	case 'H':
	case 'X':
		if (argc < 3) {
			fprintf(stderr, "%s: Missing quantum\n", argv[0]);
			cmd = -1;
			break;
		}
		g_quantum = atoi(argv[2]);
		break;
	case 'p': //Validating args for forking
		if (argc < 3) {
			fprintf(stderr, "%s: Missing <n> processes\n", argv[0]);
			cmd = -1;
			break;
		}

		int n = atoi(argv[2]);
		if (n < 1 || n > 10) {
			fprintf(stderr, "%s: <n> processes are not within valid parameters\n", argv[0]);
			cmd = -1;
			break;
		}

		g_quantum = n;
		break;
	case 't': //Validating args for threading
		if (argc < 3) {
			fprintf(stderr, "%s: Missing <n> threads\n", argv[0]);
			cmd = -1;
			break;
		}

		int threadCount = atoi(argv[2]);
		if (threadCount < 1 || threadCount > 10) {
			fprintf(stderr, "%s: <n> threads are not within valid parameters\n", argv[0]);
			cmd = -1;
			break;
		}

		g_quantum = threadCount;
		break;
	case 'K':
	case 'R':
	case 'G':
	case 'Q':
	case 'h':
		break;
	default:
		fprintf(stderr, "%s: Invalid command\n", argv[0]);
		cmd = -1;
	}

ret:
	if (cmd < 0 || cmd == 'h') {
		usage(argv[0]);
		exit((cmd == 'h')? EXIT_SUCCESS : EXIT_FAILURE);
	}
	return cmd;
}

//Helper to pass into pthread_create()
//Creates new task_info and prints output
void *thread_helper(void *fd) {
	;
	struct task_info *tiThread;
	tiThread = malloc(sizeof(struct task_info));

	int ret = ioctl((int)fd, SCULL_IOCKQUANTUM, tiThread);
	if (ret == 0) {
		printf("state %i, stack %lx, cpu %i, prio %i, sprio %i, nprio %i, rtprio %i, " 
			"pid %i, tgid %i, nv %i, niv %i\n", tiThread->state, tiThread->stack, tiThread->cpu, tiThread->prio,
			tiThread->static_prio, tiThread->normal_prio, tiThread->rt_priority, tiThread->pid, tiThread->tgid,
			tiThread->nvcsw, tiThread->nivcsw);
	}

	free(tiThread);
} 

static int do_op(int fd, cmd_t cmd)
{
	int ret, q;

	switch (cmd) {
	case 'R':
		ret = ioctl(fd, SCULL_IOCRESET);
		if (ret == 0)
			printf("Quantum reset\n");
		break;
	case 'Q':
		q = ioctl(fd, SCULL_IOCQQUANTUM);
		printf("Quantum: %d\n", q);
		ret = 0;
		break;
	case 'G':
		ret = ioctl(fd, SCULL_IOCGQUANTUM, &q);
		if (ret == 0)
			printf("Quantum: %d\n", q);
		break;
	case 'T':
		ret = ioctl(fd, SCULL_IOCTQUANTUM, g_quantum);
		if (ret == 0)
			printf("Quantum set\n");
		break;
	case 'S':
		q = g_quantum;
		ret = ioctl(fd, SCULL_IOCSQUANTUM, &q);
		if (ret == 0)
			printf("Quantum set\n");
		break;
	case 'X':
		q = g_quantum;
		ret = ioctl(fd, SCULL_IOCXQUANTUM, &q);
		if (ret == 0)
			printf("Quantum exchanged, old quantum: %d\n", q);
		break;
	case 'H':
		q = ioctl(fd, SCULL_IOCHQUANTUM, g_quantum);
		printf("Quantum shifted, old quantum: %d\n", q);
		ret = 0;
		break;
	case 'K': //Creates new task_info 
		;
		struct task_info *ti;
		ti = malloc(sizeof(struct task_info));
		ret = ioctl(fd, SCULL_IOCKQUANTUM, ti);
		if (ret == 0) {
			printf("Quantum returned task_info: %d\n", ret);
		}
		else {
			printf("task_info Error\n");
		}

		free(ti);
		break;
	case 'p': //Performs forking
		;
		int i;
		i = g_quantum;
		while (i > 0) {	
			int forkRet = fork();

			//New task_info structs after forking
			struct task_info *tiNew;
			tiNew = malloc(sizeof(struct task_info));	
			ret = ioctl(fd, SCULL_IOCKQUANTUM, tiNew);

			
			if (forkRet < 0) { //Error
				printf("Fork Error\n");
				break;
			}
			else if (forkRet > 0) { //Parent			
				wait(NULL);
			}
			else { //Child
				if (ret == 0) {
					printf("state %i, stack %lx, cpu %i, prio %i, sprio %i, nprio %i, rtprio %i, " 
						"pid %i, tgid %i, nv %i, niv %i\n", tiNew->state, tiNew->stack, tiNew->cpu, tiNew->prio,
						tiNew->static_prio, tiNew->normal_prio, tiNew->rt_priority, tiNew->pid, tiNew->tgid,
						tiNew->nvcsw, tiNew->nivcsw);
					exit(0);
				}
				else {
					printf("task_info not returned\n");
				}
			}

			i--;
			free(tiNew);
		}

		break;
	case 't': //Performs threading
		;
		int index, status;
		index = g_quantum;

		pthread_t threads[11];

		//Creating new threads of task_info structs
		while (index > 0) {
			status = pthread_create(&threads[index-1], NULL, thread_helper, (void *)fd);
			if (status != 0) {
				printf("Thread Error\n");
				exit(-1);
			}

			index--;
		}

		//Joining threads
		index = g_quantum;
		while (index > 0) {
			pthread_join(threads[index-1], NULL);
			index--;
		}

		break;
	default:
		/* Should never occur */
		abort();
		ret = -1; /* Keep the compiler happy */
	}

	if (ret != 0)
		perror("ioctl");
	return ret;
}

int main(int argc, const char **argv)
{
	int fd, ret;
	cmd_t cmd;

	cmd = parse_arguments(argc, argv);

	fd = open(CDEV_NAME, O_RDONLY);
	if (fd < 0) {
		perror("cdev open");
		return EXIT_FAILURE;
	}

	printf("Device (%s) opened\n", CDEV_NAME);

	ret = do_op(fd, cmd);

	if (close(fd) != 0) {
		perror("cdev close");
		return EXIT_FAILURE;
	}

	printf("Device (%s) closed\n", CDEV_NAME);

	return (ret != 0)? EXIT_FAILURE : EXIT_SUCCESS;
}
