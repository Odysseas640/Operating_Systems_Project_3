#ifndef _ODYS_SHARED_MEMORY_STUFF_
#define _ODYS_SHARED_MEMORY_STUFF_

struct shared_memory_stuff {
	sem_t general_sem;
	int saladmaker1_pid;
	int saladmaker2_pid;
	int saladmaker3_pid;
	sem_t saladmaker1_semaphore;
	sem_t saladmaker2_semaphore;
	sem_t saladmaker3_semaphore;
	int n_tomatoes;
	int n_peppers;
	int n_onions;
	sem_t tomatoes_semaphore;
	sem_t peppers_semaphore;
	sem_t onions_semaphore;
	char salad1_sem_finish[20];
	char salad2_sem_finish[20];
	char salad3_sem_finish[20];
	sem_t shared_file_sem;
	// int stopp;
	int salads_left;
};
#endif