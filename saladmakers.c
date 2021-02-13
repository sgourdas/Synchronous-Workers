#include "Types.h"


int main(int argc, char **argv) { // Compile with -lrt or -lpthread

	int id, minTime, maxTime, ingredient, *sm = NULL;
	sem_t *maker_sem = NULL, *chef_sem = NULL, *mutex_sem = NULL, *unified_sem = NULL;
	// Set seet for srand
	srand(time(NULL));
	// Identify the arguments and assign values to the correspondign variables
	identifyMakerArgs(argc, argv, &minTime, &maxTime, &id, &ingredient);
	// Attach shared memory segment
	if(*(sm = (int *) shmat(id, NULL, 0)) == -1)
		return -1;
	// Init semaphores
	if((chef_sem = sem_open(CHEF_SEMAPHORE, O_CREAT)) == SEM_FAILED)
		return -1;
	if((mutex_sem = sem_open(MUTEX_SEMAPHORE, O_CREAT)) == SEM_FAILED)
		return -1;
	if((unified_sem = sem_open(UNIFIED_SEMAPHORE, O_CREAT)) == SEM_FAILED)
		return -1;
	// Identify and open maker semaphore
	identifySem(&maker_sem, ingredient);

	int timer;
	char temp[30];
    // Determine the corresponding log file
    sprintf(temp, "saladmaker%d.log", ingredient + 1);	// We use +1 for same output to the one shown
    // Open log files for writing
	FILE *fp = fopen(temp, "w");
	FILE *unifiedFP = fopen("unified.log", "a");
	// While we have salads left to make, work
	while(*sm > 0) {
		// Log that we are waiting for ingredients
		makerLogger(WAITING, fp, unifiedFP, unified_sem, ingredient);
		// Wait for chef to notify me
		sem_wait(maker_sem);
		// Log that we got ingredients needed
		makerLogger(GET, fp, unifiedFP, unified_sem, ingredient);
		// Notify chef I've started
		sem_post(chef_sem);
		// Log that we are starting creation of a salad
		makerLogger(START, fp, unifiedFP, unified_sem, ingredient);
		// Get a random time between minTime and maxTime to work on a salad
		timer = (rand() % (maxTime - minTime + 1)) + minTime;
		// Sleep for the randomly generated time -- indicating makign a salad
		sleep(timer);
		// Log that we finished making a salad
		makerLogger(END, fp, unifiedFP, unified_sem, ingredient);
		// Decrement salads left to make
		sem_wait(mutex_sem);
		(*sm)--;
		sem_post(mutex_sem);

	}
	// Detach segment
	if(shmdt(sm) != 0)
		return -1;
	// Close file pointers
	fclose(fp);
	fclose(unifiedFP);

	return 0;

}

void identifyMakerArgs(int argc, char **argv, int *minTime, int *maxTime, int *id, int *ingredient) {	// Identify input arguments and assign to correct variables

	if(argc == 9) {

		for (int i = 1 ; i < argc ; i++) {

			if(!strcmp("-t1", argv[i]))/* if we have -n */
				*minTime = atoi(argv[i + 1]);
			else if(!strcmp("-t2", argv[i])) /* if we have -m */
				*maxTime = atoi(argv[i + 1]);
			else if(!strcmp("-s", argv[i])) /* if we have -i */
				*id = atoi(argv[i + 1]);
			else if(!strcmp("-i", argv[i])) /* if we have -f */
				*ingredient = atoi(argv[i + 1]);

		}

	} else {

		exit(-1);

	}

}

void identifySem(sem_t **maker_sem, int ingredient) {	// Identify the semaphore name based on the ingredient this maker recieved

	switch(ingredient) {
		
		case ONION:
			
			if((*maker_sem = sem_open(SALAD_MAKER1_SEMAPHORE, O_CREAT)) == SEM_FAILED)
				exit(-1);
				
			break;

		case PEPPER:
			
			if((*maker_sem = sem_open(SALAD_MAKER2_SEMAPHORE, O_CREAT)) == SEM_FAILED)
				exit(-1);

			break;

		case TOMATO:
			
			if((*maker_sem = sem_open(SALAD_MAKER3_SEMAPHORE, O_CREAT)) == SEM_FAILED)
				exit(-1);

			break;

	}

}

void makerLogger(int logIdentifier, FILE *fp, FILE *unifiedFP, sem_t *unified_sem, int ingredient) {

	char temp[200];
	struct tm *local;
	struct timeval tv;

	gettimeofday(&tv, NULL);
   	local = localtime(&tv.tv_sec);

	switch(logIdentifier) {

		case WAITING:

			sprintf(temp, "[%.2d:%.2d:%.2d:%.2d.%.2d] [%d] [Saladmaker%d] [Waiting for ingredients]\n", local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, ((int) (tv.tv_usec / 1000)) % 100, getpid(), ingredient + 1);
		
			break;

		case GET:
			
			sprintf(temp, "[%.2d:%.2d:%.2d:%.2d.%.2d] [%d] [Saladmaker%d] [Get ingredients]\n", local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, ((int) (tv.tv_usec / 1000)) % 100, getpid(), ingredient + 1);
			
			break;

		case START:
			
			sprintf(temp, "[%.2d:%.2d:%.2d:%.2d.%.2d] [%d] [Saladmaker%d] [Start making salad]\n", local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, ((int) (tv.tv_usec / 1000)) % 100, getpid(), ingredient + 1);
		
			break;

		case END:
			
			sprintf(temp, "[%.2d:%.2d:%.2d:%.2d.%.2d] [%d] [Saladmaker%d] [End making salad]\n", local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, ((int) (tv.tv_usec / 1000)) % 100, getpid(), ingredient + 1);
	
			break;

	}

	fprintf(fp, "%s", temp);
	fflush(fp);
	sem_wait(unified_sem);
	fprintf(unifiedFP, "%s", temp);
	fflush(unifiedFP);
	sem_post(unified_sem);

}