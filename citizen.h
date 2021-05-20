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

Citizen* create_request(char* id, char* countryTo);
void destroy_request(Citizen* request);

#endif
