#ifndef HASHTABLE_FILENAMES_H
#define HASHTABLE_FILENAMES_H

#include "skiplist.h"
#include "BF.h"
#include "citizen.h"

typedef struct HashtableFilenamesNode {
	char* fileName;
	int who;
	struct HashtableFilenamesNode* next; //pointer to next bucket node
} HashtableFilenamesNode;

typedef struct HashtableFilenames {
	HashtableFilenamesNode** nodes;
	int hash_nodes;
} HashtableFilenames;

HashtableFilenames* hash_filenames_create(int hashNodes);
void hash_filenames_destroy(HashtableFilenames* ht);
HashtableFilenamesNode* hash_filenames_search(HashtableFilenames* ht, char* fileName);
HashtableFilenamesNode* hash_filenames_insert(HashtableFilenames* ht, char* fileName);
void hash_filenames_delete(HashtableFilenames* ht, char* fileName);

HashtableFilenamesNode** hash_filenames_to_array(HashtableFilenames* ht, int* len);

#endif