#include "times_list.h"

TimesList::TimesList() {
	this->times_list = NULL;
}
int TimesList::insert(char* line) {
	char* saveptr = line;
	int start_stopp = 0;
	if (strncmp(line, "START", 5) == 0)
		start_stopp = 0;
	else if (strncmp(line, "STOP", 4) == 0)
		start_stopp = 1;
	else
		cout << "MISTAKE IN TIMES LIST" << endl;
	strtok_r(saveptr, ",", &saveptr); // Skip "START" or "STOP"
	int secondz = atoi(strtok_r(saveptr, ",", &saveptr));
	int millisecondz = atoi(strtok_r(saveptr, ",", &saveptr));
	if (this->times_list == NULL) {
		this->times_list = new TimesListNode;
		this->times_list->seconds = secondz;
		this->times_list->milliseconds = millisecondz;
		this->times_list->start_stop = start_stopp;
		this->times_list->Next = NULL;
		return 0;
	}
	TimesListNode* temp_node = this->times_list;
	if (secondz < this->times_list->seconds || (secondz < this->times_list->seconds && millisecondz == this->times_list->milliseconds)) {
		TimesListNode* new_node = new TimesListNode;
		new_node->seconds = secondz;
		new_node->milliseconds = millisecondz;
		new_node->start_stop = start_stopp;
		this->times_list = new_node;
		new_node->Next = temp_node;
	}
	else {
		while (temp_node->Next != NULL) {
			if (secondz < temp_node->Next->seconds || (secondz == temp_node->Next->seconds && millisecondz < temp_node->Next->milliseconds)) {
				break;
			}
			temp_node = temp_node->Next;
		}
		TimesListNode* new_node = new TimesListNode;
		new_node->seconds = secondz;
		new_node->milliseconds = millisecondz;
		new_node->start_stop = start_stopp;
		new_node->Next = temp_node->Next;
		temp_node->Next = new_node;
	}
	return 0;
}
void TimesList::print() {
	cout << "TimesList print" << endl;
	TimesListNode* current_node = this->times_list;
	while (current_node != NULL) {
		cout << current_node->seconds << "." << current_node->milliseconds << " " << current_node->start_stop << endl;
		current_node = current_node->Next;
	}
	cout << "TimesList print end" << endl;
}
void TimesList::print_matching_times() {
	cout << "TimesList print matching times" << endl;
	struct timeval tv;
	struct tm *info;
	char buffer[64];
	int n_running = 0;
	TimesListNode* current_node = this->times_list;
	while (current_node != NULL) {
		if (current_node->start_stop == 0)
			n_running++;
		else if (current_node->start_stop == 1)
			n_running--;
		if (n_running == 2 && current_node->start_stop == 0) {
			tv.tv_sec = current_node->seconds;
			tv.tv_usec = current_node->milliseconds;
			info = localtime(&(tv.tv_sec));
			strftime(buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			cout << "[" << buffer << "." << current_node->milliseconds << ", ";
		}
		else if (n_running == 1 && current_node->start_stop == 1) {
			tv.tv_sec = current_node->seconds;
			tv.tv_usec = current_node->milliseconds;
			info = localtime(&(tv.tv_sec));
			strftime(buffer, sizeof(buffer), "%d:%I:%M:%S", info);
			cout << buffer << "." << current_node->milliseconds << "]" << endl;
		}
		current_node = current_node->Next;
		// cout << "Running: " << n_running << endl;
	}
	cout << "TimesList print matching times end" << endl;
}
TimesList::~TimesList() {
	TimesListNode* current_node = this->times_list;
	while (current_node != NULL) {
		TimesListNode* to_delete = current_node;
		current_node = current_node->Next;
		delete to_delete;
	}
}