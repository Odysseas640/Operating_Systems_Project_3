CC=g++
CFLAGS=-Wall -std=c++11 -ggdb3
CFLAGS2=-Wall -std=c++11 -ggdb3 -pthread

all: saladmaker.o chef.o times_list.o saladmaker chef

saladmaker.o: saladmaker.cpp
	$(CC) $(CFLAGS) -c saladmaker.cpp

chef.o: chef.cpp
	$(CC) $(CFLAGS) -c chef.cpp

times_list.o: times_list.cpp
	$(CC) $(CFLAGS) -c times_list.cpp

saladmaker: saladmaker.o
	$(CC) $(CFLAGS2) -o saladmaker saladmaker.o

chef: chef.o times_list.o
	$(CC) $(CFLAGS2) -o chef chef.o times_list.o

.PHONY: clean

clean:
	rm -f saladmaker.o chef.o times_list.o ordered_list.o saladmaker chef