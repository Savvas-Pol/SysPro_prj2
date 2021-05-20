#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#include "help_functions.h"

FILE* read_arguments(int argc, char** argv, int* bloomSize) { //reads arguments from command line
    int i;

    FILE* citizenRecordsFile;

    if (argc < 5) {
        printf("Wrong arguments!!!\n");
        return NULL;
    } else {
        for (i = 0; i < 4; i++) {
            if (!strcmp(argv[i], "-c")) {
                if (!(citizenRecordsFile = fopen(argv[i + 1], "r"))) {
                    printf("Error in opening %s\n", argv[i + 1]);
                    return NULL;
                }
            } else if (!strcmp(argv[i], "-b")) {
                *bloomSize = atoi(argv[i + 1]);
            }
        }
    }
    return citizenRecordsFile;
}

DIR* read_arguments_for_travel_monitor(int argc, char** argv, int* bloomSize, int *bufferSize, int *numMonitors, char ** inputDirectoryPath) {
    int i;

    DIR* inputDirectory;

    if (argc != 9) {
        printf("Wrong arguments!!!\n");
        return NULL;
    } else {
        for (i = 0; i < 9; i++) {
            if (!strcmp(argv[i], "-i")) {
                if (!(inputDirectory = opendir(argv[i + 1]))) {
                    printf("Error in opening %s\n", argv[i + 1]);
                    return NULL;
                } else {
                    *inputDirectoryPath = argv[i + 1];
                }
            } else if (!strcmp(argv[i], "-b")) {
                *bufferSize = atoi(argv[i + 1]);
            } else if (!strcmp(argv[i], "-s")) {
                *bloomSize = atoi(argv[i + 1]);
            } else if (!strcmp(argv[i], "-m")) {
                *numMonitors = atoi(argv[i + 1]);
            }
        }
    }
    return inputDirectory;
}

void read_arguments_for_vaccine_monitor(int argc, char** argv, int* bloomSize, int *bufferSize, int *numMonitors, int * id) {
    int i;

    if (argc != 3) {
        printf("Wrong arguments!!!\n");
    } else {
        for (i = 0; i < 3; i++) {
            if (!strcmp(argv[i], "-i")) {
                *id = atoi(argv[i + 1]);
            }
        }
    }
}

void fill_record(char* line, Record* temp) { //breaks line into tokens and creates a new record
    int i = 0, j;

    char* token;
    token = strtok(line, " \n"); //word by word

    while (token != NULL) {
        if (i == 0) {
            temp->citizenID = malloc((strlen(token)) + 1);
            strcpy(temp->citizenID, token);
        } else if (i == 1) {
            temp->firstName = malloc((strlen(token)) + 1);
            strcpy(temp->firstName, token);
        } else if (i == 2) {
            temp->lastName = malloc((strlen(token)) + 1);
            strcpy(temp->lastName, token);
        } else if (i == 3) {
            temp->country = malloc((strlen(token)) + 1);
            strcpy(temp->country, token);
        } else if (i == 4) {
            temp->age = atoi(token);
        } else if (i == 5) {
            temp->virusName = malloc((strlen(token)) + 1);
            strcpy(temp->virusName, token);
        } else if (i == 6) {
            if (!strcmp(token, "YES")) {
                j = 0;
                token = strtok(NULL, " \n");
                token = strtok(token, "-\n");

                temp->dateVaccinated = malloc(sizeof (Date));

                while (token != NULL) {
                    if (j == 0)
                        temp->dateVaccinated->day = atoi(token);
                    else if (j == 1)
                        temp->dateVaccinated->month = atoi(token);
                    else if (j == 2)
                        temp->dateVaccinated->year = atoi(token);
                    token = strtok(NULL, "-\n");
                    j++;
                }

            } else {
                temp->dateVaccinated = NULL;
            }
        }

        token = strtok(NULL, " \n");
        i++;
    }
}

bool find_conflict(Record record, Citizen* citizen) { //finds if any value is different from that of the citizen
    if (strcmp(record.citizenID, citizen->citizenID) != 0) {
        return true;
    }
    if (strcmp(record.firstName, citizen->firstName) != 0) {
        return true;
    }
    if (strcmp(record.lastName, citizen->lastName) != 0) {
        return true;
    }
    if (strcmp(record.country, citizen->country) != 0) {
        return true;
    }
    if (record.age != citizen->age) {
        return true;
    }
    return false;
}

void free_record(Record* temp) { //free
    free(temp->citizenID);
    free(temp->country);
    free(temp->firstName);
    free(temp->lastName);
    free(temp->virusName);

    if (temp->dateVaccinated != NULL) {
        free(temp->dateVaccinated);
    }
}

void send_info(int fd, char *info, int infolength, int bufferSize) {
    if (write(fd, (char*) &infolength, sizeof (infolength)) == -1) {
        if (errno == EINTR) {
            return;
        } else {
            perror("Error in write!!!\n");
            exit(1);
        }
    }

    int n = 0;

    while (n < infolength) {
        int m;

        if (infolength - n >= bufferSize) {
            m = write(fd, info, bufferSize);
            if (m == -1)
                perror("Error in write!!!\n");
        } else {
            m = write(fd, info, infolength - n);
            if (m == -1)
                perror("Error in write!!!\n");
        }

        n = n + m;
        info = info + m;
    }
}

int receive_info(int fd, char **pstart, int bufferSize) {
    int infolength;
    int n = 0;

    if ((n=read(fd, (char*) &infolength, sizeof (infolength))) == -1) {
        if (errno == EINTR) {
            return 0;
        } else {
            perror("Error in read!!!\n");
            exit(1);
        }
    }
    
    if (n == 0) {
        exit(1);
    }
    
    *pstart = malloc(infolength);

    n = 0;

    char * info = *pstart;

    while (n < infolength) {
        int m;

        if (infolength - n >= bufferSize) {
            m = read(fd, info, bufferSize);
            if (m == -1)
                perror("Error in read!!!\n");
        } else {
            m = read(fd, info, infolength - n);
            if (m == -1)
                perror("Error in read!!!\n");
        }

        n = n + m;
        info = info + m;
    }

    return infolength;
}

int receive_int(int fd, int buffersize) {
    char * info1 = NULL;
    int info_length1 = buffersize;
    info_length1 = receive_info(fd, &info1, info_length1);
    int result = *((int*) info1);
    free(info1);
    return result;
}