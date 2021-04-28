#ifndef DATE_H
#define DATE_H

typedef struct Date {
	int day;
	int month;
	int year;
} Date;

int date_compare(Date* d1, Date* d2); //returns -1 if d1<d2, 1 if d1>d2, 0 otherwise
Date* get_current_date();	//return current date
Date* duplicateDate(Date* d);	//duplicates date given

#endif
