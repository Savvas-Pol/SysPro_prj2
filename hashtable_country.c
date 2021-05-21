#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_country.h"
#include "hashtable_virus.h"

HashtableCountry* hash_country_create(int hashNodes) {

	int i;

	HashtableCountry* ht = malloc(sizeof (HashtableCountry));
	ht->hash_nodes = hashNodes;

	ht->nodes = (HashtableCountryNode**) malloc(hashNodes * sizeof (HashtableCountryNode*)); //create hashtable for countries
	for (i = 0; i < hashNodes; i++) {
		ht->nodes[i] = NULL;
	}

	return ht;
}

void hash_country_destroy(HashtableCountry* ht) {

	int i;

	for (i = 0; i < ht->hash_nodes; i++) {
		HashtableCountryNode* temp = ht->nodes[i];

		while (temp != NULL) {
			ht->nodes[i] = temp->next;

			free(temp->countryName);
			free(temp);

			temp = ht->nodes[i];
		}
	}

	free(ht->nodes);
	free(ht);
}

HashtableCountryNode* hash_country_search(HashtableCountry* ht, char* countryName) {

	int pos = hash_function((unsigned char*) countryName, ht->hash_nodes);

	HashtableCountryNode* temp = ht->nodes[pos];

	while (temp != NULL) {
		if (!strcmp(temp->countryName, countryName))
			return temp;

		temp = temp->next;
	}
	return temp;
}

HashtableCountryNode* hash_country_insert(HashtableCountry* ht, char* countryName) {

	int pos = hash_function((unsigned char*) countryName, ht->hash_nodes);

	HashtableCountryNode* new;

	new = (HashtableCountryNode*) malloc(sizeof (HashtableCountryNode));

	new->countryName = (char*) malloc(strlen(countryName) + 1);
	strcpy(new->countryName, countryName);
	new->next = ht->nodes[pos];
	ht->nodes[pos] = new;

	return new;
}

void hash_country_delete(HashtableCountry* ht, char* countryName) {

	int pos = hash_function((unsigned char*) countryName, ht->hash_nodes);

	HashtableCountryNode* temp = ht->nodes[pos], *temp2;
	int first = 1; // flag to check if we are in first node

	while (temp != NULL) {
		if (!strcmp(temp->countryName, countryName)) {
			if (first)
				ht->nodes[pos] = temp->next;
			else
				temp2->next = temp->next;

			free(temp->countryName);
			free(temp);
			return;
		}
		temp2 = temp;
		temp = temp->next;
		first = 0;
	}
}

HashtableCountryNode** hash_country_to_array(HashtableCountry* ht, int* len) {
	
	int i, j;
	HashtableCountryNode* temp;

	*len = 0;

	for (i = 0; i < ht->hash_nodes; i++) {
		temp = ht->nodes[i];

		while (temp != NULL) {
			(*len)++;

			temp = temp->next;
		}
	}

	HashtableCountryNode** table = malloc(sizeof (HashtableCountryNode*)*(*len));

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
			if(strcmp(table[i]->countryName,table[j]->countryName)>0){
				temp = table[i];
				table[i] = table[j];
				table[j] = temp;
			}
		}
	 }

	return table;
}