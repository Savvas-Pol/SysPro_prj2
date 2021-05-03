#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>

#include "help_functions.h"
#include "hashtable_virus.h"
#include "hashtable_citizen.h"
#include "hashtable_country.h"
#include "hashtable_monitor.h"
#include "BF.h"
#include "record.h"
#include "commands.h"
#include "constants.h"
#include "commands_travelmonitor.h"


// "${OUTPUT_PATH}" -m 4 -b 2000 -s 1000 -i data_small
int vaccine_monitor_main(int argc, char** argv);

int main(int argc, char** argv) {

    /*  ---     DECLARATIONS    --- */

    int bloomSize, bufferSize, numMonitors, j;
    char* token;

    char* line = NULL;

    DIR* inputDirectory;
    struct dirent *direntp;

    /*      ---------------     */

    if ((inputDirectory = read_arguments_for_travel_monitor(argc, argv, &bloomSize, &bufferSize, &numMonitors)) == NULL) { //read arguments from command line
        return -1;
    }

    HashtableVirus* ht_viruses = hash_virus_create(HASHTABLE_NODES); //create HashTable for viruses
    HashtableCountry* ht_countries = hash_country_create(HASHTABLE_NODES); //create HashTable for countries
    HashtableMonitor* ht_monitors = hash_monitor_create(HASHTABLE_NODES);

    while ((direntp = readdir(inputDirectory)) != NULL) {
        if (direntp->d_name[0] != '.') {
            printf("inode %d of the entry %s \n", (int) direntp->d_ino, direntp->d_name);

            HashtableCountryNode* cc = hash_country_search(ht_countries, direntp->d_name);
            if (cc == NULL) {
                hash_country_insert(ht_countries, direntp->d_name);
            }
        }
    }

    closedir(inputDirectory);

    for (j = 0; j < numMonitors; j++) {
        char name[100];
        sprintf(name, "%d", j);
        hash_monitor_insert(ht_monitors, name);
    }

    for (j = 0; j < numMonitors; j++) {
        char name[100];
        sprintf(name, "%d", j);

        HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);
        sprintf(node->from_child_to_parent, "from_child_to_parent_%d.fifo", j);
        sprintf(node->from_parent_to_child, "from_parent_to_child_%d.fifo", j);

        printf("Monitor Node: j=%d, id=%s, %s, %s \n", j, node->monitorName, node->from_child_to_parent, node->from_parent_to_child);

        mkfifo(node->from_child_to_parent, 0600);
        mkfifo(node->from_parent_to_child, 0600);
    }

    for (j = 0; j < numMonitors; j++) {
        char name[100];
        sprintf(name, "%d", j);

        HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

        pid_t pid = fork();

        if (pid > 0) {
            node->pid = pid;

            if ((node->fd_from_parent_to_child = open(node->from_parent_to_child, O_WRONLY)) < 0) {
                perror("travelMonitor: can't open read fifo");
                exit(1);
            }

            if ((node->fd_from_child_to_parent = open(node->from_child_to_parent, O_RDONLY)) < 0) {
                perror("travelMonitor: can't open write fifo");
                exit(1);
            }
        } else if (pid == 0) {
            char buf_m[10];
            char buf_b[10];
            char buf_s[10];

            sprintf(buf_m, "%d", numMonitors);
            sprintf(buf_b, "%d", bufferSize);
            sprintf(buf_s, "%d", bloomSize);

            argc = 9;
            argv = malloc(sizeof (char*)*10);
            argv[0] = "vaccineMonitor";
            argv[1] = "-m";
            argv[2] = buf_m;
            argv[3] = "-b";
            argv[4] = buf_b;
            argv[5] = "-s";
            argv[6] = buf_s;
            argv[7] = "-i";
            argv[8] = name;
            argv[9] = NULL;
            return vaccine_monitor_main(argc, argv);
        }
    }

    int tablelen;

    HashtableCountryNode** table = hash_country_to_array(ht_countries, &tablelen);

    char * buffer = malloc(bufferSize);

    for (j = 0; j < tablelen; j++) {
        char * country = table[j]->countryName;
        table[j]->who = j % numMonitors;

        char name[100];
        sprintf(name, "%d", table[j]->who);

        HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

        int fd = node->fd_from_parent_to_child;

        printf("Sending country :%s to worker %d through pipe: %s via fd: %d \n", country, table[j]->who, node->from_parent_to_child, fd);

        strcpy(buffer, country);
        write(fd, buffer, bufferSize);
    }


    for (j = 0; j < numMonitors; j++) {
        char name[100];
        sprintf(name, "%d", j);

        HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

        int fd = node->fd_from_parent_to_child;

        strcpy(buffer, "#");
        write(fd, buffer, bufferSize);
    }


    while (1) { //commands from user
        size_t len = 0;
        printf("\nGive command: ");
        getline(&line, &len, stdin);
        token = strtok(line, " \n");

        if (token != NULL) {

            if (!strcmp(token, "/travelRequest") || !strcmp(token, "travelRequest")) {
	        	char* tokens[6];

	        	tokens[0] = strtok(NULL, " \n"); //citizenID
	        	tokens[1] = strtok(NULL, " \n"); //date
	        	tokens[2] = strtok(NULL, " \n"); //countryFrom
	        	tokens[3] = strtok(NULL, " \n"); //countryTo
	        	tokens[4] = strtok(NULL, " \n"); //virusName
	        	tokens[5] = strtok(NULL, " \n"); //NULL

	        	if (tokens[0] == NULL || tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL || tokens[4] == NULL || tokens[5] != NULL) {
	        		printf("syntax error\n");
	        	} else {
	        		travel_request(ht_viruses, ht_countries, ht_monitors, bloomSize, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4]);
	        	}
            } else if (!strcmp(token, "/travelStats") || !strcmp(token, "travelStats")) {
            	char* tokens[5];

	        	tokens[0] = strtok(NULL, " \n"); //virusName
	        	tokens[1] = strtok(NULL, " \n"); //date1
	        	tokens[2] = strtok(NULL, " \n"); //date2
	        	tokens[3] = strtok(NULL, " \n"); //country
	        	tokens[4] = strtok(NULL, " \n"); //NULL

	        	if (tokens[0] == NULL) {
	        		printf("syntax error\n");
	        	} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] == NULL){
	        		travel_stats(ht_viruses, ht_countries, ht_monitors, bloomSize, tokens[0], tokens[1], tokens[2]);
	        	} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] != NULL && tokens[4] == NULL){
	        		travel_stats_country(ht_viruses, ht_countries, ht_monitors, bloomSize, tokens[0], tokens[1], tokens[2], tokens[3]);
	        	} else {
	        		printf("syntax error\n");
	        	}
            } else if (!strcmp(token, "/addVaccinationRecords") || !strcmp(token, "addVaccinationRecords")) {
	        	char* tokens[2];

	        	tokens[0] = strtok(NULL, " \n"); //country
	        	tokens[1] = strtok(NULL, " \n"); //NULL

	        	if (tokens[0] == NULL || tokens[1] != NULL) {
	        		printf("syntax error\n");
	        	} else {
	        		add_vaccination_records(ht_viruses, ht_countries, ht_monitors, bloomSize, tokens[0]);
	        	}
            } else if (!strcmp(token, "/searchVaccinationStatus") || !strcmp(token, "searchVaccinationStatus")) {
            	char* tokens[2];

	        	tokens[0] = strtok(NULL, " \n"); //citizenID
	        	tokens[1] = strtok(NULL, " \n"); //NULL

	        	if (tokens[0] == NULL || tokens[1] != NULL) {
	        		printf("syntax error\n");
	        	} else {
	        		search_vaccination_status(ht_viruses, ht_countries, ht_monitors, bloomSize, tokens[0]);
	        	}
            } else if (!strcmp(token, "/exit") || !strcmp(token, "exit")) {
                break;
            }
        }
    }

    // frees

    if (line != NULL) {
        free(line);
    }

    for (j = 0; j < numMonitors; j++) {
        char name[100];
        sprintf(name, "%d", j);

        HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

        waitpid(node->pid, 0, 0);

        close(node->fd_from_child_to_parent);
        close(node->fd_from_parent_to_child);

        unlink(node->from_child_to_parent);
        unlink(node->from_parent_to_child);
    }


    hash_virus_destroy(ht_viruses);
    hash_country_destroy(ht_countries);
    hash_monitor_destroy(ht_monitors);

    free(buffer);

    return 0;
}