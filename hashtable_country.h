#ifndef HASHTABLE_COUNTRY_H
#define HASHTABLE_COUNTRY_H

#include "skiplist.h"
#include "BF.h"
#include "citizen.h"

typedef struct HashtableCountryNode {
	char* countryName;
	int who;
	struct HashtableCountryNode* next; //pointer to next bucket node
} HashtableCountryNode;

typedef struct HashtableCountry {
	HashtableCountryNode** nodes;
	int hash_nodes;
} HashtableCountry;

HashtableCountry* hash_country_create(int hashNodes);
void hash_country_destroy(HashtableCountry* ht);
HashtableCountryNode* hash_country_search(HashtableCountry* ht, char* countryName);
HashtableCountryNode* hash_country_insert(HashtableCountry* ht, char* countryName);
void hash_country_delete(HashtableCountry* ht, char* countryName);

HashtableCountryNode** hash_country_to_array(HashtableCountry* ht, int* len);

#endif