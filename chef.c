#include "Types.h"


int main(int argc, char **argv) { // Compile with or -pthread
	// Reset previous semaphores with the same name, that may have not been cleaned up
	reset();
	// Provide srand with a seed
	srand(time(NULL));
	// Variable declaration
	int id, saladsNum, manTime, *sm;
	sem_t *chef_sem = NULL, *maker1_sem = NULL, *maker2_sem = NULL, *maker3_sem = NULL, *mutex_sem = NULL, *unified_sem = NULL;
	// Identify arguments passed to the program
	identifyChefArgs(argc, argv, &saladsNum, &manTime);
	// Create segment
	if((id = shmget(IPC_PRIVATE, SEGMENT_SIZE, SEGMENT_PERM)) == -1)
		exit(-1);
	// Attach segment
	if(*(sm = (int *) shmat(id, NULL, 0)) == -1)
		exit(-1);
	// Initialize semaphores
	if((chef_sem = sem_open(CHEF_SEMAPHORE, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR, 0)) == SEM_FAILED)
		exit(-1);
	if((maker1_sem = sem_open(SALAD_MAKER1_SEMAPHORE, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR, 0)) == SEM_FAILED)
		exit(-1);
	if((maker2_sem = sem_open(SALAD_MAKER2_SEMAPHORE, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR, 0)) == SEM_FAILED)
		exit(-1);
	if((maker3_sem = sem_open(SALAD_MAKER3_SEMAPHORE, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR, 0)) == SEM_FAILED)
		exit(-1);
	if((mutex_sem = sem_open(MUTEX_SEMAPHORE, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR, 1)) == SEM_FAILED)
		exit(-1);
	if((unified_sem = sem_open(UNIFIED_SEMAPHORE, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR, 1)) == SEM_FAILED)
		exit(-1);
	// Assign the saladsNum value equal to shared memory
	*sm = saladsNum;
	// Use this to run makers faster
	printf("./saladmakers -t1 1 -t2 3 -s %d -i 0\n", id);

	int ingredient1, ingredient2, ingredient1Prev = -1, ingredient2Prev = -1;
    // Erase file contents
    FILE *tempFP = fopen("unified.log", "w");
    fclose(tempFP);
    // Open for appending
	FILE *fp = fopen("unified.log", "a");
	// Do job untill we exceed the number of needed salads
	while(*sm > 0) {
		// Pick two random ingredients
		ingredient1 = rand() % 3;
		
		do {

			ingredient2 = rand() % 3;

		} while(ingredient1 == ingredient2);	// Make sure the ingredients are not the same
		// If the ingredients are the same as before select others
		if((ingredient1 == ingredient1Prev && ingredient2 == ingredient2Prev) || (ingredient1 == ingredient2Prev && ingredient2 == ingredient1Prev)) 
			continue;
		// Print message saying that we selected ingrediends
		chefLogger(SELECT, fp, unified_sem, ingredient1, ingredient2);
		// Print message saying that we are going to notify the saladmaker corresponding to the ingredients
		chefLogger(NOTIFY, fp, unified_sem, ingredient1, ingredient2);
		// Identify the maker corresponding to these ingredients and wait for his response
		switch(identifyMaker(ingredient1, ingredient2)) {	// Notify maker that he is good to go

			case 1:
				sem_post(maker1_sem);	
				break;

			case 2:
				sem_post(maker2_sem);
				break;

			case 3:
				sem_post(maker3_sem);
				break;

			default:
				printf("error\n");
				return -1;

		}
		// Store ingredients as the old ones -- preparing for next loop
		ingredient1Prev = ingredient1;
		ingredient2Prev = ingredient2;
		// Wait for maker to start
		sem_wait(chef_sem);
		// Print message saying that we are going to rest
		chefLogger(REST, fp, unified_sem, ingredient1, ingredient2);
		// Rest
		sleep(manTime);

	}
	// 2 makers will halt, added post here for them to end
	sem_post(maker1_sem);
	sem_post(maker2_sem);
	sem_post(maker3_sem);
	// Total salads based on *sm value
	sleep(4); // This is added for the instance that a salad maker is making a salad after the saladsNum was made -- so we wait for the correct total salads value -- change value accordingly. It is advised that you use this for perfect printing and use a value above the maximum sleep time for a maker.
	printf("Total #salads: %d\n", saladsNum - *sm);
	// Salads per salad maker based on their logs
	makerSalads(1);
	makerSalads(2);
	makerSalads(3);
	// Determine and print the time periods where 2 or more makers where working simultaneously
	sharedTimeperiods();
	// Close file pointer
	fclose(fp);
	// Cleanup semaphores and shared memory
	cleanup(chef_sem, maker1_sem, maker2_sem, maker3_sem, mutex_sem, unified_sem, sm, id);

	return 0;

}

void reset(void) {	// Unlink previously remaining semaphores that might wrongly affect our process

	sem_unlink(CHEF_SEMAPHORE);
	sem_unlink(SALAD_MAKER1_SEMAPHORE);
	sem_unlink(SALAD_MAKER2_SEMAPHORE);
	sem_unlink(SALAD_MAKER3_SEMAPHORE);
	sem_unlink(MUTEX_SEMAPHORE);
	sem_unlink(UNIFIED_SEMAPHORE);

}

void identifyChefArgs(int argc, char **argv, int *saladsNum, int *manTime) {	// Identify arguments passed to the process

	if(argc == 5) {

		for (int i = 1 ; i < argc ; i++) {

			if(!strcmp("-n", argv[i]))			/* if we have -n */
				*saladsNum = atoi(argv[i + 1]);
			else if(!strcmp("-m", argv[i])) 	/* if we have -m */
				*manTime = atoi(argv[i + 1]);

		}

	} else {

		exit(-1);

	}

}

