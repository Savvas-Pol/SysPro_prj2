#ifndef CITIZEN_H
#define CITIZEN_H

typedef struct Citizen {
	char* citizenID;
	char* firstName;
	char* lastName;
	char* country;
	int age;
} Citizen;

Citizen* citizen_create( char* citizenID, char* firstName, char* lastName, char* country, int age);	//creates new citizen
void citizen_destroy(Citizen* c);		//free

#endif
