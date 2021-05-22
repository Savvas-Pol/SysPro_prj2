#ifndef SKIPLIST_H
#define SKIPLIST_H

#include "citizen.h"
#include "date.h"
#include "constants.h"

typedef struct SkipListNode {
	Citizen* citizen;
	Date* date;

	struct SkipListNode **next;
} SkipListNode;

typedef struct SkipList {
	struct SkipListNode* head;
	struct SkipListNode* tail;
	int maxLevel;
} SkipList;

SkipListNode* skiplist_create_node(Citizen* citizen, Date* date, int maxLevel);		//creates new node in skiplist
SkipList* skiplist_init(int maxLevel);			//initialize skiplist with given size
void skiplist_destroy(SkipList* sl);		//destroy

SkipListNode* skiplist_search(SkipList* sl, char* id);
void skiplist_insert(SkipList* sl, Citizen* citizen, Date* date, char* id);
void skiplist_delete(SkipList* sl, char* id);			//delete node with given id

#endif

