#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

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

int check_six_months(Date* d1, Date* d2) { //returns 1 if vaccination date is within six months, otherwise 0

	Date* maxDate = (Date *) calloc(1, sizeof(Date));	//max accepted date

	maxDate->day = d1->day;
	maxDate->month = d1->month + 6;
	if(maxDate->month > 12) {
		maxDate->year = d1->year + 1;
		maxDate->month = maxDate->month % 12;
	} else {
		maxDate->year = d1->year;
	}

	if(date_compare(maxDate, d2) == -1) {
		free(maxDate);
		return 0;
	} else {
		free(maxDate);
		return 1;
	}
}

Date* char_to_date(char* date) {	//converts string to Date

	Date* newDate = calloc(1, sizeof (Date));

	char* tempDate = strdup(date);
	char* token = strtok(tempDate, "-");
	int j = 0;
	while (token != NULL) {
		if (j == 0)
			newDate->day = atoi(token);
		else if (j == 1)
			newDate->month = atoi(token);
		else if (j == 2)
			newDate->year = atoi(token);
		token = strtok(NULL, "-\n");
		j++;
	}
	return newDate;
}