#include <string.h>
#include <stdlib.h>

#include "citizen.h"

Citizen* citizen_create(char* citizenID, char* firstName, char* lastName, char* country, int age) {	//creates new citizen
	Citizen* c = (Citizen*) calloc(1, sizeof (Citizen));

	c->citizenID = strdup(citizenID);
	c->firstName = strdup(firstName);
	c->lastName = strdup(lastName);
	c->country = strdup(country);
	c->age = age;

	return c;
}

void citizen_destroy(Citizen* c) {		//free
	free(c->citizenID);
	free(c->country);
	free(c->firstName);
	free(c->lastName);
	free(c);
}