/* Taylor Clark
CS 4760
Assignment #5
Resource Management

*/


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "info.h"

struct owned_resources {
	int num;
	int amo;
};

int main(int argc, char *argv[]){
	
	pid_t pid = (long)getpid();		// process id
	printf("(!!) Process %ld starting. \n", pid);
	
	/* SEMAPHORE INFO */
	sem_t *semaphore = sem_open(SEM_NAME, O_RDWR);
	if (semaphore == SEM_FAILED) {
		printf("%ld: ", pid);
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
	/* SEMAPHORE INFO END */
	
	/* VARIOUS VARIABLES */
	srand(time(NULL));
	int *reqNum;
	int bound = 300;					
	int letGo = 0;						// # of milliseconds to release a resource after
	int cStop = 0;						// # of milliseconds to check stop after
	int which;							// which resources to let go / request
	int doWhat;							// 0 = release; 1 = request
	int now;							// save time for use on now to release or grab 
										// a new resource (milliseconds)	
	struct owned_resources own[10];
	int p_own = 0;						// points to last filled spot in own
										
	
	/* SHARED MEMORY */
	int shmidA, shmidB, shmidC;
	
	// locate the segments
	if ((shmidA = shmget(KEYA, 75, 0666)) < 0) {
        perror("child shmget A failed");
        exit(1);
    }
	else if ((shmidB = shmget(KEYB, (sizeof(struct Resources)*5), 0666)) < 0) {
        perror("child shmget B failed");
        exit(1);
    }
	else if ((shmidC = shmget(KEYC, (sizeof(struct Request)*10), 0666)) < 0) {
        perror("child shmget C failed");
        exit(1);
    }
	
	// attach to our data space
	int *clock;
	struct Resources *r;
	struct Request *req;
	
	if ((clock = shmat(shmidA, NULL, 0)) == (char *) -1) {
        perror("child shmat A failed");
        exit(1);
    }
	else if ((r = shmat(shmidB, NULL, 0)) == (char *) -1) {
        perror("child shmat B failed.");
        exit(1);
    }
	else if ((req = shmat(shmidC, NULL, 0)) == (char *) -1) {
        perror("child shmat C failed.");
        exit(1);
    }
	/* SHARED MEMORY END*/	
	
	int stop = 0;
	int i;
	now = (clock[0] * 1,000) + (clock[1] * 1,000,000);
	reqNum = clock[2];
	
	// enter while loop
	while(stop == 0) { 
		// see if its time to stop
		cStop = rand() % 251;
		if(now <= now + cStop){
			// 1 in 10 chance to end program
			if(rand() % 10 < 1){ 
				printf("%ld: Ending process @ %i.%i, releasing resources.\n", pid, clock[0], clock[1]);
				stop = 1; 
				// release all own resources
				for(i = 0; i < 10; i++){
					r[own[i].num].amoUsed -= own[i].amo;
					own[i].num = 99;
					own[i].amo = 0;
					if(p_own > 0){
						p_own--;
					}
					else { p_own = 0; }
				continue; 
				}
			}
		}
		
		// 1,000 ms = 1 second
		// 1,000,000,000 ns = 1 seconds
		// 1 ms = 1,000,000 ns
		/* letGo = rand() % bound;
		if(now <= now + letGo) { */
			doWhat = rand() % 2;
			switch(doWhat){
				// release
				case 0: {
					// choose a random resource to let go of
					which = rand() % 11;
					if(own[which].amo == 0){
						// skip because its not in use
					}
					else {
						r[own[which].num].amoUsed -= own[which].amo;
						printf("%ld: Releasing %i of R%i @ %i.%i.\n", pid, own[which].amo, which, clock[0], clock[1]);
						own[which].num = 99;
						own[which].amo = 0;
						if(p_own > 0){
							p_own--;
						}
						else { p_own = 0; }
					}
					break;
				}
				// request
				case 1: {
					which = rand() % 5;
					int n;
					
					while(sem_trywait(semaphore) != 0){ /* wait for sem */ }
					n = reqNum;
					printf("%ld: Submitting req #%i.\n", pid, n);
					
					req[n].pid = pid;
					req[n].which = which;
					// MAX request is limited to how many the resource has
					int x = (rand() % r[which].amount + 1);
					req[n].amo = x;
					req[n].times = clock[0];
					req[n].timens = clock[1];
					req[n].granted = 0;
					printf("%ld: Requesting %i of R%i @ %i.%i...\n\n", req[n].pid, req[n].amo, req[n].which, clock[0], clock[1]);
					
					if(reqNum > 10) { reqNum = 0; }
					else { reqNum += 1; }
					sem_post(semaphore);
					
					// now wait till request is granted
					while(req[n].granted == 0) { /* wait */ }
					
					printf("(!!) %ld: Request granted for %i of R%i @ %i.%i.\n", pid, x, which, clock[0], clock[1]);
					own[p_own].num = req[n].which;
					own[p_own].amo = req[n].amo;
					p_own++;
					break;
				}
				default: {
					printf("%ld: This isn't right.\n", pid);
					stop = 1;
					// release all own resources
					for(i = 0; i < 10; i++){
						r[own[i].num].amoUsed -= own[i].amo;
						own[i].num = 99;
						own[i].amo = 0;
					}
					continue;
				}
			}
		/* } */
		
		now = (clock[0] * 1,000) + (clock[1] * 1,000,000);
	}
	
	return 0;
}