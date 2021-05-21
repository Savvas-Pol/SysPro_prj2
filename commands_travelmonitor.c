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

void travel_request(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize, char * citizenID, char* date, char* countryFrom, char* countryTo, char* virusName, int requestID, int* totalAccepted, int* totalRejected) {
    //printf("Called travel_request with: %s, %s, %s, %s, %s\n", citizenID, date, countryFrom, countryTo, virusName);

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
	   
        HashtableVirusNode* node = hash_virus_search(ht_viruses, virusName);
        skiplist_insert(node->not_vaccinated_persons, request, newDate, request->citizenID);
        (*totalRejected)++;

        destroy_request(request);
        return;
    }

    if (q == 2) {
        printf("REQUEST REJECTED - VIRUS NOT FOUND\n");
        return;
    }

    char name[10] = {0};
    sprintf(name, "%d", country->who);

    HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

    printf("Node for %s is %s \n", country->countryName, node->monitorName);

    char* command = malloc(strlen("travelRequest") + strlen(citizenID) + strlen(date) + strlen(countryFrom) + strlen(countryTo) + strlen(virusName) + 6);
    sprintf(command, "travelRequest %s %s %s %s %s", citizenID, date, countryFrom, countryTo, virusName); //reconstruct command

    printf("Sending command :%s to worker %d through pipe: %s via fd: %d \n", command, country->who, node->from_parent_to_child, node->fd_from_parent_to_child);

    char * info = command;
    int info_length = strlen(command) + 1;

    send_info(node->fd_from_parent_to_child, info, info_length, bufferSize);

    receive_info(node->fd_from_child_to_parent, &info, bufferSize);

    printf("%s\n", info);

    char newID[100];
    sprintf(newID, "%d", requestID);
    Citizen* request = create_request(newID, countryTo);

    Date* newDate = char_to_date(date);

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
    destroy_request(request);
    free(newDate);
    free(info);
    free(command);
}

void travel_stats(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, char* virusName, char* date1, char* date2) {
    //printf("Called travel_stats with: %s, %s, %s\n", virusName, date1, date2);

    HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
    int totalAccepted = 0, totalRejected = 0, j;

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
    //printf("Called travel_stats_country with: %s, %s, %s, %s\n", virusName, date1, date2, country);

    HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
    int totalAccepted = 0, totalRejected = 0, j;

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
    //printf("Called add_vaccination_records with: %s\n", countryName);

    HashtableCountryNode* country = hash_country_search(ht_countries, countryName);

    if (country == NULL) {
        printf("Country not found on parent\n");
        return;
    }

    char name[10] = {0};
    sprintf(name, "%d", country->who);

    HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

    printf("Node for %s is %s \n", country->countryName, node->monitorName);

    kill(node->pid, SIGUSR1);

    int fd = node->fd_from_child_to_parent;

    while (1) {
        char * info3 = NULL;
        receive_info(fd, &info3, bufferSize);

        char * buffer = info3;

        if (buffer[0] == '#') {
            free(buffer);
            break;
        }

        char * virusName = info3;

        HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName); //search if virus exists
        if (virusNode == NULL) {
            virusNode = hash_virus_insert(ht_viruses, virusName);
            virusNode->bloom = bloom_init(bloomSize);
            virusNode->vaccinated_persons = skiplist_init(SKIP_LIST_MAX_LEVEL);
            virusNode->not_vaccinated_persons = skiplist_init(SKIP_LIST_MAX_LEVEL);
        }

        char * bloomVector = NULL;
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
	
    char name[10] = {0};
    int tablelen;
    HashtableMonitorNode** table = hash_monitor_to_array(ht_monitors, &tablelen);

    char* command = malloc(strlen("searchVaccinationStatus") + strlen(citizenID) + 2);
    sprintf(command, "searchVaccinationStatus %s", citizenID); //reconstruct command

    printf("Sending command : %s to all monitors\n", command);

    char * info = command;
    int info_length = strlen(command) + 1;
    for(i = 0; i < tablelen; i++) {
    	send_info(table[i]->fd_from_parent_to_child, info, info_length, bufferSize);
    }

    // for(i = 0; i < numMonitors; i++) {
    // 	char name[10] = {0};
	   //  sprintf(name, "%d", i);

	   //  HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);
    // 	receive_info(node->fd_from_child_to_parent, &info, bufferSize);

	    
    // }
    free(command);
}




