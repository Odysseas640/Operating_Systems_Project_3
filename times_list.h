#ifndef __ODYS_TIMES_LIST__
#define __ODYS_TIMES_LIST__
#include <iostream>
#include <cstring>
#include <cmath>
#include <sys/time.h>
using namespace std;

typedef struct times_list_node TimesListNode;
struct times_list_node {
	int seconds;
	int milliseconds;
	int start_stop;
	TimesListNode* Next;
};

class TimesList {
private:
	TimesListNode* times_list;
public:
	TimesList();
	int insert(char* line);
	void print();
	void print_matching_times();
	~TimesList();
};

#endif