#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_citizen.h"
#include "hashtable_virus.h"

HashtableCitizen* hash_citizen_create(int hashNodes) {

	int i;

	HashtableCitizen* ht = malloc(sizeof (HashtableCitizen));
	ht->hash_nodes = hashNodes;

	ht->nodes = (HashtableCitizenNode**) malloc(hashNodes * sizeof (HashtableCitizenNode*)); //create hashtable for citizens
	for (i = 0; i < hashNodes; i++) {
		ht->nodes[i] = NULL;
	}

	return ht;
}

void hash_citizen_destroy(HashtableCitizen* ht) {

	int i;

	for (i = 0; i < ht->hash_nodes; i++) {
		HashtableCitizenNode* temp = ht->nodes[i];

		while (temp != NULL) {
			ht->nodes[i] = temp->next;

			citizen_destroy(temp->citizen);
			free(temp);

			temp = ht->nodes[i];
		}
	}

	free(ht->nodes);
	free(ht);
}

HashtableCitizenNode* hash_citizen_search(HashtableCitizen* ht, char* citizenID) {

	int pos = hash_function((unsigned char*) citizenID, ht->hash_nodes);

	HashtableCitizenNode* temp = ht->nodes[pos];

	while (temp != NULL) {
		if (!(strcmp(temp->citizen->citizenID, citizenID))) {
			return temp;
		}

		temp = temp->next;
	}
	return temp;
}

HashtableCitizenNode* hash_citizen_search_for_all_fields(HashtableCitizen* ht, Citizen *citizen) {
	int pos = hash_function((unsigned char*) citizen->citizenID, ht->hash_nodes);

	HashtableCitizenNode* temp = ht->nodes[pos];

	while (temp != NULL) {
		if (!(strcmp(temp->citizen->citizenID, citizen->citizenID)) &&
				!(strcmp(temp->citizen->firstName, citizen->firstName)) &&
				!(strcmp(temp->citizen->lastName, citizen->lastName)) &&
				!(strcmp(temp->citizen->country, citizen->country)) &&
				(temp->citizen->age == citizen->age)) {
			return temp;
		}

		temp = temp->next;
	}
	return temp;
}

HashtableCitizenNode* hash_citizen_insert(HashtableCitizen* ht, Citizen *citizen) {

	int pos = hash_function((unsigned char*) citizen->citizenID, ht->hash_nodes);

	HashtableCitizenNode* new;

	new = (HashtableCitizenNode*) malloc(sizeof (HashtableCitizenNode));

	new->citizen = citizen;

	new->next = ht->nodes[pos];
	ht->nodes[pos] = new;

	return new;
}

void hash_citizen_delete(HashtableCitizen* ht, char* citizenID) {

	int pos = hash_function((unsigned char*) citizenID, ht->hash_nodes);

	HashtableCitizenNode* temp = ht->nodes[pos], *temp2;
	int first = 1; // flag to check if we are in first node

	while (temp != NULL) {
		if (!(strcmp(temp->citizen->citizenID, citizenID))) {
			if (first)
				ht->nodes[pos] = temp->next;
			else
				temp2->next = temp->next;

			free(temp->citizen);
			free(temp);
			return;
		}
		temp2 = temp;
		temp = temp->next;
		first = 0;
	}
}