#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_filenames.h"
#include "hashtable_virus.h"

HashtableFilenames* hash_filenames_create(int hashNodes) {

	int i;

	HashtableFilenames* ht = malloc(sizeof (HashtableFilenames));
	ht->hash_nodes = hashNodes;

	ht->nodes = (HashtableFilenamesNode**) malloc(hashNodes * sizeof (HashtableFilenamesNode*)); //create hashtable for filenames
	for (i = 0; i < hashNodes; i++) {
		ht->nodes[i] = NULL;
	}

	return ht;
}

void hash_filenames_destroy(HashtableFilenames* ht) {

	int i;

	for (i = 0; i < ht->hash_nodes; i++) {
		HashtableFilenamesNode* temp = ht->nodes[i];

		while (temp != NULL) {
			ht->nodes[i] = temp->next;

			free(temp->fileName);
			free(temp);

			temp = ht->nodes[i];
		}
	}

	free(ht->nodes);
	free(ht);
}

HashtableFilenamesNode* hash_filenames_search(HashtableFilenames* ht, char* fileName) {

	int pos = hash_function((unsigned char*) fileName, ht->hash_nodes);

	HashtableFilenamesNode* temp = ht->nodes[pos];

	while (temp != NULL) {
		if (!strcmp(temp->fileName, fileName))
			return temp;

		temp = temp->next;
	}
	return temp;
}

HashtableFilenamesNode* hash_filenames_insert(HashtableFilenames* ht, char* fileName) {

	int pos = hash_function((unsigned char*) fileName, ht->hash_nodes);

	HashtableFilenamesNode* new;

	new = (HashtableFilenamesNode*) malloc(sizeof (HashtableFilenamesNode));

	new->fileName = (char*) malloc(strlen(fileName) + 1);
	strcpy(new->fileName, fileName);
	new->next = ht->nodes[pos];
	ht->nodes[pos] = new;

	return new;
}

void hash_filenames_delete(HashtableFilenames* ht, char* fileName) {

	int pos = hash_function((unsigned char*) fileName, ht->hash_nodes);

	HashtableFilenamesNode* temp = ht->nodes[pos], *temp2;
	int first = 1; // flag to check if we are in first node

	while (temp != NULL) {
		if (!strcmp(temp->fileName, fileName)) {
			if (first)
				ht->nodes[pos] = temp->next;
			else
				temp2->next = temp->next;

			free(temp->fileName);
			free(temp);
			return;
		}
		temp2 = temp;
		temp = temp->next;
		first = 0;
	}
}

HashtableFilenamesNode** hash_filenames_to_array(HashtableFilenames* ht, int* len) {
	
	int i, j;
	HashtableFilenamesNode* temp;

	*len = 0;

	for (i = 0; i < ht->hash_nodes; i++) {
		temp = ht->nodes[i];

		while (temp != NULL) {
			(*len)++;

			temp = temp->next;
		}
	}

	HashtableFilenamesNode** table = malloc(sizeof (HashtableFilenamesNode*)*(*len));

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
			if(strcmp(table[i]->fileName,table[j]->fileName)>0){
				temp = table[i];
				table[i] = table[j];
				table[j] = temp;
			}
		}
	 }

	return table;
}