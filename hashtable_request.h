#ifndef HASHTABLE_REQUEST_H
#define HASHTABLE_REQUEST_H

#include "skiplist.h"
#include "BF.h"
#include "citizen.h"

typedef struct HashtableRequestNode {
	Citizen* request;
	Date* date;

	struct HashtableRequestNode* next; //pointer to next bucket node
} HashtableRequestNode;

typedef struct HashtableRequest {
	HashtableRequestNode** nodes;
	int hash_nodes;
} HashtableRequest;

HashtableRequest* hash_request_create(int hashNodes);
void hash_request_destroy(HashtableRequest* ht);
HashtableRequestNode* hash_request_search(HashtableRequest* ht, char* citizenID);
void hash_request_insert(HashtableRequest* ht, Citizen* request, Date* date);
void hash_request_delete(HashtableRequest* ht, char* citizenID);

#endif