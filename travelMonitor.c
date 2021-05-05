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
#include "commands.h"
#include "constants.h"
#include "commands_travelmonitor.h"

int quit = 0;

// "${OUTPUT_PATH}" -m 4 -b 2000 -s 1000 -i data_small
int vaccine_monitor_main(int argc, char** argv);

void catchinterrupt(int signo) {
    printf("\nCatching: signo=%d\n", signo);
    printf("Catching: returning\n");
    
    quit =1 ;
}

int main(int argc, char** argv) {

    /*  ---     DECLARATIONS    --- */

    int bloomSize, bufferSize, numMonitors, j;
    char* token;

    char* line = NULL;

    DIR* inputDirectory;
    struct dirent *direntp;

    /*      ---------------     */

    static struct sigaction act;

    if ((inputDirectory = read_arguments_for_travel_monitor(argc, argv, &bloomSize, &bufferSize, &numMonitors)) == NULL) { //read arguments from command line
        return -1;
    } else {
        act.sa_handler = catchinterrupt;
        sigfillset(&(act.sa_mask));

        sigaction(SIGINT, &act, NULL);
        sigaction(SIGQUIT, &act, NULL);
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

            char * info1 = (char *) &bloomSize;
            int info_length1 = sizeof (bloomSize);

            send_info(node->fd_from_parent_to_child, info1, info_length1, info_length1);


            char * info2 = (char *) &bufferSize;
            int info_length2 = sizeof (bufferSize);

            send_info(node->fd_from_parent_to_child, info2, info_length2, info_length2);

            printf("info_length1=%d, info_length2=%d\n", info_length1, info_length2);
        } else if (pid == 0) {
            argc = 3;
            argv = malloc(sizeof (char*)*4);
            argv[0] = "vaccineMonitor";
            argv[1] = "-i";
            argv[2] = name;
            argv[3] = NULL;
            return vaccine_monitor_main(argc, argv);
        }
    }

    int tablelen;

    HashtableCountryNode** table = hash_country_to_array(ht_countries, &tablelen);

    for (j = 0; j < tablelen; j++) {
        char * country = table[j]->countryName;
        printf(" (%d) country :%s \n", j + 1, country);
    }

    printf("----------------------------------\n");


    for (j = 0; j < tablelen; j++) {
        char * country = table[j]->countryName;
        table[j]->who = j % numMonitors;

        char name[100];
        sprintf(name, "%d", table[j]->who);

        HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

        int fd = node->fd_from_parent_to_child;

        printf("Sending country :%s to worker %d through pipe: %s via fd: %d \n", country, table[j]->who, node->from_parent_to_child, fd);

        char * info1 = (char *) country;
        int info_length1 = strlen(country) + 1;

        send_info(fd, info1, info_length1, bufferSize);
    }


    for (j = 0; j < numMonitors; j++) {
        char name[100];
        char buffer[2] = "#";
        sprintf(name, "%d", j);

        HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

        int fd = node->fd_from_parent_to_child;

        strcpy(buffer, "#");

        char * info1 = (char *) buffer;
        int info_length1 = strlen(buffer) + 1;

        send_info(fd, info1, info_length1, bufferSize);
    }

    while (quit != 1) { //commands from user
        size_t len = 0;
        printf("\nGive command: ");
        if (getline(&line, &len, stdin) == 0 || quit == 1) {
            break;
        }
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
            } else if (!strcmp(token, "/pid") || !strcmp(token, "pid")) {
                printf("Parent pid is %d \n", getpid());
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

    return 0;
}