#ifndef BF_H
#define BF_H

#include <stdbool.h>

typedef struct BF {
	char* vector;
	int size;
} BF;

unsigned long djb2(unsigned char *str);
unsigned long sdbm(unsigned char *str);
unsigned long hash_i(unsigned char *str, unsigned int i);

BF* bloom_init(int n);			//initialize bloom filter with given size
void bloom_destroy(BF* b);		//destroy
void bloom_filter_insert(BF* b, char* str, int K);		//insert in bloom filter
int bloom_filter_check(BF* b, char* str, int K);		//checks bloom filter and returns 0 if NO or 1 if MAYBE

#endif
