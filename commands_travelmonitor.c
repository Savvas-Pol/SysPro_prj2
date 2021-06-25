#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "commands_travelmonitor.h"
#include "skiplist.h"
#include "constants.h"
#include "date.h"
#include "hashtable_citizen.h"
#include "hashtable_virus.h"
#include "hashtable_country.h"
#include "hashtable_monitor.h"
#include "help_functions.h"
#include "commands_vaccinemonitor.h"
#include "hashtable_request.h"

void travel_request(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, HashtableRequest* ht_requests, int bloomSize, int bufferSize, char * citizenID, char* date, char* countryFrom, char* countryTo, char* virusName, int requestID, int* totalAccepted, int* totalRejected) {

	HashtableCountryNode* country = hash_country_search(ht_countries, countryFrom);

	if (country == NULL) {
		printf("REQUEST REJECTED - COUNTRY NOT FOUND\n");
		return;
	}

	int q = vaccine_status_bloom(ht_viruses, citizenID, virusName);

	if (q == 0) {
		printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
		char newID[100];
		sprintf(newID, "%d", requestID);
		Citizen* request = create_request(newID, countryTo);
		Date* newDate = char_to_date(date);
		hash_request_insert(ht_requests, request, newDate);

		HashtableVirusNode* node = hash_virus_search(ht_viruses, virusName);
		skiplist_insert(node->not_vaccinated_persons, request, newDate, request->citizenID);
		(*totalRejected)++;
		return;
	}

	if (q == 2) {
		printf("REQUEST REJECTED - VIRUS NOT FOUND\n");
		return;
	}

	char name[10] = {0};
	sprintf(name, "%d", country->who);

	HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

	//printf("Node for %s is %s \n", country->countryName, node->monitorName);

	char* command = malloc(strlen("travelRequest") + strlen(citizenID) + strlen(date) + strlen(countryFrom) + strlen(countryTo) + strlen(virusName) + 6);
	sprintf(command, "travelRequest %s %s %s %s %s", citizenID, date, countryFrom, countryTo, virusName); //reconstruct command

	//printf("Sending command :%s to worker %d through pipe: %s via fd: %d \n", command, country->who, node->from_parent_to_child, node->fd_from_parent_to_child);

	char* info = command;
	int info_length = strlen(command) + 1;

	send_info(node->fd_from_parent_to_child, info, info_length, bufferSize);

	receive_info(node->fd_from_child_to_parent, &info, bufferSize);

	printf("%s\n", info);

	char newID[100];
	sprintf(newID, "%d", requestID);
	Citizen* request = create_request(newID, countryTo);
	Date* newDate = char_to_date(date);
	hash_request_insert(ht_requests, request, newDate);
	
	if (!strcmp(info, "REQUEST ACCEPTED - HAPPY TRAVELS")) {
		HashtableVirusNode* node = hash_virus_search(ht_viruses, virusName);
		skiplist_insert(node->vaccinated_persons, request, newDate, request->citizenID);
		(*totalAccepted)++;
		//printf("ACCEPTED - Inserted in skiplist successfully - ID: %s on %d-%d-%d\n", request->citizenID, newDate->day, newDate->month, newDate->year);
	} else {
		HashtableVirusNode* node = hash_virus_search(ht_viruses, virusName);
		skiplist_insert(node->not_vaccinated_persons, request, newDate, request->citizenID);
		(*totalRejected)++;
		//printf("REJECTED - Inserted in skiplist successfully - ID: %s on %d-%d-%d\n", request->citizenID, newDate->day, newDate->month, newDate->year);
	}
	free(newDate);
	free(info);
	free(command);
}

void travel_stats(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, char* virusName, char* date1, char* date2) {

	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
	int totalAccepted = 0, totalRejected = 0;

	Date* date_from = char_to_date(date1);
	Date* date_to = char_to_date(date2);

	if (virusNode != NULL) {
		SkipListNode* accepted = virusNode->vaccinated_persons->head->next[0];
		SkipListNode* rejected = virusNode->not_vaccinated_persons->head->next[0];

		if (accepted != NULL) {
			while (strcmp(accepted->citizen->citizenID, "ZZZZZ") != 0) { //SEGMENTATION
				if ((date_compare(accepted->date, date_from)) == 1 && (date_compare(accepted->date, date_to)) == -1) {
					totalAccepted++;
				}
				accepted = accepted->next[0];
			}
		}

		if (rejected != NULL) {
			while (strcmp(rejected->citizen->citizenID, "ZZZZZ") != 0) {
				if ((date_compare(rejected->date, date_from)) == 1 && (date_compare(rejected->date, date_to)) == -1) {
					totalRejected++;
				}
				rejected = rejected->next[0];
			}
		}

		printf("TOTAL REQUESTS %d\n", totalAccepted + totalRejected);
		printf("ACCEPTED %d\n", totalAccepted);
		printf("REJECTED %d\n", totalRejected);
	} else {
		printf("VIRUS %s NOT FOUND\n", virusName);
	}

	free(date_from);
	free(date_to);
}

