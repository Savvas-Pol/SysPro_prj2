#ifndef HASHTABLE_CITIZEN_H
#define HASHTABLE_CITIZEN_H

#include "skiplist.h"
#include "BF.h"
#include "citizen.h"

typedef struct HashtableCitizenNode {
	Citizen * citizen;
	struct HashtableCitizenNode* next; //pointer to next bucket node
} HashtableCitizenNode;

typedef struct HashtableCitizen {
	HashtableCitizenNode** nodes;
	int hash_nodes;
} HashtableCitizen;

HashtableCitizen* hash_citizen_create(int hashNodes);
void hash_citizen_destroy(HashtableCitizen* ht);
HashtableCitizenNode* hash_citizen_search(HashtableCitizen* ht, char* citizenID);
HashtableCitizenNode* hash_citizen_search_for_all_fields(HashtableCitizen* ht, Citizen *citizen);
HashtableCitizenNode* hash_citizen_insert(HashtableCitizen* ht, Citizen *citizen);
void hash_citizen_delete(HashtableCitizen* ht, char* citizenID);

#endif