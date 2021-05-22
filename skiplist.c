#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "skiplist.h"

SkipListNode* skiplist_create_node(Citizen* citizen, Date* date, int maxLevel) { //creates new node in skiplist
	SkipListNode* sn = malloc(sizeof (SkipListNode));
	sn->citizen = citizen;
	sn->date = date;
	sn->next = calloc(SKIP_LIST_MAX_LEVEL, sizeof (SkipListNode*));
	return sn;
}

SkipList* skiplist_init(int maxLevel) { //initialize skiplist with given size

	SkipList* sl = malloc(sizeof (SkipList));
	sl->maxLevel = maxLevel;
	sl->head = skiplist_create_node(NULL, NULL, maxLevel);
	sl->tail = skiplist_create_node(NULL, NULL, maxLevel);

	for (int i = 0; i < maxLevel; i++) {
		sl->head->next[i] = sl->tail;
	}

	sl->tail->citizen = calloc(1, sizeof (Citizen));

	sl->tail->citizen->citizenID = strdup("ZZZZZ");

	return sl;
}

void skiplist_destroy(SkipList* sl) { //destroy
	SkipListNode* l = sl->head;
	SkipListNode* temp = l;

	free(sl->tail->citizen->citizenID);

	free(sl->tail->citizen);

	while (temp != NULL) {
		l = temp->next[0];

		if (temp->date)
			free(temp->date);

		free(temp->next);
		free(temp);

		temp = l;
	}

	free(sl);
}

SkipListNode* skiplist_search(SkipList* sl, char* id) {
	int i;
	SkipListNode* l = sl->head;

	for (i = SKIP_LIST_MAX_LEVEL - 1; i >= 0; i--) {
		while (l->next[i] != sl->tail && strcmp(l->next[i]->citizen->citizenID, id) < 0) {
			l = l->next[i];
		}
	}

	l = l->next[0];

	if (!strcmp(l->citizen->citizenID, id))
		return l;
	else
		return NULL;

	return 0;
}

void skiplist_insert(SkipList* sl, Citizen* citizen, Date* date, char* id) {

	int i, level;
	SkipListNode * update[sl->maxLevel];
	SkipListNode* l = sl->head;

	date = duplicateDate(date);

	for (i = SKIP_LIST_MAX_LEVEL - 1; i >= 0; i--) {
		while (l->next[i] != sl->tail && strcmp(l->next[i]->citizen->citizenID, id) < 0) {
			l = l->next[i];
		}
		update[i] = l;
	}

	l = l->next[0];

	if (strcmp(l->citizen->citizenID, id) == 0) {
		l->citizen = citizen;
		l->date = date;
	} else {
		level = (rand() % SKIP_LIST_MAX_LEVEL) + 1;

		l = skiplist_create_node(citizen, date, sl->maxLevel);
		for (i = 0; i < level; i++) {
			l->next[i] = update[i]->next[i];
			update[i]->next[i] = l;
		}
	}
}

void skiplist_delete(SkipList* sl, char* id) { //delete node with given id

	int i;
	SkipListNode * update[sl->maxLevel];
	SkipListNode* l = sl->head;

	for (i = SKIP_LIST_MAX_LEVEL - 1; i >= 0; i--) {
		while (l->next[i] != sl->tail && strcmp(l->next[i]->citizen->citizenID, id) < 0) {
			l = l->next[i];
		}
		update[i] = l;
	}

	l = l->next[0];

	if (strcmp(l->citizen->citizenID, id) == 0) {
		for (i = 0; i < SKIP_LIST_MAX_LEVEL - 1; i++) {
			if (update[i]->next[i] != l) {
				break;
			}
			update[i]->next[i] = l->next[i];
		}
		free(l->next);
		free(l);
	}
}