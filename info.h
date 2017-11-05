/* Taylor Clark
CS 4760
Assignment #5
Resource Management

*/

extern int reqNum;

/* SEMAPHORE */
#define SEM_NAME "/semaName"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define MUTEX 1

/* SHARED ME KEYS */
#define KEYA 1001
#define KEYB 888
#define KEYC 777

/* RESOURCE DESCRIPTS */
struct Resources {
	int amount;						// amount available, between 0 and 10
	int amoUsed;					// amount currently used
	int shared;						// 0 = no; 1 = yes
	// resouce ownership array
	// struct alloc allocated[];
};

/* RESOURCE REQUEST STRUCT */
struct Request {
	pid_t pid;
	int which;
	int amo;
	int granted;
	int times;
	int timens;
}

/* USE DATA */
// A simple struct to track certain ways to use the resources.
struct alloc{
	pid_t pid;
	int amo;
};	