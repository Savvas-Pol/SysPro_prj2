#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>

#include "help_functions.h"
#include "hashtable_virus.h"
#include "hashtable_citizen.h"
#include "hashtable_country.h"
#include "hashtable_filenames.h"
#include "BF.h"
#include "record.h"
#include "commands_vaccinemonitor.h"
#include "constants.h"

int writelog = 0;
int add_records = 0;

void catchinterrupt2(int signo) {
	printf("\nCatching: signo=%d\n", signo);
	printf("Catching: returning\n");

	writelog = 1;
}

void catchinterrupt_act_add_records(int signo) {
	printf("\nCatching: signo=%d\n", signo);
	printf("Catching: returning\n");

	add_records = 1;
}

//int main(int argc, char** argv) {
int vaccine_monitor_main(int argc, char** argv) {

	/*  ---     DECLARATIONS    --- */

	int bloomSize = -1, bufferSize = -1, numMonitors = -1, j, id;
	char* token;
	char *inputDirectoryPath = NULL;

	char* line = NULL;
	size_t len = 0;

	DIR* inputDirectory = NULL;
	struct dirent *direntp;

	char from_child_to_parent[1000];
	char from_parent_to_child[1000];

	static struct sigaction act;
	static struct sigaction act_add_records;

	/*      ---------------     */

	srand(time(0));

	read_arguments_for_vaccine_monitor(argc, argv, &bloomSize, &bufferSize, &numMonitors, &id);

	act.sa_handler = catchinterrupt2;
	sigfillset(&(act.sa_mask));

	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	//sigaction(SIGCHLD, &act, NULL);

	act_add_records.sa_handler = catchinterrupt_act_add_records;
	sigfillset(&(act_add_records.sa_mask));

	sigaction(SIGUSR1, &act_add_records, NULL);

	sprintf(from_child_to_parent, "from_child_to_parent_%d.fifo", id);
	sprintf(from_parent_to_child, "from_parent_to_child_%d.fifo", id);

	printf("Child started: s:%d-b:%d-m:%d-id:%d, %s, %s\n", bloomSize, bufferSize, numMonitors, id, from_child_to_parent, from_parent_to_child);

	int readfd, writefd;

	if ((readfd = open(from_parent_to_child, O_RDONLY)) < 0) {
		perror("vaccineMonitor: can't open read fifo");
	}

	if ((writefd = open(from_child_to_parent, O_WRONLY)) < 0) {
		perror("vaccineMonitor: can't open write fifo");
	}

	printf("Child: <%d>: waiting for bloom size and buffer size ... \n", id);

	bloomSize = receive_int(readfd, sizeof (int));
	bufferSize = receive_int(readfd, sizeof (int));
	receive_info(readfd, &inputDirectoryPath, bufferSize);

	printf("Child: <%d>: waiting for countries (bloom:%d, buffer:%d, dir:%s )... \n", id, bloomSize, bufferSize, inputDirectoryPath);

	HashtableVirus* ht_viruses = hash_virus_create(HASHTABLE_NODES); //create HashTable for viruses
	HashtableCitizen* ht_citizens = hash_citizen_create(HASHTABLE_NODES); //create HashTable for citizens
	HashtableCountry* ht_countries = hash_country_create(HASHTABLE_NODES); //create HashTable for countries
	HashtableFilenames* ht_filenames = hash_filenames_create(HASHTABLE_NODES); //create HashTable for filenames

	while (1) {
		char * info3 = NULL;
		receive_info(readfd, &info3, bufferSize);

		char * buffer = info3;

		if (buffer[0] == '#') {
			break;
		}

		printf("Child: <%d>: country received: %s ... \n", id, buffer);

		HashtableCountryNode* cc = hash_country_search(ht_countries, buffer);
		if (cc == NULL) {
			hash_country_insert(ht_countries, buffer);
		}

		char * buffer4 = malloc(strlen(inputDirectoryPath) + 1 + strlen(buffer) + 1);
		strcpy(buffer4, inputDirectoryPath);
		strcat(buffer4, "/");
		strcat(buffer4, buffer);

		if (!(inputDirectory = opendir(buffer4))) {
			printf("Error in opening %s\n", buffer4);
		} else {
			while ((direntp = readdir(inputDirectory)) != NULL) {
				if (direntp->d_name[0] != '.') {
					printf("inode %d of the entry %s \n", (int) direntp->d_ino, direntp->d_name);

					char * buffer5 = malloc(strlen(inputDirectoryPath) + 1 + strlen(buffer) + 1 + strlen(direntp->d_name) + 1);
					strcpy(buffer5, inputDirectoryPath);
					strcat(buffer5, "/");
					strcat(buffer5, buffer);
					strcat(buffer5, "/");
					strcat(buffer5, direntp->d_name);

					FILE * citizenRecordsFile = fopen(buffer5, "r");

					if (citizenRecordsFile != NULL) {
						int r;

						hash_filenames_insert(ht_filenames, buffer5);

						while ((r = getline(&line, &len, citizenRecordsFile)) != -1) { //read file line by line
							Record record;

							fill_record(line, &record); //create a temp record
							insert_citizen_record(ht_viruses, ht_citizens, ht_countries, bloomSize, record, 1); //flag=1 means from file
							free_record(&record); //free temp record
						}

						fclose(citizenRecordsFile);
						free(buffer5);
					}
				}
			}
		}

		free(buffer4);

	}

	printf("Child: <%d>: building structures ... \n", id);

	int tablelen;

	HashtableVirusNode** table = hash_virus_to_array(ht_viruses, &tablelen);

	for (j = 0; j < tablelen; j++) {
		char * virus = table[j]->virusName;

		printf("Sending disease :%s to parent through pipe: %s via fd: %d \n", virus, from_child_to_parent, writefd);

		char * info1 = (char *) virus;
		int info_length1 = strlen(virus) + 1;

		send_info(writefd, info1, info_length1, bufferSize);

		char * info2 = table[j]->bloom->vector;
		int info_length2 = bloomSize;

		send_info(writefd, info2, info_length2, bufferSize);
	}

	char buffer[2] = "#";
	char * info1 = (char *) buffer;
	int info_length1 = strlen(buffer) + 1;
	send_info(writefd, info1, info_length1, bufferSize);

	//closedir(inputDirectory);

	while (1) {
		char * line = NULL;

		if (writelog == 1) {
			writelog = 0;
			// writelog
		}

		if (add_records == 1) {
			add_vaccination_records_for_child(inputDirectoryPath, ht_filenames, ht_citizens, ht_countries, ht_viruses, table, tablelen, bloomSize, from_child_to_parent, bufferSize, readfd, writefd);
			add_records = 0;
		}

		sigset_t set1;
		sigemptyset(&set1);

		sigprocmask(SIG_SETMASK, &set1, NULL); // allow everything here!

		// read from pipe instead of stdin
		receive_info(readfd, &line, bufferSize);

		if (writelog == 1) {
			continue;
		}

		if (add_records == 1) {
			continue;
		}

		if (line == NULL) {
			continue;
		}

		token = strtok(line, " \n");

		sigaddset(&set1, SIGINT);
		sigaddset(&set1, SIGQUIT);
		sigaddset(&set1, SIGUSR1);

		sigprocmask(SIG_SETMASK, &set1, NULL); // disallow everything here!

		if (token != NULL) {

			if (!strcmp(token, "travelRequest")) {
				char* tokens[5];

				tokens[0] = strtok(NULL, " \n"); //citizenID
				tokens[1] = strtok(NULL, " \n"); //date
				tokens[2] = strtok(NULL, " \n"); //country
				tokens[3] = strtok(NULL, " \n"); //country
				tokens[4] = strtok(NULL, " \n"); //virusName

				travel_request_for_child(ht_viruses, ht_citizens, tokens[0], tokens[1], tokens[2], tokens[4], readfd, writefd, bufferSize);
			} else if (!strcmp(token, "searchVaccinationStatus")) {
				char* tokens[1];

				tokens[0] = strtok(NULL, " \n"); //citizenID

				search_vaccination_status_for_child(ht_viruses, ht_countries, ht_citizens, bloomSize, bufferSize, readfd, writefd, tokens[0]);
			} else if (!strcmp(token, "/vaccineStatusBloom")) {
				char* tokens[3];

				tokens[0] = strtok(NULL, " \n"); //citizenID
				tokens[1] = strtok(NULL, " \n"); //virusName
				tokens[2] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL || tokens[1] == NULL || tokens[2] != NULL) {
					printf("syntax error\n");
				} else {
					vaccine_status_bloom(ht_viruses, tokens[0], tokens[1]);
				}
			} else if (!strcmp(token, "/vaccineStatus")) {
				char* tokens[3];

				tokens[0] = strtok(NULL, " \n"); //citizenID
				tokens[1] = strtok(NULL, " \n"); //virusName
				tokens[2] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL) {
					printf("syntax error\n");
				} else if (tokens[0] != NULL && tokens[1] == NULL) {
					vaccine_status_id(ht_viruses, ht_citizens, tokens[0], bufferSize, readfd, writefd);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] == NULL) {
					vaccine_status_id_virus(ht_viruses, ht_citizens, tokens[0], tokens[1], NULL);
				} else { // more than 2
					printf("syntax error\n");
				}
			} else if (!strcmp(token, "/populationStatus")) {
				char* tokens[5];

				tokens[0] = strtok(NULL, " \n"); //country
				tokens[1] = strtok(NULL, " \n"); //virusName
				tokens[2] = strtok(NULL, " \n"); //date1
				tokens[3] = strtok(NULL, " \n"); //date2
				tokens[4] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL) {
					printf("syntax error\n");
				} else if (tokens[0] != NULL && tokens[1] == NULL) {
					population_status_virus(ht_viruses, ht_citizens, ht_countries, tokens[0]);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] == NULL) {
					population_status_country(ht_viruses, ht_countries, tokens[0], tokens[1]);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] == NULL) {
					population_status_virus_dates(ht_viruses, ht_countries, tokens[0], tokens[1], tokens[2]);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] != NULL && tokens[4] == NULL) {
					population_status_country_dates(ht_viruses, ht_countries, tokens[0], tokens[1], tokens[2], tokens[3]);
				} else {
					printf("syntax error\n");
				}
			} else if (!strcmp(token, "/popStatusByAge")) {
				char* tokens[5];

				tokens[0] = strtok(NULL, " \n"); //country
				tokens[1] = strtok(NULL, " \n"); //virusName
				tokens[2] = strtok(NULL, " \n"); //date1
				tokens[3] = strtok(NULL, " \n"); //date2
				tokens[4] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL) {
					printf("syntax error\n");
				} else if (tokens[0] != NULL && tokens[1] == NULL) {
					pop_status_by_age_virus(ht_viruses, ht_countries, tokens[0]);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] == NULL) {
					pop_status_by_age_country(ht_viruses, ht_countries, tokens[0], tokens[1]);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] == NULL) {
					pop_status_by_age_virus_dates(ht_viruses, ht_countries, tokens[0], tokens[1], tokens[2]);
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] != NULL && tokens[4] == NULL) {
					pop_status_by_age_country_dates(ht_viruses, ht_countries, tokens[0], tokens[1], tokens[2], tokens[3]);
				} else {
					printf("syntax error\n");
				}
			} else if (!strcmp(token, "/insertCitizenRecord")) {
				char* tokens[9];
				Record record = {0};

				tokens[0] = strtok(NULL, " \n"); //citizenID
				tokens[1] = strtok(NULL, " \n"); //firstName
				tokens[2] = strtok(NULL, " \n"); //lastName
				tokens[3] = strtok(NULL, " \n"); //country
				tokens[4] = strtok(NULL, " \n"); //age
				tokens[5] = strtok(NULL, " \n"); //virusName
				tokens[6] = strtok(NULL, " \n"); //YES/NO
				tokens[7] = strtok(NULL, " \n"); //date
				tokens[8] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL || tokens[8] != NULL) {
					printf("syntax error\n");
				} else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] != NULL && tokens[3] != NULL && tokens[4] != NULL && tokens[5] != NULL && tokens[6] != NULL && tokens[7] != NULL) {
					//YES
					record.citizenID = malloc((strlen(tokens[0])) + 1);
					record.firstName = malloc((strlen(tokens[1])) + 1);
					record.lastName = malloc((strlen(tokens[2])) + 1);
					record.country = malloc((strlen(tokens[3])) + 1);
					record.virusName = malloc((strlen(tokens[5])) + 1);

					strcpy(record.citizenID, tokens[0]);
					strcpy(record.firstName, tokens[1]);
					strcpy(record.lastName, tokens[2]);
					strcpy(record.country, tokens[3]);
					record.age = atoi(tokens[4]);
					strcpy(record.virusName, tokens[5]);

					record.dateVaccinated = malloc(sizeof (Date));

					token = strtok(tokens[7], "-");

					j = 0;

					while (token != NULL) {
						if (j == 0)
							record.dateVaccinated->day = atoi(token);
						else if (j == 1)
							record.dateVaccinated->month = atoi(token);
						else if (j == 2)
							record.dateVaccinated->year = atoi(token);
						token = strtok(NULL, "-\n");
						j++;
					}

					insert_citizen_record(ht_viruses, ht_citizens, ht_countries, bloomSize, record, 0); //flag=0 means from file
					free_record(&record);
				} else { //NO
					record.citizenID = malloc((strlen(tokens[0])) + 1);
					record.firstName = malloc((strlen(tokens[1])) + 1);
					record.lastName = malloc((strlen(tokens[2])) + 1);
					record.country = malloc((strlen(tokens[3])) + 1);
					record.virusName = malloc((strlen(tokens[5])) + 1);

					strcpy(record.citizenID, tokens[0]);
					strcpy(record.firstName, tokens[1]);
					strcpy(record.lastName, tokens[2]);
					strcpy(record.country, tokens[3]);
					record.age = atoi(tokens[4]);
					strcpy(record.virusName, tokens[5]);
					record.dateVaccinated = NULL;

					insert_citizen_record(ht_viruses, ht_citizens, ht_countries, bloomSize, record, 0);

					free_record(&record);
				}
			} else if (!strcmp(token, "/vaccinateNow")) {
				char* tokens[7];

				tokens[0] = strtok(NULL, " \n"); //citizenID
				tokens[1] = strtok(NULL, " \n"); //firstName
				tokens[2] = strtok(NULL, " \n"); //lastName
				tokens[3] = strtok(NULL, " \n"); //country
				tokens[4] = strtok(NULL, " \n"); //age
				tokens[5] = strtok(NULL, " \n"); //virusName
				tokens[6] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL || tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL || tokens[4] == NULL || tokens[5] == NULL || tokens[6] != NULL) {
					printf("syntax error\n");
				} else {
					vaccinate_now(ht_viruses, ht_citizens, ht_countries, bloomSize, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
				}
			} else if (!strcmp(token, "/list-nonVaccinated-Persons")) {
				char* tokens[2];

				tokens[0] = strtok(NULL, " \n"); //virusName
				tokens[1] = strtok(NULL, " \n"); //NULL

				if (tokens[0] == NULL || tokens[1] != NULL) {
					printf("syntax error\n");
				} else {
					list_nonVaccinated_Persons(ht_viruses, tokens[0]);
				}
			} else if (!strcmp(token, "/exit")) {
				break;
			}
		}
	}

	// frees

	if (line != NULL) {
		free(line);
	}

	hash_virus_destroy(ht_viruses);
	hash_citizen_destroy(ht_citizens);
	hash_country_destroy(ht_countries);
	hash_filenames_destroy(ht_filenames);

	printf("Child: <%d>: Exiting ... \n", id);

	return 0;
}