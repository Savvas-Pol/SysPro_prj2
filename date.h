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
int check_six_months(Date* d1, Date* d2); //returns 1 if vaccination date is within six months, otherwise 0

Date* char_to_date(char* date);	//converts string to Date

#endif