//void insert_citizen_record(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, int bloomSize, Record record, int flag) {
//
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, record.virusName);		//search if virus exists
//
//	bool vaccinating = record.dateVaccinated != NULL;	//if date is not NULL we have YES
//
//	if (virusNode != NULL) {
//		SkipListNode* sn1 = skiplist_search(virusNode->vaccinated_persons, record.citizenID);			//search citizen in vaccinated_persons skiplist
//		SkipListNode* sn2 = skiplist_search(virusNode->not_vaccinated_persons, record.citizenID);		//search citizen in not_vaccinated_persons skiplist
//
//		bool vaccinated = sn1 != NULL;
//		bool not_vaccinated = sn2 != NULL;
//
//		if (vaccinated || not_vaccinated) { 		//if already in a skiplist
//			if (vaccinating && vaccinated) {	//YES and already vaccinated
//				printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %d-%d-%d\n", sn1->citizen->citizenID, sn1->date->day, sn1->date->month, sn1->date->year);
//				return;
//			}
//
//			if (vaccinating && !vaccinated) {		//YES and not vaccinated
//				if (flag == 0) {		//command from keyboard
//					if (!find_conflict(record, sn2->citizen)) { 		//check if given record is correct
//						Citizen* c = sn2->citizen;
//
//						skiplist_delete(virusNode->not_vaccinated_persons, c->citizenID);
//						skiplist_insert(virusNode->vaccinated_persons, c, record.dateVaccinated, c->citizenID);
//						bloom_filter_insert(virusNode->bloom, c->citizenID, HASH_FUNCTIONS_K);
//						// printf("Vaccinating: %s %s %s \n", c->citizenID, c->firstName, c->lastName);
//					} else {
//						printf("Conflict on: %s %s %s\n", record.citizenID, record.firstName, record.lastName);
//					}
//				} else {		//while reading file
//					printf("Conflict on: %s %s %s\n", record.citizenID, record.firstName, record.lastName);
//				}
//				return;
//			}
//
//			if (!vaccinating && vaccinated) {	//NO and already vaccinated
//				printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %d-%d-%d\n", sn1->citizen->citizenID, sn1->date->day, sn1->date->month, sn1->date->year);
//				return;
//			}
//
//			if (!vaccinating && !vaccinated) {		//NO and not vaccinated
//				printf("Error: Already registered as not vaccinated\n");
//				return;
//			}
//			exit(1);
//		} else {		//if not in any skiplist
//			HashtableCitizenNode* cn = hash_citizen_search(ht_citizens, record.citizenID);
//
//			if (cn != NULL) {
//				if (vaccinating) {
//					skiplist_insert(virusNode->vaccinated_persons, cn->citizen, record.dateVaccinated, cn->citizen->citizenID);
//					bloom_filter_insert(virusNode->bloom, cn->citizen->citizenID, HASH_FUNCTIONS_K);
//				} else {
//					skiplist_insert(virusNode->not_vaccinated_persons, cn->citizen, record.dateVaccinated, cn->citizen->citizenID);
//				}
//				//printf("insert successful on existing citizen: %s %s %s %s\n", cn->citizen->citizenID, cn->citizen->firstName, cn->citizen->lastName, record.virusName);
//				return;
//			} else {
//				Citizen* c = citizen_create(record.citizenID, record.firstName, record.lastName, record.country, record.age);
//
//				hash_citizen_insert(ht_citizens, c);
//
//				HashtableCountryNode* cc = hash_country_search(ht_countries, c->country);
//				if (cc == NULL) {
//					hash_country_insert(ht_countries, c->country);
//				}
//
//				if (vaccinating) {
//					skiplist_insert(virusNode->vaccinated_persons, c, record.dateVaccinated, c->citizenID);
//					bloom_filter_insert(virusNode->bloom, c->citizenID, HASH_FUNCTIONS_K);
//				} else {
//					skiplist_insert(virusNode->not_vaccinated_persons, c, record.dateVaccinated, c->citizenID);
//				}
//
//				//printf("insert successful on new citizen: %s %s %s %s \n", c->citizenID, c->firstName, c->lastName, record.virusName);
//				return;
//			}
//		}
//	} else {		//if virus does not exist
//		//printf("new virus created: %s \n", record.virusName);
//
//		virusNode = hash_virus_insert(ht_viruses, record.virusName);
//		virusNode->bloom = bloom_init(bloomSize);
//		virusNode->vaccinated_persons = skiplist_init(SKIP_LIST_MAX_LEVEL);
//		virusNode->not_vaccinated_persons = skiplist_init(SKIP_LIST_MAX_LEVEL);
//	}
//
//
//	Citizen* c = citizen_create(record.citizenID, record.firstName, record.lastName, record.country, record.age);
//
//	hash_citizen_insert(ht_citizens, c);
//
//	HashtableCountryNode* cc = hash_country_search(ht_countries, c->country);
//	if (cc == NULL) {
//		hash_country_insert(ht_countries, c->country);
//	}
//
//	if (vaccinating) {
//		skiplist_insert(virusNode->vaccinated_persons, c, record.dateVaccinated, c->citizenID);
//		bloom_filter_insert(virusNode->bloom, c->citizenID, HASH_FUNCTIONS_K);
//	} else {
//		skiplist_insert(virusNode->not_vaccinated_persons, c, record.dateVaccinated, c->citizenID);
//	}
//
//	//printf("insert successful on new citizen: %s %s %s %s \n", c->citizenID, c->firstName, c->lastName, record.virusName);
//
//	return;
//}
//
//void vaccine_status_bloom(HashtableVirus* ht_viruses, char * citizenID, char * virusName) {
//
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//
//	if (virusNode != NULL) {
//		int q = bloom_filter_check(virusNode->bloom, citizenID, HASH_FUNCTIONS_K);
//
//		if (q == 0) {
//			printf("NOT VACCINATED \n");
//		} else {
//			printf("MAYBE \n");
//		}
//	} else {
//		printf("Virus: %s missing\n", virusName);
//	}
//}
//
//void vaccine_status_id_virus(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, char * citizenID, char * virusName) {
//
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//	HashtableCitizenNode* citizenNode = hash_citizen_search(ht_citizens, citizenID);
//
//	if (citizenNode != NULL) {
//		if (virusNode != NULL) {
//			SkipListNode* sn1 = skiplist_search(virusNode->vaccinated_persons, citizenID);
//
//			bool vaccinated = sn1 != NULL;
//
//			if (vaccinated) {
//				printf("VACCINATED ON %d-%d-%d \n", sn1->date->day, sn1->date->month, sn1->date->year);
//			} else {
//				printf("NOT VACCINATED \n");
//			}
//
//		} else {
//			printf("Virus: %s missing\n", virusName);
//		}
//	} else {
//		printf("Citizen: %s missing\n", citizenID);
//	}
//}
//
//void vaccine_status_id(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, char * citizenID) {
//
//	int i;
//	HashtableVirusNode* temp;
//	SkipListNode* citizen;
//	HashtableCitizenNode* citizenNode = hash_citizen_search(ht_citizens, citizenID);
//
//	if (citizenNode != NULL) {
//		for (i = 0; i < HASHTABLE_NODES; i++) {
//			temp = ht_viruses->nodes[i];
//			while (temp != NULL) {
//				citizen = skiplist_search(temp->vaccinated_persons, citizenID);
//				if (citizen != NULL) {
//					printf("%s YES %d-%d-%d \n", temp->virusName, citizen->date->day, citizen->date->month, citizen->date->year);
//				}
//
//				citizen = skiplist_search(temp->not_vaccinated_persons, citizenID);
//				if (citizen != NULL) {
//					printf("%s NO \n", temp->virusName);
//				}
//				temp = temp->next;
//			}
//		}
//	} else {
//		printf("Citizen: %s missing\n", citizenID);
//	}
//}
//
//void population_status_virus(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, char* virusName) {
//
//	int i;
//	HashtableCountryNode* temp;
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//
//	if (virusNode != NULL) {
//		for (i = 0; i < HASHTABLE_NODES; i++) {
//			temp = ht_countries->nodes[i];
//			while (temp != NULL) {
//				population_status_country(ht_viruses, ht_countries, temp->countryName, virusName);
//				temp = temp->next;
//			}
//		}
//	} else {
//		printf("Virus: %s missing\n", virusName);
//	}
//}
//
//void population_status_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName) {
//
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//	HashtableCountryNode* countryNode = hash_country_search(ht_countries, country);
//	int vaccinated_people = 0, total = 0;
//	double percentage;
//
//	if (countryNode != NULL) {
//		if (virusNode != NULL) {
//			SkipListNode* temp = virusNode->vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check vaccinated_persons skiplist for vaccinated people
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						vaccinated_people++;
//						total++;
//					}
//				}
//				temp = temp->next[0];
//			}
//
//			temp = virusNode->not_vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check not_vaccinated_persons skiplist to find total people for percentage
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						total++;
//					}
//				}
//				temp = temp->next[0];
//			}
//			percentage = (double)vaccinated_people / total * 100;
//			if (total != 0)
//				printf("%s %d %.2f%%\n", country, vaccinated_people, percentage);
//			else
//				printf("%s %d 0.00%%\n", country, vaccinated_people);
//		} else {
//			printf("Virus: %s missing\n", virusName);
//		}
//	} else {
//		printf("Country: %s missing\n", country);
//	}
//}
//
//void population_status_virus_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName, char* date1, char* date2) {
//	
//	int i;
//	HashtableCountryNode* temp;
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//
//	if (virusNode != NULL) {
//		for (i = 0; i < HASHTABLE_NODES; i++) {
//			temp = ht_countries->nodes[i];
//			while (temp != NULL) {
//				population_status_country_dates(ht_viruses, ht_countries, temp->countryName, virusName, date1, date2);
//				temp = temp->next;
//			}
//		}
//	} else {
//		printf("Virus: %s missing\n", virusName);
//	}
//}
//
//void population_status_country_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName, char* date1, char* date2) {
//	
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//	HashtableCountryNode* countryNode = hash_country_search(ht_countries, country);
//	int vaccinated_people = 0, total = 0, j;
//	double percentage;
//	Date* date_from = calloc(1, sizeof (Date));
//	Date* date_to = calloc(1, sizeof (Date));
//
//	char tempdate1[11];		//max possible digits of a date
//	char tempdate2[11];
//	
//	strcpy(tempdate1, date1);
//	char* token = strtok(tempdate1, "-");
//	j = 0;
//	while (token != NULL) {
//		if (j == 0)
//			date_from->day = atoi(token);
//		else if (j == 1)
//			date_from->month = atoi(token);
//		else if (j == 2)
//			date_from->year = atoi(token);
//		token = strtok(NULL, "-\n");
//		j++;
//	}
//	
//	strcpy(tempdate2, date2);
//	token = strtok(tempdate2, "-");
//	j = 0;
//	while (token != NULL) {
//		if (j == 0)
//			date_to->day = atoi(token);
//		else if (j == 1)
//			date_to->month = atoi(token);
//		else if (j == 2)
//			date_to->year = atoi(token);
//		token = strtok(NULL, "-\n");
//		j++;
//	}
//
//	if (countryNode != NULL) {
//		if (virusNode != NULL) {
//			SkipListNode* temp = virusNode->vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check vaccinated_persons skiplist for vaccinated people
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						if((date_compare(temp->date, date_from)) == 1 && (date_compare(temp->date, date_to)) == -1) {
//							vaccinated_people++;                        
//						}
//						total++;
//					}
//				}
//				temp = temp->next[0];
//			}
//
//			temp = virusNode->not_vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check not_vaccinated_persons skiplist to find total people for percentage
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						total++;
//					}
//				}
//				temp = temp->next[0];
//			}
//			percentage = (double)vaccinated_people / total * 100;
//			if (total != 0)
//				printf("%s %d %.2f%%\n", country, vaccinated_people, percentage);
//			else
//				printf("%s %d 0.00%%\n", country, vaccinated_people);
//		} else {
//			printf("Virus: %s missing\n", virusName);
//		}
//	} else {
//		printf("Country: %s missing\n", country);
//	}
//	
//	free(date_from);
//	free(date_to);
//}
//
//void pop_status_by_age_virus(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName) {
//
//	int i;
//	HashtableCountryNode* temp;
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//
//	if (virusNode != NULL) {
//		for (i = 0; i < HASHTABLE_NODES; i++) {
//			temp = ht_countries->nodes[i];
//			while (temp != NULL) {
//				pop_status_by_age_country(ht_viruses, ht_countries, temp->countryName, virusName);
//				temp = temp->next;
//			}
//		}
//	} else {
//		printf("Virus: %s missing\n", virusName);
//	}
//}
//
//void pop_status_by_age_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName) {
//
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//	HashtableCountryNode* countryNode = hash_country_search(ht_countries, country);
//	int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, total1 = 0, total2 = 0, total3 = 0, total4 = 0;
//	double percentage1, percentage2, percentage3, percentage4;
//
//	if (countryNode != NULL) {
//		if (virusNode != NULL) {
//			SkipListNode* temp = virusNode->vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check vaccinated_persons skiplist for vaccinated people
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						if (temp->citizen->age > 0 && temp->citizen->age <= 20) {
//							sum1++;
//							total1++;
//						}
//						else if (temp->citizen->age > 20 && temp->citizen->age <= 40) {
//							sum2++;
//							total2++;
//						}
//						else if (temp->citizen->age > 40 && temp->citizen->age <= 60) {
//							sum3++;
//							total3++;
//						}
//						else {
//							sum4++;
//							total4++;
//						}
//					}
//				}
//				temp = temp->next[0];
//			}
//
//			temp = virusNode->not_vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check not_vaccinated_persons skiplist to find total people for percentage
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						if (temp->citizen->age > 0 && temp->citizen->age <= 20) {
//							total1++;
//						}
//						else if (temp->citizen->age > 20 && temp->citizen->age <= 40) {
//							total2++;
//						}
//						else if (temp->citizen->age > 40 && temp->citizen->age <= 60) {
//							total3++;
//						}
//						else {
//							total4++;
//						}
//					}
//				}
//				temp = temp->next[0];
//			}
//			percentage1 = (double)sum1 / total1 * 100;
//			percentage2 = (double)sum2 / total2 * 100;
//			percentage3 = (double)sum3 / total3 * 100;
//			percentage4 = (double)sum4 / total4 * 100;
//			printf("%s\n", country);
//			if (total1 != 0)
//				printf("0-20 %d %.2f%%\n", sum1, percentage1);
//			else
//				printf("0-20 %d 0.00%%\n", sum1);
//			if (total2 != 0)
//				printf("20-40 %d %.2f%%\n", sum2, percentage2);
//			else
//				printf("20-40 %d 0.00%%\n", sum2);
//			if (total3 != 0)
//				printf("40-60 %d %.2f%%\n", sum3, percentage3);
//			else
//				printf("40-60 %d 0.00%%\n", sum3);
//			if (total4 != 0)
//				printf("60+ %d %.2f%%\n", sum4, percentage4);
//			else
//				printf("60+ %d 0.00%%\n", sum4);
//		} else {
//			printf("Virus: %s missing\n", virusName);
//		}
//	} else {
//		printf("Country: %s missing\n", country);
//	}
//
//}
//
//void pop_status_by_age_virus_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName, char* date1, char* date2) {
//	
//	int i;
//	HashtableCountryNode* temp;
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//
//	if (virusNode != NULL) {
//		for (i = 0; i < HASHTABLE_NODES; i++) {
//			temp = ht_countries->nodes[i];
//			while (temp != NULL) {
//				pop_status_by_age_country_dates(ht_viruses, ht_countries, temp->countryName, virusName, date1, date2);
//				temp = temp->next;
//			}
//		}
//	} else {
//		printf("Virus: %s missing\n", virusName);
//	}
//}
//
//void pop_status_by_age_country_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName, char* date1, char* date2) {
//	
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//	HashtableCountryNode* countryNode = hash_country_search(ht_countries, country);
//	int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, total1 = 0, total2 = 0, total3 = 0, total4 = 0, j;
//	double percentage1, percentage2, percentage3, percentage4;
//
//	Date* date_from = calloc(1, sizeof (Date));
//	Date* date_to = calloc(1, sizeof (Date));
//
//	char tempdate1[11];
//	char tempdate2[11];
//	
//	strcpy(tempdate1, date1);
//	char* token = strtok(tempdate1, "-");
//	j = 0;
//	while (token != NULL) {
//		if (j == 0)
//			date_from->day = atoi(token);
//		else if (j == 1)
//			date_from->month = atoi(token);
//		else if (j == 2)
//			date_from->year = atoi(token);
//		token = strtok(NULL, "-\n");
//		j++;
//	}
//
//	strcpy(tempdate2, date2);
//
//	token = strtok(tempdate2, "-");
//	j = 0;
//	while (token != NULL) {
//		if (j == 0)
//			date_to->day = atoi(token);
//		else if (j == 1)
//			date_to->month = atoi(token);
//		else if (j == 2)
//			date_to->year = atoi(token);
//		token = strtok(NULL, "-\n");
//		j++;
//	}
//
//	if (countryNode != NULL) {
//		if (virusNode != NULL) {
//			SkipListNode* temp = virusNode->vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check vaccinated_persons skiplist for vaccinated people
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						if ((date_compare(temp->date, date_from)) == 1 && (date_compare(temp->date, date_to)) == -1) {
//							if (temp->citizen->age > 0 && temp->citizen->age <= 20) {
//								sum1++;
//								total1++;
//							}
//							else if (temp->citizen->age > 20 && temp->citizen->age <= 40) {
//								sum2++;
//								total2++;
//							}
//							else if (temp->citizen->age > 40 && temp->citizen->age <= 60) {
//								sum3++;
//								total3++;
//							}
//							else {
//								sum4++;
//								total4++;
//							}
//						}
//					}
//				}
//				temp = temp->next[0];
//			}
//
//			temp = virusNode->not_vaccinated_persons->head->next[0];
//			while (temp != NULL) {  //check not_vaccinated_persons skiplist to find total people for percentage
//				if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) {
//					if (!strcmp(temp->citizen->country, country)) {
//						if (temp->citizen->age > 0 && temp->citizen->age <= 20) {
//							total1++;
//						}
//						else if (temp->citizen->age > 20 && temp->citizen->age <= 40) {
//							total2++;
//						}
//						else if (temp->citizen->age > 40 && temp->citizen->age <= 60) {
//							total3++;
//						}
//						else {
//							total4++;
//						}
//					}
//				}
//				temp = temp->next[0];
//			}
//			percentage1 = (double)sum1 / total1 * 100;
//			percentage2 = (double)sum2 / total2 * 100;
//			percentage3 = (double)sum3 / total3 * 100;
//			percentage4 = (double)sum4 / total4 * 100;
//			printf("%s\n", country);
//			if (total1 != 0)
//				printf("0-20 %d %.2f%%\n", sum1, percentage1);
//			else
//				printf("0-20 %d 0.00%%\n", sum1);
//			if (total2 != 0)
//				printf("20-40 %d %.2f%%\n", sum2, percentage2);
//			else
//				printf("20-40 %d 0.00%%\n", sum2);
//			if (total3 != 0)
//				printf("40-60 %d %.2f%%\n", sum3, percentage3);
//			else
//				printf("40-60 %d 0.00%%\n", sum3);
//			if (total4 != 0)
//				printf("60+ %d %.2f%%\n", sum4, percentage4);
//			else
//				printf("60+ %d 0.00%%\n", sum4);
//		} else {
//			printf("Virus: %s missing\n", virusName);
//		}
//	} else {
//		printf("Country: %s missing\n", country);
//	}
//	free(date_from);
//	free(date_to);
//}
//
//void vaccinate_now(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, int bloomSize, char * citizenID, char * firstName, char * lastName, char * country, char * age, char * virusName) {
//
//	Date* d = get_current_date();
//
//	Record record = { 0 };
//	
//	record.citizenID = malloc((strlen(citizenID)) + 1);
//	record.firstName = malloc((strlen(firstName)) + 1);
//	record.lastName = malloc((strlen(lastName)) + 1);
//	record.country = malloc((strlen(country)) + 1);
//	record.virusName = malloc((strlen(virusName)) + 1);
//
//	strcpy(record.citizenID, citizenID);
//	strcpy(record.firstName, firstName);
//	strcpy(record.lastName, lastName);
//	strcpy(record.country, country);
//	record.age = atoi(age);
//	strcpy(record.virusName, virusName);
//
//	record.dateVaccinated = d;
//
//	insert_citizen_record(ht_viruses, ht_citizens, ht_countries, bloomSize, record, 0);
//
//	// free(d);
//	
//	free_record(&record);
//}
//
//void list_nonVaccinated_Persons(HashtableVirus* ht_viruses, char* virusName) {
//
//	HashtableVirusNode* virusNode = hash_virus_search(ht_viruses, virusName);
//
//	if (virusNode != NULL) {
//		SkipListNode* temp = virusNode->not_vaccinated_persons->head->next[0]; //first node after head
//
//		while (temp != NULL) {
//			if (strcmp(temp->citizen->citizenID, "ZZZZZ") != 0) //don't print skip list tail node
//				printf("%s %s %s %s %d\n", temp->citizen->citizenID, temp->citizen->firstName, temp->citizen->lastName, temp->citizen->country, temp->citizen->age);
//			temp = temp->next[0];
//		}
//	} else {
//		printf("Virus: %s missing\n", virusName);
//	}
//}