#include <iostream>
#include <cstring>
#include <sys/errno.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "shared_memory_stuff.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "times_list.h"
using namespace std;
extern int errno;

int main(int argc, char const *argv[]) {
	struct timeval tv;
	struct tm *info;
	char buffer[64];
	int numOfSalads = -1;
	float mantime = -1.0;
	for (int i = 1; i < argc-1; i=i+2) {
		if (strcmp(argv[i],"-n") == 0) {
			for (int j = 0; j < (int) strlen(argv[i+1]); ++j) {
				if (argv[i+1][j] > '9' || argv[i+1][j] < '0') {
					cout << "Number of workers is not a number. Terminating." << endl;
					return 1;
				}
			}
			numOfSalads = atoi(argv[i+1]); // CHECK IF THIS IS A NUMBER
		}
		else if (strcmp(argv[i],"-m") == 0) {
			for (int j = 0; j < (int) strlen(argv[i+1]); ++j) {
				if ((argv[i+1][j] > '9' || argv[i+1][j] < '0') && argv[i+1][j] != '.') {
					cout << "Number of workers is not a number. Terminating." << endl;
					return 1;
				}
			}
			mantime = atof(argv[i+1]); // CHECK IF THIS IS A NUMBER
		}
		else
			cout << "MISTAKE" << endl;
	}
	// Done finding arguments
	if (numOfSalads <= 0 || mantime <= 0) {
		cout << "Could not find expected arguments. Terminating." << endl;
		return 1;
	}
	cout << numOfSalads << " salads, " << mantime << " mantime" << endl;

	// Create shared memory
	int id = 0, err = 0;
	struct shared_memory_stuff * mem;
	id = shmget(IPC_PRIVATE, sizeof(struct shared_memory_stuff), 0666); // Make shared memory segment
	if (id == -1) {
		perror("Shared memory creation failed: ");
		exit(2);
	}
	printf("Shared memory address: %d\n", id);
	mem = (struct shared_memory_stuff *) shmat (id , ( void *) 0, 0) ; // Attach the segment
	if (*( int *) mem == -1) {
		perror("Shared memory attachment failed: ");
		exit(3);
	}
	sem_init(&mem->general_sem, 1, 1); // This is to protect the shared memory stuff when they're being initialized,
	// ...but it's not really useful unless you can start 2 processes at the exact same time.
	sem_wait(&mem->general_sem);
	strcpy(mem->salad1_sem_finish, "salad1_sem_finish");
	strcpy(mem->salad2_sem_finish, "salad2_sem_finish");
	strcpy(mem->salad3_sem_finish, "salad3_sem_finish");
	cout << "Chef attached shared memory" << endl;
	mem->n_tomatoes = 0;
	mem->n_peppers = 0;
	mem->n_onions = 0;
	// mem->stopp = 0;
	mem->salads_left = numOfSalads;
	mem->saladmaker1_pid = 0;
	mem->saladmaker2_pid = 0;
	mem->saladmaker3_pid = 0;
	sem_init(&mem->saladmaker1_semaphore, 1, 0);
	sem_init(&mem->saladmaker2_semaphore, 1, 0);
	sem_init(&mem->saladmaker3_semaphore, 1, 0);
	sem_t *salad1_sem_finish = sem_open(mem->salad1_sem_finish, O_CREAT, 0755, 0);
	if (salad1_sem_finish == SEM_FAILED) {
		cout << "Saladmaker 1 finish semaphore failed" << endl;
		exit(2);
	}
	sem_init(salad1_sem_finish, 1, 0);
	sem_t *salad2_sem_finish = sem_open(mem->salad2_sem_finish, O_CREAT, 0755, 0);
	if (salad2_sem_finish == SEM_FAILED) {
		cout << "Saladmaker 2 finish semaphore failed" << endl;
		exit(2);
	}
	sem_init(salad2_sem_finish, 1, 0);
	sem_t *salad3_sem_finish = sem_open(mem->salad3_sem_finish, O_CREAT, 0755, 0);
	if (salad3_sem_finish == SEM_FAILED) {
		cout << "Saladmaker 3 finish semaphore failed" << endl;
		exit(2);
	}
	sem_init(salad3_sem_finish, 1, 0);
	sem_init(&mem->tomatoes_semaphore, 1, 1);
	sem_init(&mem->peppers_semaphore, 1, 1);
	sem_init(&mem->onions_semaphore, 1, 1);
	sem_init(&mem->shared_file_sem, 1, 1);
	sem_post(&mem->general_sem);

	FILE *shared_file = fopen("shared_file", "a");
	fprintf(shared_file, " ----------------  NEW EXECUTION  ---------------- \n");
	fflush(shared_file);
	int saladmaker_n;
	int salads1 = 0, salads2 = 0, salads3 = 0;
	srand(time(NULL));
	int previous_saladmaker = -1;
	for (int i = 0; i < numOfSalads; ++i) {
		saladmaker_n = rand() % 3 + 1; // Get a random number from 1 to 3
		if (saladmaker_n == previous_saladmaker) { // Don't choose the same saladmaker twice in a row
			saladmaker_n++;
			if (saladmaker_n > 3)
				saladmaker_n = 1;
		}
		previous_saladmaker = saladmaker_n;
		cout << "Chose saladmaker " << saladmaker_n << endl;
		if (saladmaker_n == 1) { // Add a tomato and a pepper for saladmaker 1
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Selecting ingredients ntomata piperia]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sem_wait(&mem->tomatoes_semaphore);
			mem->n_tomatoes++;
			sem_post(&mem->tomatoes_semaphore);
			sem_wait(&mem->peppers_semaphore);
			mem->n_peppers++;
			sem_post(&mem->peppers_semaphore);
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Notify saladmaker 1]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sem_post(&mem->saladmaker1_semaphore); // Signal saladmaker 1
			sem_wait(salad1_sem_finish); // Chef has to wait for saladmaker to take the ingredients off the table
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Sleep mantime]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sleep(mantime);
			salads1++;
		}
		else if (saladmaker_n == 2) { // Add a tomato and an onion for saladmaker 2
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Selecting ingredients ntomata kremmydi]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sem_wait(&mem->tomatoes_semaphore);
			mem->n_tomatoes++;
			sem_post(&mem->tomatoes_semaphore);
			sem_wait(&mem->onions_semaphore);
			mem->n_peppers++;
			sem_post(&mem->onions_semaphore);
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Notify saladmaker 2]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sem_post(&mem->saladmaker2_semaphore); // Signal saladmaker 2
			sem_wait(salad2_sem_finish);
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Sleep mantime]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sleep(mantime);
			salads2++;
		}
		else if (saladmaker_n == 3) { // Add an onion and a pepper for saladmaker 3
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Selecting ingredients piperia kremmydi]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sem_wait(&mem->onions_semaphore);
			mem->n_tomatoes++;
			sem_post(&mem->onions_semaphore);
			sem_wait(&mem->peppers_semaphore);
			mem->n_peppers++;
			sem_post(&mem->peppers_semaphore);
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Notify saladmaker 3]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sem_post(&mem->saladmaker3_semaphore); // Signal saladmaker 3
			sem_wait(salad3_sem_finish);
			gettimeofday(&tv, NULL);
			info = localtime(&(tv.tv_sec));
			strftime (buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			sem_wait(&mem->shared_file_sem);
			fprintf(shared_file, "[%s.%ld][%d][Chef][Sleep mantime]\n", buffer, tv.tv_usec/10000, getpid());
			fflush(shared_file);
			sem_post(&mem->shared_file_sem);
			sleep(mantime);
			salads3++;
		}
		mem->salads_left--;
	}
	// Tell saladmakers there's a salad to make, but they see that salads_left==0 and quit.
	sem_post(&mem->saladmaker1_semaphore);
	sem_post(&mem->saladmaker2_semaphore);
	sem_post(&mem->saladmaker3_semaphore);
	sem_wait(salad1_sem_finish); // Wait for saladmakers to quit
	sem_wait(salad2_sem_finish);
	sem_wait(salad3_sem_finish);

	sem_destroy(&mem->general_sem);
	sem_destroy(&mem->saladmaker1_semaphore);
	sem_destroy(&mem->saladmaker2_semaphore);
	sem_destroy(&mem->saladmaker3_semaphore);
	sem_close(salad1_sem_finish);
	sem_unlink("salad1_sem_finish");
	sem_close(salad2_sem_finish);
	sem_unlink("salad2_sem_finish");
	sem_close(salad3_sem_finish);
	sem_unlink("salad3_sem_finish");
	sem_destroy(&mem->tomatoes_semaphore);
	sem_destroy(&mem->peppers_semaphore);
	sem_destroy(&mem->onions_semaphore);
	sem_destroy(&mem->shared_file_sem);
	err = shmctl(id, IPC_RMID, 0); // Remove segment
	if (err == -1)
		perror("Removal. ");
	fclose(shared_file);

	// Read output files from 3 saladmakers, save the time periods they were active in a list.
	TimesList timez_list;
	char line[100];
	FILE *sync_file = fopen("sync_file1", "r");
	while (fgets(line, sizeof(line), sync_file)) {
		// printf("%s", line);
		timez_list.insert(line);
	}
	fclose(sync_file);
	sync_file = fopen("sync_file2", "r");
	while (fgets(line, sizeof(line), sync_file)) {
		// printf("%s", line);
		timez_list.insert(line);
	}
	fclose(sync_file);
	sync_file = fopen("sync_file3", "r");
	while (fgets(line, sizeof(line), sync_file)) {
		// printf("%s", line);
		timez_list.insert(line);
	}
	fclose(sync_file);
	cout << "#salads of salad_maker1 [" << mem->saladmaker1_pid << "] : " << salads1 << endl;
	cout << "#salads of salad_maker2 [" << mem->saladmaker2_pid << "] : " << salads2 << endl;
	cout << "#salads of salad_maker3 [" << mem->saladmaker3_pid << "] : " << salads3 << endl;
	// timez_list.print();
	timez_list.print_matching_times();
	return 0;
}
// ./saladmaker -t1 2 -t2 5 -s 65555
// ./chef -n 5 -m 2
// P() = wait()
// V() = signal()

// Each saladmaker has a semaphore that starts at 0, not 1.
// When the chef has put on the table ingredients for a saladmaker's salad,
//   he does saladmaker.semaphore++. The saladmaker waits at that semaphore,
//   and increments it down to make the salad.
// When the chef wants no more salads, he increments all 3 semaphores by 1,
//   the saladmakers get past it and check the salads_left==0, and then they finish.