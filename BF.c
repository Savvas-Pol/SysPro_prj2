#include <stdio.h>
#include <stdlib.h>

#include "BF.h"

unsigned long djb2(unsigned char *str) {
	int c;
	unsigned long hash = 5381;

	while ((c = *str++) != '\0') {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}

	return hash;
}

unsigned long sdbm(unsigned char *str) {
	int c;
	unsigned long hash = 0;

	while ((c = *str++) != '\0') {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

unsigned long hash_i(unsigned char *str, unsigned int i) {
	return djb2(str) + i * sdbm(str) + i * i;
}

BF* bloom_init(int bloomSize) {			//initialize bloom filter with given size

	BF* bloom;

	if ((bloom = malloc(sizeof (BF))) == NULL) {
		printf(" Error in allocation of Bloom Filter...\n");
		return NULL;
	}

	bloom->size = bloomSize;

	if ((bloom->vector = calloc(bloom->size, sizeof (char))) == NULL) {
		printf(" Error in allocation of vector...\n");
		return NULL;
	}

	return bloom;

}

void bloom_destroy(BF * b) {		//destroy
	free(b->vector);
	free(b);
}

void bloom_filter_insert(BF* b, char* str, int K) {		//insert in bloom filter
	int pos, index, offset;
	char mask;
	int bits = 8*b->size;

	for (int i = 0; i < K; i++) {
		pos = hash_i((unsigned char*)str, i) % bits;
		index = pos / 8;
		offset = pos % 8;
		mask = 1 << offset;
		b->vector[index] = b->vector[index] | mask;
	}
}

int bloom_filter_check(BF* b, char* str, int K) {		//checks bloom filter and returns 0 if NO or 1 if MAYBE
	int pos, index, offset;
	char mask;
	int bits = 8*b->size;
	int res;

	for (int i = 0; i < K; i++) {
		pos = hash_i((unsigned char*)str, i) % bits;
		index = pos / 8;
		offset = pos % 8;
		mask = 1 << offset;
		res = b->vector[index] & mask;
		
		if (res == 0) {
			return 0;		//NO
		}
	}
	
	return 1;		//MAYBE
}