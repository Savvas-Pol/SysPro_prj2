#ifndef HASHTABLE_VIRUS_H
#define HASHTABLE_VIRUS_H

#include "skiplist.h"
#include "BF.h"
#include "citizen.h"

typedef struct HashtableVirusNode {
	char* virusName;
	BF* bloom;
	SkipList* vaccinated_persons;
	SkipList* not_vaccinated_persons;
	struct HashtableVirusNode* next; //pointer to next bucket node
} HashtableVirusNode;

typedef struct HashtableVirus {
	HashtableVirusNode** nodes;
	int hash_nodes;
} HashtableVirus;

int hash_function(unsigned char *str, int buckets);

HashtableVirus* hash_virus_create(int hashNodes);
void hash_virus_destroy(HashtableVirus* ht);
HashtableVirusNode* hash_virus_search(HashtableVirus* ht, char* virusName);
HashtableVirusNode* hash_virus_insert(HashtableVirus* ht, char* virusName);
void hash_virus_delete(HashtableVirus* ht, char* virusName);

HashtableVirusNode** hash_virus_to_array(HashtableVirus* ht, int* len);

#endif