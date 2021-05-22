#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
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
#include "commands_vaccinemonitor.h"
#include "constants.h"
#include "commands_travelmonitor.h"
#include "skiplist.h"

int quit = 0;
//int child = 0;

int vaccine_monitor_main(int argc, char** argv);

void catchinterrupt(int signo) {
	printf("\nCatching: signo=%d\n", signo);
	printf("Catching: returning\n");

	quit = 1;
}

int main(int argc, char** argv) {

	/*  ---     DECLARATIONS    --- */

	int bloomSize, bufferSize, numMonitors, i, j, requestID = 0;
	int totalAccepted = 0, totalRejected = 0;
	char* token;
	char *inputDirectoryPath = NULL;

	char* line = NULL;

	DIR* inputDirectory;
	struct dirent *direntp;

	FILE* logfile;

	static struct sigaction act;

	/*      ---------------     */

	if ((inputDirectory = read_arguments_for_travel_monitor(argc, argv, &bloomSize, &bufferSize, &numMonitors, &inputDirectoryPath)) == NULL) { //read arguments from command line
		return -1;
	} else {
		act.sa_handler = catchinterrupt;
		sigfillset(&(act.sa_mask));

		sigaction(SIGINT, &act, NULL);
		sigaction(SIGQUIT, &act, NULL);
		//sigaction(SIGCHLD, &act, NULL);
	}

	HashtableVirus* ht_viruses = hash_virus_create(HASHTABLE_NODES); //create HashTable for viruses
	HashtableCountry* ht_countries = hash_country_create(HASHTABLE_NODES); //create HashTable for countries
	HashtableMonitor* ht_monitors = hash_monitor_create(HASHTABLE_NODES); //create HashTable for monitors

	while ((direntp = readdir(inputDirectory)) != NULL) {
		if (direntp->d_name[0] != '.') {
			HashtableCountryNode* country = hash_country_search(ht_countries, direntp->d_name);
			if (country == NULL) {
				hash_country_insert(ht_countries, direntp->d_name);		//insert countries in parent's hashtable
			}
		}
	}
	closedir(inputDirectory);

	for (j = 0; j < numMonitors; j++) {
		char name[100];
		sprintf(name, "%d", j);						//monitor names
		hash_monitor_insert(ht_monitors, name);		//insert monitors in parent's hashtable
	}

	create_pipes(ht_monitors, numMonitors);

	for (j = 0; j < numMonitors; j++) {
		char name[100];
		sprintf(name, "%d", j);

		HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

		pid_t pid = fork();

		if (pid > 0) { //parent
			node->pid = pid;

			if ((node->fd_from_parent_to_child = open(node->from_parent_to_child, O_WRONLY)) < 0) {
				perror("travelMonitor: can't open read fifo");
				exit(1);
			}

			if ((node->fd_from_child_to_parent = open(node->from_child_to_parent, O_RDONLY)) < 0) {
				perror("travelMonitor: can't open write fifo");
				exit(1);
			}

			char * info1 = (char *) &bloomSize;
			int info_length1 = sizeof (bloomSize);

			send_info(node->fd_from_parent_to_child, info1, info_length1, info_length1); //first message is bloomSize

			char * info2 = (char *) &bufferSize;
			int info_length2 = sizeof (bufferSize);

			send_info(node->fd_from_parent_to_child, info2, info_length2, info_length2); //second message is bufferSize

			char * info3 = inputDirectoryPath;
			int info_length3 = strlen(inputDirectoryPath) + 1;

			send_info(node->fd_from_parent_to_child, info3, info_length3, info_length3);
		} else if (pid == 0) { //child
			argc = 3;
			argv = malloc(sizeof (char*)*4);
			argv[0] = "vaccineMonitor";
			argv[1] = "-i";
			argv[2] = name;
			argv[3] = NULL;
			
			//execvp("./vaccineMonitor", argv);
			return vaccine_monitor_main(argc, argv);
		}
	}
	//only parent continues from now on
	int tablelen;
	HashtableCountryNode** table = hash_country_to_array(ht_countries, &tablelen); //convert hash table to array to sort countries

	send_countries_to_monitors(ht_monitors, table, tablelen, numMonitors, bufferSize); //send countries round robin to monitors
	send_finishing_character(ht_monitors, numMonitors, bufferSize); //send finishing character "#" to all monitors
	receive_bloom_filter(ht_monitors, ht_viruses, numMonitors, bloomSize, bufferSize);

	int vtablelen;
	HashtableVirusNode** vtable = hash_virus_to_array(ht_viruses, &vtablelen);

	while (quit != 1) { //commands from user
		size_t len = 0;

		sigset_t set1;
		sigemptyset(&set1);

		sigprocmask(SIG_SETMASK, &set1, NULL); // allow everything here!

		printf("\nGive command: ");
		if (getline(&line, &len, stdin) == 0 || quit == 1) {
			break;
		}
		token = strtok(line, " \n");

		sigaddset(&set1, SIGINT);
		sigaddset(&set1, SIGQUIT);
		sigaddset(&set1, SIGUSR1);
		//sigaddset(&set1, SIGCHLD);

		sigprocmask(SIG_SETMASK, &set1, NULL); // disallow everything here!

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
					travel_request(ht_viruses, ht_countries, ht_monitors, bloomSize, bufferSize, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], requestID, &totalAccepted, &totalRejected);
					requestID++;
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
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] == NULL) {
					travel_stats(ht_viruses, ht_countries, ht_monitors, bloomSize, tokens[0], tokens[1], tokens[2]);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] != NULL && tokens[4] == NULL) {
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
					add_vaccination_records(ht_viruses, ht_countries, ht_monitors, bloomSize, bufferSize, tokens[0]);
				}
			} else if (!strcmp(token, "/searchVaccinationStatus") || !strcmp(token, "searchVaccinationStatus")) {
				char* tokens[2];

				tokens[0] = strtok(NULL, " \n"); //citizenID
				tokens[1] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL || tokens[1] != NULL) {
					printf("syntax error\n");
				} else {
					search_vaccination_status(ht_viruses, ht_countries, ht_monitors, bloomSize, bufferSize, numMonitors, tokens[0]);
				}
			} else if (!strcmp(token, "/exit") || !strcmp(token, "exit")) {
				break;
			} else {
				printf("Invalid command!! Try again...\n");
			}
		}
	}

	char logfile_name[100];
	sprintf(logfile_name, "log_file.%d", getpid());
	logfile = fopen(logfile_name, "w");
	if (logfile == NULL) {
		printf("Error! Could not open file\n");
		exit(-1);
	}
	HashtableCountryNode* temp;
	for (i = 0; i < HASHTABLE_NODES; i++) {
		temp = ht_countries->nodes[i];
		while (temp != NULL) {
			fprintf(logfile, "%s\n", temp->countryName);
			temp = temp->next;
		}
	}
	fprintf(logfile, "TOTAL TRAVEL REQUESTS %d\n", totalAccepted + totalRejected);
	fprintf(logfile, "ACCEPTED %d\n", totalAccepted);
	fprintf(logfile, "REJECTED %d\n", totalRejected);

	// frees
	for (j = 0; j < numMonitors; j++) {
		HashtableMonitorNode* node = hash_monitor_search_with_int(ht_monitors, j);
		kill(node->pid, SIGKILL);
	}

	if (line != NULL) {
		free(line);
	}

	for (j = 0; j < numMonitors; j++) {
		HashtableMonitorNode* node = hash_monitor_search_with_int(ht_monitors, j);

		waitpid(node->pid, 0, 0);

		close(node->fd_from_child_to_parent);
		close(node->fd_from_parent_to_child);

		unlink(node->from_child_to_parent);
		unlink(node->from_parent_to_child);
	}

	hash_virus_destroy(ht_viruses);
	hash_country_destroy(ht_countries);
	hash_monitor_destroy(ht_monitors);

	free(table);
	free(vtable);

	return 0;
}