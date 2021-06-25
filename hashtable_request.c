#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_citizen.h"
#include "hashtable_virus.h"
#include "hashtable_request.h"

HashtableRequest* hash_request_create(int hashNodes) {

	int i;

	HashtableRequest* ht = malloc(sizeof (HashtableRequest));
	ht->hash_nodes = hashNodes;

	ht->nodes = (HashtableRequestNode**) malloc(hashNodes * sizeof (HashtableRequestNode*)); //create hashtable for citizens
	for (i = 0; i < hashNodes; i++) {
		ht->nodes[i] = NULL;
	}

	return ht;
}

void hash_request_destroy(HashtableRequest* ht) {

	int i;

	for (i = 0; i < ht->hash_nodes; i++) {
		HashtableRequestNode* temp = ht->nodes[i];

		while (temp != NULL) {
			ht->nodes[i] = temp->next;

			destroy_request(temp->request);
			//free(temp->date);
			free(temp);

			temp = ht->nodes[i];
		}
	}

	free(ht->nodes);
	free(ht);
}

HashtableRequestNode* hash_request_search(HashtableRequest* ht, char* citizenID) {

	int pos = hash_function((unsigned char*) citizenID, ht->hash_nodes);

	HashtableRequestNode* temp = ht->nodes[pos];

	while (temp != NULL) {
		if (!(strcmp(temp->request->citizenID, citizenID))) {
			return temp;
		}

		temp = temp->next;
	}
	return temp;
}

void hash_request_insert(HashtableRequest* ht, Citizen* request, Date* date) {

	int pos = hash_function((unsigned char*) request->citizenID, ht->hash_nodes);

	HashtableRequestNode* new;

	new = (HashtableRequestNode*) malloc(sizeof (HashtableRequestNode));

	new->request = request;
	new->date = date;

	new->next = ht->nodes[pos];
	ht->nodes[pos] = new;

}

void hash_request_delete(HashtableRequest* ht, char* citizenID) {

	int pos = hash_function((unsigned char*) citizenID, ht->hash_nodes);

	HashtableRequestNode* temp = ht->nodes[pos], *temp2;
	int first = 1; // flag to check if we are in first node

	while (temp != NULL) {
		if (!(strcmp(temp->request->citizenID, citizenID))) {
			if (first)
				ht->nodes[pos] = temp->next;
			else
				temp2->next = temp->next;

			free(temp->request);
			free(temp->date);
			free(temp);
			return;
		}
		temp2 = temp;
		temp = temp->next;
		first = 0;
	}
}