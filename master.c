/* Taylor Clark
CS 4760
Assignment #5
Resource Management

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <signal.h>
#include <semaphore.h>
#include "conb.h"

char errstr[50];
FILE *f;
int shmidA, shmidB, shmidC;
sem_t *semaphore;

void sig_handler(int);
void clean_up(void);

int main(int argc, char *argv[]){
	
	srand(time(NULL));
	
	/* SEMAPHORE INFO */
	// init semaphore
	semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_PERMS, MUTEX);
	if (semaphore == SEM_FAILED) {
		printf("Master: ");
        perror("sem_open(3) error");
		clean_up();
        exit(EXIT_FAILURE);
    }
	
	// close it because we dont use the semaphore in the parent and we want it to
	// autodie once all processes using it have ended
	if (sem_close(semaphore) < 0) {
        perror("sem_close(3) failed");
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }
	/* SEMAPHORE INFO END */
	/* SETTING UP SIGNAL HANDLING */
	signal(SIGINT, sig_handler);
	
	/* VARIOUS VARIABLES */
	// 1,000 ms = 1 second
	// 1,000,000,000 ns = 1 seconds
	// 1 ms = 1,000,000 ns	
	int kidLim = 5;						// limit on processes to have at once
	int active = 0;						// # of currently active processes
	int simTimeEnd = 10;				// when to end the simulation
	int numShare;						// how many resources to share this time
	int verFlag = 0;
	int reqNum = 0;						// which # request a child is
	int when;							// when to fork a new child
	
	pid_t pid, cpid;
	char fname[] = "log.out";		// file name
	
    time_t start;					// the timer information
	/* VARIOUS VARIABLES END */
	
	
	// for printing errors
	snprintf(errstr, sizeof errstr, "%s: Error: ", argv[0]);
	int c;
	opterr = 0;	

	/* COMMAND LINE PARSING */
	while ((c = getopt(argc, argv, ":ho:l:v")) != -1) {
		switch (c) {
			// setting name of file of palindromes to read from	
			// by default it is palindromes.txt
			case 'l': {
				int result;
				char exten[] = ".out";
				char newname[200];
				strcpy(newname, optarg);
				strcat(newname, exten);
				result = rename(fname, newname);
				if(result == 0) {
					printf ("File renamed from %s to %s\n", fname, newname);
					strcpy(fname, newname);
				}
				else {
					perror(errstr);
					printf("Error renaming file.\n");
				}
				break;
			}
			// set the amount of simulated time to run for
			case 'o': {
				simTimeEnd = atoi(optarg);
				printf ("Setting simulated clock end time to: %i nanoseconds\n", simTimeEnd);
				break;
			}
			// show help
			case 'h': {
				printf("\n----------\n");
				printf("HELP LIST: \n");
				printf("-h: \n");
				printf("\tShow help, valid options and required arguments. \n");
				
				printf("-l: \n");
				printf("\t Sets the name of the log file to output to.\n");
				printf("\t Default is log.out.\n");
				printf("\tex: -l filename \n");
				
				printf("-o: \n");
				printf("\t Sets the simulated time to end at.\n");
				printf("\t Default is 2 simulated seconds.\n");
				printf("\tex: -o 10 \n");
				
				printf("-v: \n");
				printf("\t Turns logging style on to verbose.\n");
				printf("\t Default is non-verbose.\n");
				printf("\tex: -v \n");
				
				printf("----------\n\n");
				break;
			}
			case 'v':{
				printf("Setting log style to verbose.\n");
				verFlag = 1;
				break;
			}
			// if no argument is given, print an error and end.
			case ':': {
				perror(errstr);
				fprintf(stderr, "-%s requires an argument. \n", optopt);
				return 1;
			}
			// if an invalid option is caught, print that it is invalid and end
			case '?': {
				perror(errstr);
				fprintf(stderr, "Invalid option(s): %s \n", optopt);
				return 1;
			}
		}
	}
	/* COMMAND LINE PARSING END */
	
	
	/* SHARED MEMORY */
	// shared memory clock
	// [0] is seconds, [1] is nanoseconds
	int *clock;
	struct Resources *r;
	struct Request *requests;
	
	// create segments to hold all the info from file
	if ((shmidA = shmget(KEYA, 50, IPC_CREAT | 0666)) < 0) {
        perror("Master shmgetA failed.");
		clean_up();
        exit(1);
    }
	else if ((shmidB = shmget(KEYB, (sizeof(struct Resources)*kidLim), IPC_CREAT | 0666)) < 0) {
        perror("Master shmgetB failed.");
		clean_up();
        exit(1);
    }
	else if ((shmidC = shmget(KEYC, (sizeof(struct Resources)*20), IPC_CREAT | 0666)) < 0) {
        perror("Master shmgetC failed.");
		clean_up();
        exit(1);
    }
	
	// attach segments to data space
	if ((clock = shmat(shmidA, NULL, 0)) == (char *) -1) {
        perror("Master shmat failed.");
		clean_up();
        exit(1);
    }
	else if ((r = shmat(shmidB, NULL, 0)) == (char *) -1) {
        perror("Master shmat failed.");
		clean_up();
        exit(1);
    }
	else if ((requests = shmat(shmidC, NULL, 0)) == (char *) -1) {
        perror("Master shmat failed.");
		clean_up();
        exit(1);
    }
		
	// write to shared memory the intial clock settings
	// clear out shmMsg
	clock[0] = 0;			// seconds
	clock[1] = 0;			// nanoseconds
	/* SHARED MEMORY END*/

	
	// open file for writing to
	// will rewrite the file everytime
	f = fopen(fname, "w");
	if(f == NULL){
		perror(errstr);
		printf("Error opening file.\n");
		clean_up();
		exit(1);
	}

	/* POPULATE RESOURCES */
	// set how many to share this time around (between 15% (3) and 25% (5))
	numShare = (3 + (int)(rand() / (double)((RAND_MAX + 1) * (5 - 3 + 1))));
	int nsn = 0;
	for(i = 0; i < 20; i++) {
		r[i].amount = (rand() % 10) + 1;
		r[i].amoUsed = 0;
		// if we havent marked enough resources as shared yet, mark one
		if(nsn < numShare) {
			r[i].shared = 1;
			nsn++;
		}	
	}
		
	// calculate end time
	start = time(NULL);
	printf("Starting program...\n");
	fprintf(f, "Master: Starting clock loop at %s", ctime(&start));
	fprintf(f, "\n-------------------------\n\n");
	
	/* WHILE LOOP */
	when = 
    while (clock[0] < simTimeEnd || start <= start + 2) {  
		
		// check to see if its time to fork off a new child
		int now = (clock[0] * 1,000) + (clock[1] * 1,000,000);	
		if(now <= now + when){
			// check to see if the amount of kids currently active
			// if we are at the limit, do not spawn another
			if(activeKids < kidLim){
				pid = fork();
				if (pid < 0) {
					perror(errstr); 
					printf("Fork failed!\n");
					clean_up();
					exit(1);
				}
				if (pid == 0){
					execlp("user", "user", NULL);
					perror(errstr); 
					printf("execl() failed!\n");
					clean_up();
					exit(1);
				}
			}
		}
		
		
	}
	
	fprintf(f, "Master: Time's up!\n");
	wait(NULL);
	fprintf(f, "\n-------------------------\n\n");
	start = time(NULL);
	fprintf(f, "Master: Ending clock loop at %s", ctime(&start));
	fprintf(f, "Master: Simulated time ending at: %i seconds, %i nanoseconds.\n", clock[0], clock[1]);
	printf("Finished! Please see output file for details.\n");
	
	/* CLEAN UP */ 
	clean_up();

    return 0;
}

/* CLEAN UP */ 
// release all shared memory, malloc'd memory, semaphores and close files.
void clean_up(){
	printf("Master: Cleaning up now.\n");
	
	int i;
	for(i = 0; i < make; i++){
		// kill(b[i].pid, SIGTERM);
	}
	
	shmctl(shmidA, IPC_RMID, NULL);
	shmctl(shmidB, IPC_RMID, NULL);
	shmctl(shmidC, IPC_RMID, NULL);
	if (sem_unlink(SEM_NAME) < 0){
		perror("sem_unlink(3) failed");
	}
	sem_close(semaphore);
	fclose(f);
}

/* SIGNAL HANDLER */
// catches SIGINT (Ctrl+C) and notifies user that it did.
// it then cleans up and ends the program.
void sig_handler(int signo){
	if (signo == SIGINT){
		printf("Master: Caught Ctrl-C.\n");
		clean_up();
		exit(0);
	}
}

/* CHECK FOR DEADLOCK */
void check_deadlock(){
	
}