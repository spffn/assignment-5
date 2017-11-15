--- README ---

Taylor Clark
CS 4760
Assignment #5
Resource Management

---

Design and implement a resource management module for our Operating System Simulator oss. Use deadlock detection and recovery strategy to manage resources.

I understand that I am submitting this project while it is not completely working. At the moment, the project will compile and run, spawning processes that will attempt to request resources. Master does properly grant these requests if possible, and will notify in the log why not if it can't. There is not proper deadlock detection, but there is a description of how I would implement it.

This project catches Ctrl-C and cleans up resources as needed.

--- TO COMPILE ---
Run the makefile to compile both oss and user.


--- TO RUN ---
./(program name) -(any desired command line arguments) (their values, if needed)
EX: ./oss -t 20

--- GITHUB ---

	https://github.com/spffn/assignment-5

	
--- DESCRIPTIONS ---

-- oss
	
	This will be your main program and serve as the master process. You will start the operating system simulator (call the executable oss) as one main process who will fork multiple children at random times. The randomness will be simulated by a logical clock that will be updated by oss as well as user processes. The logical clock resides in shared memory and is accessed as a critical resource using a semaphore. You should have two unsigned integers for the clock; one will show the time in seconds and the other will show the time in nanoseconds, offset from the beginning of a second.
	
	In the beginning, oss will allocate shared memory for system data structures, including resource descriptors for each resource. All the resources are static but some of them may be shared. The resource descriptor is a fixed size structure and contains information on managing the resources within oss. Make sure that you allocate space to keep track of activities that affect the resources, such as request, allocation, and release. The resource descriptors will reside in shared memory and will be accessible to the child. Create descriptors for 20 resources, out of which about 20% should be sharable resources. 
	
	After creating the descriptors, make sure that they are populated with an initial number of resources; assign a number between 1 and 10 (inclusive) for the initial instances in each resource class. You may have to initialize another structure in the descriptor to indicate the allocation of specific instances of a resource to a process.
	
	After the resources have been set up, fork a user process at random times (between 1 and 500 milliseconds of your logical clock). Make sure that you never have more than 18 user processes in the system. If you already have 18 processes, do not create any more until some process terminates. Your user processes execute concurrently and there is no scheduling performed.
	
	They run in a loop constantly till they have to terminate. oss should grant resources when asked as long as it can find sufficient resources to allocate. Periodically oss should run a deadlock detection algorithm. If there are some deadlocked processes, it will kill them one by one till the deadlock is resolved. The policy to kill processes is determined by how long the processes request has been in the "queue". Processes which have been in it for over 1 ms without completing are killed.
	
	Make sure to release any resources claimed by a process when it is terminated. Your system should run for a maximum of 2 real life seconds, but the amount of simulated seconds maximum is up to you as long as processes are deadlocking and your system is recovering from that deadlock and continuing on at least 5 times total.
	
	I want you to keep track of statistics during your runs. Keep track of how many requests have been granted, as well as number of processes were terminated by the deadlock detection algorithm vs processes that eventually terminated successfully. How many times the deadlock detection was run, how many processes it had to terminate and percentage of total processes in a deadlock it had to kill on average. Make sure that you have signal handling to terminate all processes, if needed. In case of abnormal termination, make sure to remove shared memory and semaphores.
	
	When writing to the log file, you should have two ways of doing this. One setting (verbose on) should indicate in the log file every time master gives someone a requested resource or when master sees that a user has finished with a resource. It should also log the time when a deadlock is detected and how it removed the deadlock. That is, which processes were terminated. In addition, every 20 granted requests, output a table showing the current resources allocated to each process.
	
	When verbose is off, it should only log when a deadlock is detected and how it was resolved
	
	COMMAND LINE ARGUMENTS:
	
		-h: Show help. This shows all the accepted command lines arguments for this
			version of the program.
		-l: Will allow you to specify the filename to use as log file.
			By default, it is log.out.
			Do not include the extension (which will always be .out).
			Ex. -l newlog
		-o: Will allow you to specify the simulated time to end.
			By default, it is 2 simulated seconds.
			Ex. -o 10
		-v: Will enable verbose logging style on output.
			In verbose mode, in the log file, there will be records of:
				- Every time Master gives someone a requested resource
				- When master sees that a user has finished with a resource
				- Time when a deadlock is detected and how it removed the deadlock. That is, which processes were terminated.
				- Every 20 granted requests, current resource allocation table will be printed.

-- child
	
	While the user processes are not actually doing anything, they will be asking for resources at random times.
	
	You should have a parameter giving a bound for when a process should request/let go of a resource. Each process when it starts should roll a random number from 0 to that bound and when it occurs it should try and either claim a new resource or release a resource that it already has. It should make this request by putting a request in shared memory. It will continue looping and checking to see if it is granted that resource.
	
	At random times (between 0 and 250ms), the process checks if it should terminate successfully. If so, it should deallocate all the resources allocated to it by communicating to master that it is releasing all those resources. Make sure that this probability to terminate is low enough that churn happens in your system.
	
-- info.h
	
	This header file holds the definitions for semaphores, shared memory keys and structs used in the project.