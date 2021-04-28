#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "date.h"

int date_compare(Date* d1, Date* d2) { //returns -1 if d1<d2, 1 if d1>d2, 0 otherwise

	if (d1->year < d2->year) {
		return -1;
	} else if (d1->year > d2->year) {
		return 1;
	} else {
		if (d1->month < d2->month) {
			return -1;
		} else if (d1->month > d2->month) {
			return 1;
		} else {
			if (d1->day < d2->day) {
				return -1;
			} else if (d1->day > d2->day) {
				return 1;
			} else {
				return 0;
			}
		}
	}
}

Date* get_current_date() {	//return current date

	Date* today = (Date *) calloc(1, sizeof (Date));
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	today->day = tm.tm_mday;
	today->month = tm.tm_mon + 1;
	today->year = tm.tm_year + 1900;

	return today;
}

Date* duplicateDate(Date* d) {	//duplicates date given

	if (d != NULL) {
		Date* copy = calloc(1, sizeof(Date));
	
		copy->day = d->day;
		copy->month = d->month;
		copy->year = d->year;
	
		return copy;
	} else {
		return NULL;
	}
}