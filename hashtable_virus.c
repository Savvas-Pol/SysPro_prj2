#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_virus.h"

int hash_function(unsigned char *str, int buckets) {

	unsigned long hash = 5381;
	int c;

	while ((c = *str++) != '\0') {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash % buckets;
}

HashtableVirus* hash_virus_create(int hashNodes) {

	int i;
	
	HashtableVirus* ht = malloc(sizeof (HashtableVirus));
	ht->hash_nodes = hashNodes;

	ht->nodes = (HashtableVirusNode**) malloc(hashNodes * sizeof (HashtableVirusNode*)); //create hashtable for diseases
	for (i = 0; i < hashNodes; i++) {
		ht->nodes[i] = NULL;
	}

	return ht;
}

void hash_virus_destroy(HashtableVirus* ht) {

	int i;

	for (i = 0; i < ht->hash_nodes; i++) {
		HashtableVirusNode* temp = ht->nodes[i];

		while (temp != NULL) {
			ht->nodes[i] = temp->next;

			bloom_destroy(temp->bloom);
			skiplist_destroy(temp->vaccinated_persons);
			skiplist_destroy(temp->not_vaccinated_persons);
			
			free(temp->virusName);
			free(temp);
			
			temp = ht->nodes[i];
		}
	}

	free(ht->nodes);
	free(ht);
}

HashtableVirusNode* hash_virus_search(HashtableVirus* ht, char* virusName) {

	int pos = hash_function((unsigned char*) virusName, ht->hash_nodes);

	HashtableVirusNode* temp = ht->nodes[pos];

	while (temp != NULL) {
		if (!strcmp(temp->virusName, virusName))
			return temp;

		temp = temp->next;
	}
	return temp;
}

HashtableVirusNode* hash_virus_insert(HashtableVirus* ht, char* virusName) {

	int pos = hash_function((unsigned char*) virusName, ht->hash_nodes);

	HashtableVirusNode* new;

	new = (HashtableVirusNode*) malloc(sizeof (HashtableVirusNode));

	new->virusName = (char*) malloc(strlen(virusName) + 1);
	strcpy(new->virusName, virusName);
	new->next = ht->nodes[pos];
	ht->nodes[pos] = new;

	return new;
}

void hash_virus_delete(HashtableVirus* ht, char* virusName) {

	int pos = hash_function((unsigned char*) virusName, ht->hash_nodes);

	HashtableVirusNode* temp = ht->nodes[pos], *temp2;
	int first = 1; // flag to check if we are in first node

	while (temp != NULL) {
		if (!strcmp(temp->virusName, virusName)) {
			if (first)
				ht->nodes[pos] = temp->next;
			else
				temp2->next = temp->next;

			free(temp->virusName);
			free(temp);
			return;
		}
		temp2 = temp;
		temp = temp->next;
		first = 0;
	}
}

HashtableVirusNode** hash_virus_to_array(HashtableVirus* ht, int* len) {
	
	int i, j;
	HashtableVirusNode* temp;

	*len = 0;

	for (i = 0; i < ht->hash_nodes; i++) {
		temp = ht->nodes[i];

		while (temp != NULL) {
			(*len)++;

			temp = temp->next;
		}
	}

	HashtableVirusNode** table = malloc(sizeof (HashtableVirusNode*)*(*len));

	int counter = 0;

	for (i = 0; i < ht->hash_nodes; i++) {
		temp = ht->nodes[i];

		while (temp != NULL) {
			table[counter++] = temp;

			temp = temp->next;
		}
	}
	
	 for (i = 0; i < *len - 1; i++) {				//sort array alphabetically
		for (j = i+1; j < *len; j++) {
			if(strcmp(table[i]->virusName,table[j]->virusName)>0){
				temp = table[i];
				table[i] = table[j];
				table[j] = temp;
			}
		}
	 }

	return table;
}