#include <iostream>
#include <cstring>
#include <sys/errno.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "shared_memory_stuff.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
using namespace std;
extern int errno;

int main(int argc, char const *argv[]) {
	struct timeval tv;
	struct tm *info;
	char buffer[64];
	int shmid = -1;
	float lower_bound = -1.0, upper_bound = -1.0;
	for (int i = 1; i < argc-1; i=i+2) {
		if (strcmp(argv[i],"-t1") == 0) {
			for (int j = 0; j < (int) strlen(argv[i+1]); ++j) {
				if ((argv[i+1][j] > '9' || argv[i+1][j] < '0') && argv[i+1][j] != '.') {
					cout << "Lower bound is not a number. Terminating." << endl;
					return 1;
				}
			}
			lower_bound = atof(argv[i+1]); // CHECK IF THIS IS A NUMBER
		}
		else if (strcmp(argv[i],"-t2") == 0) {
			for (int j = 0; j < (int) strlen(argv[i+1]); ++j) {
				if ((argv[i+1][j] > '9' || argv[i+1][j] < '0') && argv[i+1][j] != '.') {
					cout << "Upper bound is not a number. Terminating." << endl;
					return 1;
				}
			}
			upper_bound = atof(argv[i+1]); // CHECK IF THIS IS A NUMBER
		}
		else if (strcmp(argv[i],"-s") == 0) {
			for (int j = 0; j < (int) strlen(argv[i+1]); ++j) {
				if (argv[i+1][j] > '9' || argv[i+1][j] < '0') {
					cout << "Shared memory ID is not a number. Terminating." << endl;
					return 1;
				}
			}
			shmid = atoi(argv[i+1]); // CHECK IF THIS IS A NUMBER
		}
		else
			cout << "MISTAKE" << endl;
	}
	// Done finding arguments
	if (lower_bound < 0 || upper_bound < 0 || shmid < 0) {
		cout << "Could not find expected arguments. Terminating." << endl;
		return 1;
	}
	if (lower_bound > upper_bound) {
		cout << "Lower bound is higher than the lower bound. Terminating." << endl;
		return 1;
	}
	cout << "Lower bound: " << lower_bound << ", upper_bound: " << upper_bound << ", shmid: " << shmid << endl;

	// Attach shared memory
	int err;
	struct shared_memory_stuff * mem;
	mem = (struct shared_memory_stuff*) shmat(shmid, (void *) 0 ,0); // Attach the segment
	if (mem == (void *) -1) {
		perror("Saladmaker: Error attaching shared memory");
		exit(1);
	}

	// Decide which saladmaker this is, 1, 2 or 3
	sem_wait(&mem->general_sem);
	char filename[15];
	char sync_file_name[15];
	int *ingredient1_amount, *ingredient2_amount;
	sem_t *ingredient1_sem, *ingredient2_sem, *ingredients_ready_sem, *sem_finish;
	int which_saladmaker_is_this;
	if (mem->saladmaker1_pid == 0) { // Set pointers to the correct stuff in the shared memory according to which saladmaker this is
		mem->saladmaker1_pid = getpid();
		which_saladmaker_is_this = 1;
		ingredient1_amount = &mem->n_tomatoes;
		ingredient2_amount = &mem->n_peppers;
		ingredient1_sem = &mem->tomatoes_semaphore;
		ingredient2_sem = &mem->peppers_semaphore;
		ingredients_ready_sem = &mem->saladmaker1_semaphore;
		sem_finish = sem_open(mem->salad1_sem_finish, O_EXCL);
		if (sem_finish == SEM_FAILED) {
			cout << "Saladmaker 1: finish semaphore failed" << endl;
			exit(2);
		}
		strcpy(filename, "saladmaker1");
		strcpy(sync_file_name, "sync_file1");
		srand(time(NULL));
	}
	else if (mem->saladmaker2_pid == 0) {
		mem->saladmaker2_pid = getpid();
		which_saladmaker_is_this = 2;
		ingredient1_amount = &mem->n_tomatoes;
		ingredient2_amount = &mem->n_onions;
		ingredient1_sem = &mem->tomatoes_semaphore;
		ingredient2_sem = &mem->onions_semaphore;
		ingredients_ready_sem = &mem->saladmaker2_semaphore;
		sem_finish = sem_open(mem->salad2_sem_finish, O_EXCL);
		if (sem_finish == SEM_FAILED) {
			cout << "Saladmaker 2: finish semaphore failed" << endl;
			exit(2);
		}
		strcpy(filename, "saladmaker2");
		strcpy(sync_file_name, "sync_file2");
		srand(time(NULL) + 5);
	}
	else if (mem->saladmaker3_pid == 0) {
		mem->saladmaker3_pid = getpid();
		which_saladmaker_is_this = 3;
		ingredient1_amount = &mem->n_peppers;
		ingredient2_amount = &mem->n_onions;
		ingredient1_sem = &mem->onions_semaphore;
		ingredient2_sem = &mem->peppers_semaphore;
		ingredients_ready_sem = &mem->saladmaker3_semaphore;
		sem_finish = sem_open(mem->salad3_sem_finish, O_EXCL);
		if (sem_finish == SEM_FAILED) {
			cout << "Saladmaker 3: finish semaphore failed" << endl;
			exit(2);
		}
		strcpy(filename, "saladmaker3");
		strcpy(sync_file_name, "sync_file3");
		srand(time(NULL) + 17);
	}
	else {
		cout << "Assignment says 3 saladmakers. Quitting..." << endl;
		shmdt((void *) mem); // Detach segment
		exit(3);
	}
	sem_t *shared_file_sem = &mem->shared_file_sem;
	sem_post(&mem->general_sem);

	FILE *this_saladmaker_file = fopen(filename, "w");
	FILE *sync_file = fopen(sync_file_name, "w");
	FILE *shared_file = fopen("shared_file", "a");
	int quitt = 0;
	float salad_make_time;
	do {
		gettimeofday(&tv, NULL);
		info = localtime(&(tv.tv_sec));
		strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
		fprintf(this_saladmaker_file, "[%s.%ld][%d][Saladmaker%d][Waiting for ingredients]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		sem_wait(shared_file_sem);
		fprintf(shared_file, "[%s.%ld][%d][Saladmaker%d][Waiting for ingredients]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		fflush(shared_file);
		sem_post(shared_file_sem);
		sem_wait(ingredients_ready_sem);
		if (mem->salads_left <= 0 /*&& (*ingredient1_amount == 0 || *ingredient2_amount == 0)*/)
			break;
		gettimeofday(&tv, NULL);
		fprintf(sync_file, "START,%ld,%ld\n", tv.tv_sec, tv.tv_usec/10000);
		info = localtime(&(tv.tv_sec));
		strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
		fprintf(this_saladmaker_file, "[%s.%ld][%d][Saladmaker%d][Get ingredients]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		sem_wait(shared_file_sem);
		fprintf(shared_file, "[%s.%ld][%d][Saladmaker%d][Get ingredients]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		fflush(shared_file);
		sem_post(shared_file_sem);
		sem_wait(ingredient1_sem);
		(*ingredient1_amount)--;
		sem_post(ingredient1_sem);
		sem_wait(ingredient2_sem);
		(*ingredient2_amount)--;
		sem_post(ingredient2_sem);
		sem_post(sem_finish); // Tell chef "I took the ingredients off the table"
		cout << "Started making salad" << endl;
		// Get a random amount of time to make this salad
		salad_make_time = ((float)rand()/(float)(RAND_MAX)) * (upper_bound - lower_bound) + lower_bound;
		gettimeofday(&tv, NULL);
		info = localtime(&(tv.tv_sec));
		strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
		fprintf(this_saladmaker_file, "[%s.%ld][%d][Saladmaker%d][Start making salad]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		sem_wait(shared_file_sem);
		fprintf(shared_file, "[%s.%ld][%d][Saladmaker%d][Start making salad]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		fflush(shared_file);
		sem_post(shared_file_sem);
		sleep(salad_make_time);
		cout << "Finished, took " << salad_make_time << endl;
		gettimeofday(&tv, NULL);
		fprintf(sync_file, "STOP,%ld,%ld\n", tv.tv_sec, tv.tv_usec/10000);
		info = localtime(&(tv.tv_sec));
		strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
		fprintf(this_saladmaker_file, "[%s.%ld][%d][Saladmaker%d][End making salad]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		sem_wait(shared_file_sem);
		fprintf(shared_file, "[%s.%ld][%d][Saladmaker%d][End making salad]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
		fflush(shared_file);
		sem_post(shared_file_sem);
	} while (quitt == 0); // This condition is useless, loop will stop at break;

	gettimeofday(&tv, NULL);
	info = localtime(&(tv.tv_sec));
	strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
	fprintf(this_saladmaker_file, "[%s.%ld][%d][Saladmaker%d][Quitting]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
	sem_wait(shared_file_sem);
	fprintf(shared_file, "[%s.%ld][%d][Saladmaker%d][Quitting]\n", buffer, tv.tv_usec/10000, getpid(), which_saladmaker_is_this);
	fflush(shared_file);
	sem_post(shared_file_sem);

	fclose(this_saladmaker_file);
	fclose(shared_file);
	fclose(sync_file);
	err = shmdt (( void *) mem ); // Detach segment
	if (err == -1)
		perror("Detachment.");
	sem_post(sem_finish);
	sem_close(sem_finish);
	return 0;
}