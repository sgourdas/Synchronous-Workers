#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFF_SIZE 256
#define SEGMENT_SIZE sizeof(sem_t)
#define SEGMENT_PERM 0666

#define CHEF_SEMAPHORE "/chef"
#define SALAD_MAKER1_SEMAPHORE "/saladmaker1"
#define SALAD_MAKER2_SEMAPHORE "/saladmaker2"
#define SALAD_MAKER3_SEMAPHORE "/saladmaker3"
#define MUTEX_SEMAPHORE "/mutex"
#define UNIFIED_SEMAPHORE "/unified"

enum ingredients {TOMATO, PEPPER, ONION};
enum chefLogs {SELECT, NOTIFY, REST};
enum makerLogs {WAITING, GET, START, END};
/* CHEF FUNCTIONS */
void reset(void);
void identifyChefArgs(int, char **, int *, int *);
void setup(int *, int **, sem_t **, sem_t **, sem_t **, sem_t **, sem_t **);
int identifyMaker(int, int);
void chefLogger(int, FILE *, sem_t *, int, int);
void makerSalads(int);
void sharedTimeperiods(void);
void cleanup(sem_t *, sem_t *, sem_t *, sem_t *, sem_t *, sem_t *, int *, int);
/* SALAD MAKER FUNCTIONS */
void identifyMakerArgs(int, char **, int *, int *, int *, int *);
void identifySem(sem_t **, int);
void makerLogger(int, FILE *, FILE *, sem_t *, int);