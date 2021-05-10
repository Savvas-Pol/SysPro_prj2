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
#include "BF.h"
#include "record.h"
#include "commands.h"
#include "constants.h"

// "${OUTPUT_PATH}" -m 4 -b 2000 -s 1000 -i id

int writelog = 0;

void catchinterrupt2(int signo) {
    printf("\nCatching: signo=%d\n", signo);
    printf("Catching: returning\n");
    
    writelog = 1;
}

int vaccine_monitor_main(int argc, char** argv) {
    srand(time(0));

    /*  ---     DECLARATIONS    --- */

    int bloomSize, bufferSize, numMonitors, j, id;
    char* token;

    char* line = NULL;
    size_t len = 0;

    FILE* citizenRecordsFile;
    DIR* inputDirectory;
    struct dirent *direntp;

    char from_child_to_parent[1000];
    char from_parent_to_child[1000];

    static struct sigaction act;

    /*      ---------------     */

    read_arguments_for_vaccine_monitor(argc, argv, &bloomSize, &bufferSize, &numMonitors, &id);

    act.sa_handler = catchinterrupt2;
    sigfillset(&(act.sa_mask));

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

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

    printf("Child: <%d>: waiting for countries (bloom:%d, buffer:%d )... \n", id, bloomSize, bufferSize);

    HashtableVirus* ht_viruses = hash_virus_create(HASHTABLE_NODES); //create HashTable for viruses
    HashtableCitizen* ht_citizens = hash_citizen_create(HASHTABLE_NODES); //create HashTable for citizens
    HashtableCountry* ht_countries = hash_country_create(HASHTABLE_NODES); //create HashTable for countries

    while (1) {
        char * info3 = NULL;
        int info_length3;

        info_length3 = receive_info(readfd, &info3, bufferSize);

        char * buffer = info3;

        if (buffer[0] == '#') {
            break;
        }

        printf("Child: <%d>: country received: %s ... \n", id, buffer);

        HashtableCountryNode* cc = hash_country_search(ht_countries, buffer);
        if (cc == NULL) {
            hash_country_insert(ht_countries, buffer);
        }
    }

    printf("Child: <%d>: building structures ... \n", id);

    printf("Child: <%d>: Exiting ... \n", id);

    // if (!(inputDirectory = opendir(argv[2]))) {
    //     printf("Error in opening %s\n", argv[2]);
    //     return -1;
    // }
    // while ((direntp = readdir(inputDirectory)) != NULL) {
    //     if (direntp->d_name[0] != '.') {
    //         printf("inode %d of the entry %s \n", (int) direntp->d_ino, direntp->d_name);

    //         HashtableCountryNode* cc = hash_country_search(ht_countries, direntp->d_name);
    //         if (cc == NULL) {
    //             hash_country_insert(ht_countries, direntp->d_name);
    //         }
    //     }
    // }

    //closedir(inputDirectory);

    return 0;

    int r;
    while ((r = getline(&line, &len, citizenRecordsFile)) != -1) { //read file line by line
        Record record;

        fill_record(line, &record); //create a temp record
        insert_citizen_record(ht_viruses, ht_citizens, ht_countries, bloomSize, record, 1); //flag=1 means from file
        free_record(&record); //free temp record
    }

    while (1) { //commands from user
        printf("\nGive command: ");
        
        if (writelog == 1) {
            writelog = 0;
            // writelog
        }

        // read from pipe instead of stdin
        getline(&line, &len, stdin);
        token = strtok(line, " \n");
        
        if (writelog == 1) {
            writelog = 0;
            // writelog
        }

        if (token != NULL) {

            if (!strcmp(token, "/vaccineStatusBloom")) {
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
                    vaccine_status_id(ht_viruses, ht_citizens, tokens[0]);
                } else if (tokens[0] != NULL && tokens[1] != NULL && tokens[2] == NULL) {
                    vaccine_status_id_virus(ht_viruses, ht_citizens, tokens[0], tokens[1]);
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

    fclose(citizenRecordsFile);

    hash_virus_destroy(ht_viruses);
    hash_citizen_destroy(ht_citizens);
    hash_country_destroy(ht_countries);

    return 0;
}