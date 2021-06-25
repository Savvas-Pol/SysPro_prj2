#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>

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

int write_all(int fd, void*buff, size_t size) {      //function from http://cgi.di.uoa.gr/~antoulas/k24/lectures/l11.pdf
	int sent, n;
	for (sent = 0; sent < size; sent += n) {
		if ((n = write(fd, buff + sent, size - sent)) == -1) {
			return-1;
		}
	}
	return sent;
}

int read_all(int fd, void*buff, size_t size) {      //function from http://cgi.di.uoa.gr/~antoulas/k24/lectures/l11.pdf
	int sent, n;
	for (sent = 0; sent < size; sent += n) {
		if ((n = read(fd, buff + sent, size - sent)) == -1) {
			return-1;
		}
	}
	return sent;
}

void send_info(int fd, char* info, int infolength, int bufferSize) {
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
			m = write_all(fd, info, bufferSize);
			if (m == -1)
				perror("Error in write!!!\n");
		} else {
			m = write_all(fd, info, infolength - n);
			if (m == -1)
				perror("Error in write!!!\n");
		}

		n = n + m; //bytes written
		info = info + m; //move pointer by m
	}
}

int receive_info(int fd, char** pstart, int bufferSize) {
	int infolength;
	int n = 0;

	if ((n = read(fd, (char*) &infolength, sizeof (infolength))) == -1) {
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

	char* info = *pstart;

	while (n < infolength) {
		int m;

		if (infolength - n >= bufferSize) {
			m = read_all(fd, info, bufferSize);
			if (m == -1)
				perror("Error in read!!!\n");
		} else {
			m = read_all(fd, info, infolength - n);
			if (m == -1)
				perror("Error in read!!!\n");
		}

		n = n + m;
		info = info + m;
	}

	return infolength;
}

int receive_int(int fd, int buffersize) {

	char* info1 = NULL;
	int info_length1 = buffersize;

	info_length1 = receive_info(fd, &info1, info_length1);
	
	int result = *((int*) info1);
	free(info1);

	return result;
}

void respawn_child(HashtableMonitor* ht_monitors, HashtableVirus* ht_viruses, int bloomSize, int bufferSize, char *inputDirectoryPath, int argc, char** argv, HashtableCountryNode** table, int tablelen) {
	pid_t p;

	while ((p=(waitpid(-1, NULL, WNOHANG))) > 0) {
		printf("Child with PID %d died \n", p);

		HashtableMonitorNode* node = hash_monitor_search_pid(ht_monitors, p);
		
		close(node->fd_from_child_to_parent);
		close(node->fd_from_parent_to_child);

		unlink(node->from_child_to_parent);
		unlink(node->from_parent_to_child);
		
		if (mkfifo(node->from_child_to_parent, 0600) == -1 && errno == EEXIST) {		//create named pipes
			unlink(node->from_child_to_parent);
			mkfifo(node->from_child_to_parent, 0600);            
		}
		
		if (mkfifo(node->from_parent_to_child, 0600) == -1 && errno == EEXIST) {
			unlink(node->from_parent_to_child);
			mkfifo(node->from_parent_to_child, 0600);            
		}

		pid_t newpid = fork();
		if (newpid > 0) { //parent
			node->pid = newpid;		//update pid

			if ((node->fd_from_parent_to_child = open(node->from_parent_to_child, O_WRONLY)) < 0) {
				perror("travelMonitor: can't open read fifo");
				exit(1);
			}

			if ((node->fd_from_child_to_parent = open(node->from_child_to_parent, O_RDONLY)) < 0) {
				perror("travelMonitor: can't open write fifo");
				exit(1);
			}

			char* info1 = (char *) &bloomSize;
			int info_length1 = sizeof (bloomSize);

			send_info(node->fd_from_parent_to_child, info1, info_length1, info_length1); //first message is bloomSize

			char* info2 = (char *) &bufferSize;
			int info_length2 = sizeof (bufferSize);

			send_info(node->fd_from_parent_to_child, info2, info_length2, info_length2); //second message is bufferSize

			char* info3 = inputDirectoryPath;
			int info_length3 = strlen(inputDirectoryPath) + 1;

			send_info(node->fd_from_parent_to_child, info3, info_length3, info_length3);
			// receive_bloom_filter(ht_monitors, ht_viruses, numMonitors, bloomSize, bufferSize);

			for (int j = 0; j < tablelen; j++) {		//send countries to new child
				char * country = table[j]->countryName;
				
				char name[100];
				sprintf(name, "%d", table[j]->who);
				if(strcmp(name, node->monitorName) == 0) {
					HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

					int fd = node->fd_from_parent_to_child;

					printf("Sending country :%s to worker %d through pipe: %s via fd: %d \n", country, table[j]->who, node->from_parent_to_child, fd);

					char * info1 = (char *) country;
					int info_length1 = strlen(country) + 1;

					send_info(fd, info1, info_length1, bufferSize);
				} 
			}
			HashtableMonitorNode* node = hash_monitor_search_pid(ht_monitors, newpid);
			int fd = node->fd_from_parent_to_child;
			char buffer[2] = "#";
			strcpy(buffer, "#");
			char* info4 = (char*) buffer;
			int info_length4 = strlen(buffer) + 1;

			send_info(fd, info4, info_length4, bufferSize);
			
			while (1) {
				char * info3 = NULL;
				receive_info(fd, &info3, bufferSize);

				char * buffer = info3;

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
				
				for (int k=0;k<bloomSize;k++) {
					virusNode->bloom->vector[k] |= bloomVector[k];
				}

				free(bloomVector);            
				free(buffer);            
			}

		} else if (newpid == 0) { //child
			argc = 3;
			argv = malloc(sizeof (char*)*4);
			argv[0] = "vaccineMonitor";
			argv[1] = "-i";
			argv[2] = node->monitorName;
			argv[3] = NULL;

			execvp("./vaccineMonitor", argv);
		}
	}
}