#ifndef HELP_FUNCTIONS_H
#define HELP_FUNCTIONS_H

#include <stdbool.h>
#include <dirent.h>

#include "record.h"
#include "citizen.h"

FILE* read_arguments(int argc, char** argv, int* bloomSize);		//reads arguments from command line
void fill_record(char* line, Record* temp);		//breaks line into tokens and creates a new record
bool find_conflict(Record record, Citizen* citizen);			//finds if any value is different from that of the citizen
void free_record(Record* temp);		//free

DIR* read_arguments_for_travel_monitor(int argc, char** argv, int* bloomSize, int *bufferSize, int *numMonitors);		//reads arguments from command line
void read_arguments_for_vaccine_monitor(int argc, char** argv, int* bloomSize, int *bufferSize, int *numMonitors, int * id);		//reads arguments from command line

#endif
