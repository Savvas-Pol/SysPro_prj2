#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "hashtable_monitor.h"
#include "help_functions.h"

HashtableMonitor* hash_monitor_create(int hashNodes) {

	int i;

	HashtableMonitor* ht = malloc(sizeof (HashtableMonitor));
	ht->hash_nodes = hashNodes;

	ht->nodes = (HashtableMonitorNode**) malloc(hashNodes * sizeof (HashtableMonitorNode*)); //create hashtable for countries
	for (i = 0; i < hashNodes; i++) {
		ht->nodes[i] = NULL;
	}

	return ht;
}

void hash_monitor_destroy(HashtableMonitor* ht) {

	int i;

	for (i = 0; i < ht->hash_nodes; i++) {
		HashtableMonitorNode* temp = ht->nodes[i];

		while (temp != NULL) {
			ht->nodes[i] = temp->next;

			free(temp->monitorName);
			free(temp);

			temp = ht->nodes[i];
		}
	}

	free(ht->nodes);
	free(ht);
}

HashtableMonitorNode* hash_monitor_search(HashtableMonitor* ht, char* monitorName) {

	int pos = hash_function((unsigned char*) monitorName, ht->hash_nodes);

	HashtableMonitorNode* temp = ht->nodes[pos];

	while (temp != NULL) {
		if (!strcmp(temp->monitorName, monitorName))
			return temp;

		temp = temp->next;
	}
	return temp;
}

HashtableMonitorNode* hash_monitor_search_with_int(HashtableMonitor* ht, int monitorName) {
	char key[100];
	sprintf(key, "%d",monitorName );
	return hash_monitor_search(ht, key);
}

HashtableMonitorNode* hash_monitor_search_pid(HashtableMonitor* ht, pid_t pid) {
	int i;

	for (i = 0; i < ht->hash_nodes; i++) {
		HashtableMonitorNode* temp = ht->nodes[i];

		while (temp != NULL) {
			//ht->nodes[i] = temp->next;

			if(temp->pid == pid) {
				return temp;
			}

			temp = temp->next;
		}
	}
	return NULL;
}

HashtableMonitorNode* hash_monitor_insert(HashtableMonitor* ht, char* monitorName) {

	int pos = hash_function((unsigned char*) monitorName, ht->hash_nodes);

	HashtableMonitorNode* new;

	new = (HashtableMonitorNode*) malloc(sizeof (HashtableMonitorNode));

	new->monitorName = (char*) malloc(strlen(monitorName) + 1);
	strcpy(new->monitorName, monitorName);
	new->next = ht->nodes[pos];
	ht->nodes[pos] = new;

	return new;
}

void hash_monitor_delete(HashtableMonitor* ht, char* monitorName) {

	int pos = hash_function((unsigned char*) monitorName, ht->hash_nodes);

	HashtableMonitorNode* temp = ht->nodes[pos], *temp2;
	int first = 1; // flag to check if we are in first node

	while (temp != NULL) {
		if (!strcmp(temp->monitorName, monitorName)) {
			if (first)
				ht->nodes[pos] = temp->next;
			else
				temp2->next = temp->next;

			free(temp->monitorName);
			free(temp);
			return;
		}
		temp2 = temp;
		temp = temp->next;
		first = 0;
	}
}

void create_pipes(HashtableMonitor* ht_monitors, int numMonitors) {

	for (int j = 0; j < numMonitors; j++) {
		char name[100];
		sprintf(name, "%d", j);

		HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);
		sprintf(node->from_child_to_parent, "from_child_to_parent_%d.fifo", j);
		sprintf(node->from_parent_to_child, "from_parent_to_child_%d.fifo", j);

		//printf("Monitor Node: j=%d, id=%s, %s, %s \n", j, node->monitorName, node->from_child_to_parent, node->from_parent_to_child);

		if (mkfifo(node->from_child_to_parent, 0600) == -1 && errno == EEXIST) {		//create named pipes
			unlink(node->from_child_to_parent);
			mkfifo(node->from_child_to_parent, 0600);            
		}
		
		if (mkfifo(node->from_parent_to_child, 0600) == -1 && errno == EEXIST) {
			unlink(node->from_parent_to_child);
			mkfifo(node->from_parent_to_child, 0600);            
		}
	}
}

void send_countries_to_monitors(HashtableMonitor* ht_monitors, HashtableCountryNode** table, int tablelen, int numMonitors, int bufferSize) {

	for (int j = 0; j < tablelen; j++) {
		char* country = table[j]->countryName;
		table[j]->who = j % numMonitors;        //round robin

		char name[100];
		sprintf(name, "%d", table[j]->who);

		HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

		int fd = node->fd_from_parent_to_child;

		//printf("Sending country :%s to worker %d through pipe: %s via fd: %d \n", country, table[j]->who, node->from_parent_to_child, fd);

		char* info1 = (char*) country;
		int info_length1 = strlen(country) + 1;

		send_info(fd, info1, info_length1, bufferSize);
	}
}

void send_finishing_character(HashtableMonitor* ht_monitors, int numMonitors, int bufferSize) {

	for (int j = 0; j < numMonitors; j++) {
		char name[100];
		char buffer[2] = "#";
		sprintf(name, "%d", j);

		HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

		int fd = node->fd_from_parent_to_child;

		strcpy(buffer, "#");

		char* info1 = (char*) buffer;
		int info_length1 = strlen(buffer) + 1;

		send_info(fd, info1, info_length1, bufferSize);
	}
}

void receive_bloom_filter(HashtableMonitor* ht_monitors, HashtableVirus* ht_viruses, int numMonitors, int bloomSize, int bufferSize) {

	// SELECT
	for (int j = 0; j < numMonitors; j++) {
		char name[100];
		sprintf(name, "%d", j);
		HashtableMonitorNode* node = hash_monitor_search(ht_monitors, name);

		int fd = node->fd_from_child_to_parent;

		while (1) {
			char* info3 = NULL;
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
			
			for (int k=0;k<bloomSize;k++) {
				virusNode->bloom->vector[k] |= bloomVector[k];
			}

			free(bloomVector);            
			free(buffer);            
		}
	}
}

HashtableMonitorNode** hash_monitor_to_array(HashtableMonitor* ht, int* len) {

	int i, j;
	HashtableMonitorNode* temp;

	*len = 0;

	for (i = 0; i < ht->hash_nodes; i++) {
		temp = ht->nodes[i];

		while (temp != NULL) {
			(*len)++;

			temp = temp->next;
		}
	}

	HashtableMonitorNode** table = malloc(sizeof (HashtableMonitorNode*)*(*len));

	int counter = 0;

	for (i = 0; i < ht->hash_nodes; i++) {
		temp = ht->nodes[i];

		while (temp != NULL) {
			table[counter++] = temp;

			temp = temp->next;
		}
	}
	
	 for (i = 0; i < *len - 1; i++) {               //sort array alphabetically
		for (j = i+1; j < *len; j++) {
			if(strcmp(table[i]->monitorName,table[j]->monitorName)>0){
				temp = table[i];
				table[i] = table[j];
				table[j] = temp;
			}
		}
	 }

	return table;
	
}