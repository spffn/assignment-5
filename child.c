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
#include "conb.h"

struct owned_resources {
	int num;
	int amo;
}

int main(int argc, char *argv[]){
	
	pid_t pid = (long)getpid();		// process id
	
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
	int bound = 300;					
	int letGo = 0;						// # of milliseconds to release a resource after
	int cStop = 0;						// # of milliseconds to check stop after
	int which;							// which resources to let go / request
	int doWhat;							// 0 = release; 1 = request
	int when;							// save time for use on when to release or grab 
										// a new resource (milliseconds)
	when = (clock[0] * 1,000) + (clock[1] * 1,000,000);		
	struct owned_resources own[10];
	int p_own = 0;					// points to last filled spot in owned
										
	
	/* SHARED MEMORY */
	int shmidA, shmidB, shmidC;
	
	// locate the segments
	if ((shmidA = shmget(KEYA, 50, 0666)) < 0) {
        perror("child shmget A failed");
        exit(1);
    }
	else if ((shmidB = shmget(KEYB, m * sizeof(struct Control_Block), 0666)) < 0) {
        perror("child shmget B failed");
        exit(1);
    }
	else if ((shmidC = shmget(KEYC, 50, 0666)) < 0) {
        perror("child shmget C failed");
        exit(1);
    }
	
	// attach to our data space
	int *clock;
	struct resource *r;
	struct Request *req;
	
	if ((clock = shmat(shmidA, NULL, 0)) == (char *) -1) {
        perror("child shmat A failed");
        exit(1);
    }
	else if ((r = shmat(shmidB, NULL, 0)) == (char *) -1) {
        perror("child shmat B failed.");
        exit(1);
    }
	else if ((sched = shmat(shmidC, NULL, 0)) == (char *) -1) {
        perror("child shmat C failed.");
        exit(1);
    }
	/* SHARED MEMORY END*/
	
	int stop = 0;
	// enter while loop
	while(stop == 0){
		// see if its time to stop
		cStop = rand() % 251;
		if(when <= when + cStop){
			// 1 in 5 chance to end program
			if(rand() % 5 < 1){ 
				stop = 1; 
				// release all owned resources
				for(int i = 0; i < 10; i++){
					r[owned[i].num].amoUsed -= owned[i].amo;
					owned[i].num = 99;
					owned[i].amo = 0;
					if(p_own > 0){
						p_own--;
					}
					else { p_own = 0; }
				continue; 
			}
		}
		
		// 1,000 ms = 1 second
		// 1,000,000,000 ns = 1 seconds
		// 1 ms = 1,000,000 ns
		letGo = rand() % bound;
		if(when <= when + letGo){
			doWhat = rand() % 2;
			switch(doWhat){
				// release
				case 0: {
					// choose a random resource to let go of
					which = rand() % 11;
					r[owned[which].num].amoUsed -= owned[which].amo;
					owned[which].num = 99;
					owned[which].amo = 0;
					if(p_own > 0){
						p_own--;
					}
					else { p_own = 0; }
					break;
				}
				// request
				case 1: {
					which = rand() % 21;
					int n;
					while(sem_trywait(semaphore) != 0){ /* wait for sem */ }
						n = reqNum;
					sem_post(semaphore);
					
					req[n].pid = pid;
					req[n].which = which;
					// MAX request is limited to how many the resource has
					req[n].amo = (rand() % r[which].amount + 1);
					req[n].times = clock[0];
					req[n].timens = clock[1];
					req[n].granted = 0;
					// now wait till request is granted
					while(req[n].granted == 0) { /* wait */ }
					
					owned[p_own].num = req[n].which;
					owned[p_own].amo = req[n].amo;
					p_own++;
					break;
				}
				default: {
					printf("%ld: This isn't right.\n", pid);
					stop = 1;
					// release all owned resources
					for(int i = 0; i < 10; i++){
						r[owned[i].num].amoUsed -= owned[i].amo;
						owned[i].num = 99;
						owned[i].amo = 0;
					}
					continue;
				}
			}
		}
		
	}
	
	return 0;
}