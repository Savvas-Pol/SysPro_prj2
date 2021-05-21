#ifndef COMMANDS_H
#define COMMANDS_H

#include "record.h"
#include "hashtable_virus.h"
#include "hashtable_citizen.h"
#include "hashtable_country.h"
#include "hashtable_filenames.h"

void insert_citizen_record(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, int bloomSize, Record record, int flag);
int vaccine_status_bloom(HashtableVirus* ht_viruses, char* citizenID, char* virusName);
int vaccine_status_id_virus(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, char* citizenID, char* virusName, char* travelDate);
void vaccine_status_id(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, char* citizenID, int bufferSize, int readfd, int writefd);
void population_status_virus(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, char* virusName);
void population_status_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName);
void population_status_virus_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName, char* date1, char* date2);
void population_status_country_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName, char* date1, char* date2);
void pop_status_by_age_virus(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName);
void pop_status_by_age_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName);
void pop_status_by_age_virus_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName, char* date1, char* date2);
void pop_status_by_age_country_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName, char* date1, char* date2);
void vaccinate_now(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, int bloomSize, char* citizenID, char* firstName, char* lastName, char* country, char* age, char* virusName);
void list_nonVaccinated_Persons(HashtableVirus* ht_viruses, char* virusName);

int travel_request_for_child(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens,  char* citizenID, char* date, char* countryFrom,char* virusName, int readfd, int writefd, int bs);
void add_vaccination_records_for_child(char* inputDirectoryPath, HashtableFilenames* ht_filenames, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, HashtableVirus* ht_viruses, HashtableVirusNode** table2, int tablelen2, int bloomSize, char* from_child_to_parent, int bufferSize, int readfd, int writefd);
void search_vaccination_status_for_child(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableCitizen* ht_citizens, int bloomSize, int bufferSize, int readfd, int writefd, char* citizenID);

#endif