void travel_stats_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, char* virusName, char* date1, char* date2, char* country) {

	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
	int totalAccepted = 0, totalRejected = 0;

	Date* date_from = char_to_date(date1);
	Date* date_to = char_to_date(date2);

	if (virusNode != NULL) {
		SkipListNode* accepted = virusNode->vaccinated_persons->head->next[0];
		SkipListNode* rejected = virusNode->not_vaccinated_persons->head->next[0];

		if (accepted != NULL) {
			while (strcmp(accepted->citizen->citizenID, "ZZZZZ") != 0) { //SEGMENTATION
				if ((date_compare(accepted->date, date_from)) == 1 && (date_compare(accepted->date, date_to)) == -1) {
					if (strcmp(accepted->citizen->country, country) == 0) {
						totalAccepted++;
					}
				}
				accepted = accepted->next[0];
			}
		}

		if (rejected != NULL) {
			while (strcmp(rejected->citizen->citizenID, "ZZZZZ") != 0) {
				if ((date_compare(rejected->date, date_from)) == 1 && (date_compare(rejected->date, date_to)) == -1) {
					if (strcmp(accepted->citizen->country, country) == 0) {
						totalRejected++;
					}
				}
				rejected = rejected->next[0];
			}
		}

		printf("TOTAL REQUESTS %d\n", totalAccepted + totalRejected);
		printf("ACCEPTED %d\n", totalAccepted);
		printf("REJECTED %d\n", totalRejected);
	} else {
		printf("VIRUS %s NOT FOUND\n", virusName);
	}

	free(date_from);
	free(date_to);
}

void add_vaccination_records(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize, char* countryName) {

	HashtableCountryNode* country = hash_country_search(ht_countries, countryName);

	if (country == NULL) {
		printf("COUNTRY NOT FOUND\n");
		return;
	}

	char name[10] = {0};
	sprintf(name, "%d", country->who);

	HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

	//printf("Node for %s is %s \n", country->countryName, node->monitorName);

	kill(node->pid, SIGUSR1);

	int fd = node->fd_from_child_to_parent;

	while (1) {
		char * info3 = NULL;
		receive_info(fd, &info3, bufferSize);

		char* buffer = info3;

		if (buffer[0] == '#') {
			free(buffer);
			break;
		}

		char* virusName = info3;

		HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName); //search if virus exists
		if (virusNode == NULL) {
			virusNode = hash_virus_insert(ht_viruses, virusName);
			virusNode->bloom = bloom_init(bloomSize);
			virusNode->vaccinated_persons = skiplist_init(SKIP_LIST_MAX_LEVEL);
			virusNode->not_vaccinated_persons = skiplist_init(SKIP_LIST_MAX_LEVEL);
		}

		char* bloomVector = NULL;
		receive_info(fd, &bloomVector, bloomSize);

		for (int k = 0; k < bloomSize; k++) {
			virusNode->bloom->vector[k] |= bloomVector[k];
		}

		free(bloomVector);
		free(buffer);
	}
}

void search_vaccination_status(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize, int numMonitors, char* citizenID) {

	int i;
	int tablelen;
	HashtableMonitorNode** table = hash_monitor_to_array(ht_monitors, &tablelen);

	char* command = malloc(strlen("searchVaccinationStatus") + strlen(citizenID) + 2);
	sprintf(command, "searchVaccinationStatus %s", citizenID); //reconstruct command

	//printf("Sending command : %s to all monitors\n", command);

	char* info = command;
	int info_length = strlen(command) + 1;
	for (i = 0; i < tablelen; i++) {
		send_info(table[i]->fd_from_parent_to_child, info, info_length, bufferSize);
	}

	// SELECT
	for (i = 0; i < tablelen; i++) {
		while (1) {
			receive_info(table[i]->fd_from_child_to_parent, &info, bufferSize);
			
			if (strcmp(info, "#") == 0) {
				break;
			} else {
				printf("%s\n", info);
			}
		}
	}
	free(table);
	free(command);
}