int identifyMaker(int ingredient1, int ingredient2) {	// Identify the id of the maker corresponding to the input ingredients

	if((ingredient1 == TOMATO && ingredient2 == PEPPER) || (ingredient2 == TOMATO && ingredient1 == PEPPER))
		return 1;

	if((ingredient1 == TOMATO && ingredient2 == ONION) || (ingredient2 == TOMATO && ingredient1 == ONION))
		return 2;

	if((ingredient1 == ONION && ingredient2 == PEPPER) || (ingredient2 == ONION && ingredient1 == PEPPER))
		return 3;

	return -1;

}

void chefLogger(int logIdentifier, FILE *fp, sem_t *unified_sem, int ingredient1, int ingredient2) {

	char buffer[BUFF_SIZE];
	struct tm *local;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	local = localtime(&tv.tv_sec);

	switch(logIdentifier) {

		case SELECT:

			sprintf(buffer, "[%.2d:%.2d:%.2d:%.2d.%.2d] [%d] [Chef] [Selecting ingredients [%d][%d]]\n", local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, ((int) (tv.tv_usec / 1000)) % 100, getpid(), ingredient1, ingredient2);
			
			break;


		case NOTIFY:

			sprintf(buffer, "[%.2d:%.2d:%.2d:%.2d.%.2d] [%d] [Chef] [Notifying saladmaker %d]\n", local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, ((int) (tv.tv_usec / 1000)) % 100, getpid(), identifyMaker(ingredient1, ingredient2));
			
			break;

		case REST:

			sprintf(buffer, "[%.2d:%.2d:%.2d:%.2d.%.2d] [%d] [Chef] [Man time for resting]\n", local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, ((int) (tv.tv_usec / 1000)) % 100, getpid());

			break;


	}

	sem_wait(unified_sem);
	fprintf(fp, "%s", buffer);
	fflush(fp);
	sem_post(unified_sem);

}

void makerSalads(int makerIndex) {	// Calculate and print salads made from the maker of makerIndex index, based on the log

	int counter = 0;
	char file[16], thistime[20], pid[10], makerid[20], msg[300], trash[300];

	sprintf(file, "saladmaker%d.log", makerIndex);
	
	FILE *fp = fopen(file, "r");

	while(fscanf(fp, "%s %s %s %s", thistime, pid, makerid, msg) != EOF) {	// Read needed info from line

		fgets(trash, sizeof(trash), fp);	// Read the rest of the line
		
		if(!strcmp(msg, "[End"))
			counter++;

	}

	fclose(fp);

	printf("#salads of salad_maker%d [%d] : [%d]\n", atoi(makerid+11), atoi(pid+1), counter);

}

void sharedTimeperiods(void) {	// Determine the shared time period for making salads, between salad makers

	int counter = 0;
	char thistime[20], pid[10], makerid[20], msg[300], trash[300], startTimer[20], endTimer[20];

	FILE *newfp = fopen("unified.log", "r");

	while(fscanf(newfp, "%s %s %s %s", thistime, pid, makerid, msg) != EOF) {	// Read needed info from line

		fgets(trash, sizeof(trash), newfp);	// Read the rest of the line so we can continue to next one later
		// Keep a counter for how many salad makers are making salads based on start and end logs
		if(!strcmp(msg, "[Start")) {
			
			counter++;
			// If the counter of current processes making salads was made equal to 2, we mark this time as the start of a concurrency period
			if(counter == 2)
				strcpy(startTimer, thistime);

		} else if(!strcmp(msg, "[End")) {

			counter--;
			// If the counter of current processes making salads was made equal to 1, we mark this time as the end of a concurrency period
			if(counter == 1)
				strcpy(endTimer, thistime);
		
		}
		// Print the period that just ended -- if it did
		if((counter == 1) && !strcmp(msg, "[End"))
			printf("[%s, %s]\n", startTimer, endTimer);

	}

	fclose(newfp);

}

void cleanup(sem_t *chef_sem, sem_t *maker1_sem, sem_t *maker2_sem, sem_t *maker3_sem, sem_t *mutex_sem, sem_t *unified_sem, int *sm, int id) {	// Cleanup shared memory and semaphore space
	// Close the semaphores
	if(sem_close(chef_sem) == -1)
		exit(-1);
	if(sem_close(maker1_sem) == -1)
		exit(-1);
	if(sem_close(maker2_sem) == -1)
		exit(-1);
	if(sem_close(maker3_sem) == -1)
		exit(-1);
	if(sem_close(mutex_sem) == -1)
		exit(-1);
	if(sem_close(unified_sem) == -1)
		exit(-1);
	// Unlink the semaphores
	if(sem_unlink(CHEF_SEMAPHORE) == -1)
		exit(-1);
	if(sem_unlink(SALAD_MAKER1_SEMAPHORE) == -1)
		exit(-1);
	if(sem_unlink(SALAD_MAKER2_SEMAPHORE) == -1)
		exit(-1);
	if(sem_unlink(SALAD_MAKER3_SEMAPHORE) == -1)
		exit(-1);
	if(sem_unlink(MUTEX_SEMAPHORE) == -1)
		exit(-1);
	if(sem_unlink(UNIFIED_SEMAPHORE) == -1)
		exit(-1);
	// Detach the shared memory segment
	if(shmdt(sm) != 0)
		exit(-1);
	// Remove shared memory segment
	if(shmctl(id , IPC_RMID , NULL) == -1)
		exit(-1);

}