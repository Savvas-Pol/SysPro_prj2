#ifndef HELP_FUNCTIONS_H
#define HELP_FUNCTIONS_H

#include <stdbool.h>
#include <dirent.h>

#include "record.h"
#include "citizen.h"
#include "hashtable_country.h"
#include "hashtable_monitor.h"
#include "hashtable_virus.h"

FILE* read_arguments(int argc, char** argv, int* bloomSize); //reads arguments from command line
void fill_record(char* line, Record* temp); //breaks line into tokens and creates a new record
bool find_conflict(Record record, Citizen* citizen); //finds if any value is different from that of the citizen
void free_record(Record* temp); //free

DIR* read_arguments_for_travel_monitor(int argc, char** argv, int* bloomSize, int *bufferSize, int *numMonitors, char ** inputDirectoryPath); //reads arguments from command line
void read_arguments_for_vaccine_monitor(int argc, char** argv, int* bloomSize, int *bufferSize, int *numMonitors, int * id); //reads arguments from command line

void send_info(int fd, char *info, int infolength, int bufferSize);

int receive_info(int fd, char **info, int bufferSize);

int receive_int(int fd, int buffersize);

void respawn_child(HashtableMonitor* ht_monitors, HashtableVirus* ht_viruses, int bloomSize, int bufferSize, char *inputDirectoryPath, int argc, char** argv, HashtableCountryNode** table, int tablelen);

#endif
