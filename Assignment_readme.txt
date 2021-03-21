Odysseas Stefas, 1700151

Execution example
make
./chef -n 10 -m 0.1
./saladmaker -t1 1 -t2 2 -s 32
./saladmaker -t1 3 -t2 4 -s 32
./saladmaker -t1 0.1 -t2 0.2 -s 32

The chef starts by reading its arguments. Then it creates the shared memory.

The shared memory is a struct that contains the semaphores, the saladmakers' PIDs, the number of each ingredient on the table, and the number of salads left.
There's 11 semaphores. 3 are for the chef to signal each saladmaker that his ingredients are on the table. 3 are used to protect access to each ingredient, and 1 is for the shared file. 3 are for the saladmakers to tell the chef "I took the ingredients off the table", and they're also used to make sure the saladmakers terminate before the chef does. These last 3 are named, because they are used for the last time after the saladmakers have closed the shared file.
The 11th one is only used when a process starts and reads/initializes stuff in the shared memory. It doesn't really do anything unless you can start 2 processes at the same exact time.

The chef initializes all the semaphores. Then he gets into a loop where he chooses a saladmaker, puts his ingredients on the table and gives him the signal to get them. Until the chosen saladmaker has started and taken the ingredients, the chef waits at the "chef took the ingredients" semaphore.
Then the chef sleeps(mantime), and chooses a different saladmaker for the next salad.
In each round, the chef decreases the n_salads variable in the shared memory by one. When all the salads are done, the chef gives one last signal to the saladmakers to make a salad, and waits at the "saladmaker received ingredients" semaphore. When they receive that, the saladmakers check how many salads are left to make. If it's 0, they signal the chef with the "I took the ingredients off the table" semaphore, and then they terminate.

When the chef receives the signal that the saladmakers have terminated, he closes all the semaphores and the shared memory, and reads the saladmakers' log files.
There's 7 log files. One is the shared file where everyone appends what they do in real time. The other 3 are one for each saladmaker, where they write the same stuff as in the shared file. There are 3 more files, one for each saladmaker, where they write the moment they become active and they moment they stop being active. In this extra file, the saladmakers write seconds and milliseconds from 1970, instead of the formatted time asked in the assignment, because it's a lot easier for the chef to extract when 2 or more were working at once.
I could have only one extra file for all 3 saladmakers, but then I would need another semaphore to protect it, and it would introduce unnecessary delays (theoretically). So I have a separate extra file for each saladmaker.

The chef reads the 3 extra files from the saladmakers and puts the times in an ordered list, where each node contains seconds from 1970, milliseconds, and "start"/"stop". Then the chef goes through the list once, and at each node he keeps track of how many "start"s and "stop"s he's seen. When that number was 1 and it becomes 2, it means 2 or more saladmakers were running at once. When that number was 2 and it becomes 1, it means only one saladmaker was running from then on.

The saladmakers start by, again, reading their arguments. Then they open the shared memory. When each saladmaker starts, he looks at the 3 PIDs in the shared memory. If the first one is 0, he knows he's the first one and he sets it to his PID. Each saladmaker has pointers to some of the stuff in the shared memory, and there's an IF where he sets those pointers to the correct thing in the shared memory depending on which saladmaker he is, 1, 2 or 3.
