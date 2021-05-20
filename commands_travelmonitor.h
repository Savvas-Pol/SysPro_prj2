#ifndef COMMANDSTV_H
#define COMMANDSTV_H

#include "record.h"
#include "hashtable_virus.h"
#include "hashtable_citizen.h"
#include "hashtable_country.h"
#include "hashtable_monitor.h"

void travel_request(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize, char * citizenID, char* date, char* countryFrom, char* countryTo, char* virusName, int requestID);
void travel_stats(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, char* virusName, char* date1, char* date2);
void travel_stats_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, char* virusName, char* date1, char* date2, char* country);
void add_vaccination_records(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize,  char* country);
void search_vaccination_status(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize, int numMonitors, char* citizenID);

//void insert_citizen_record(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, int bloomSize, Record record, int flag);
//void vaccine_status_bloom(HashtableVirus* ht_viruses, char * citizenID, char * virusName);
//void vaccine_status_id_virus(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, char * citizenID, char * virusName);
//void vaccine_status_id(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, char * citizenID);
//void population_status_virus(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, char* virusName);
//void population_status_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName);
//void population_status_virus_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName, char* date1, char* date2);
//void population_status_country_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName, char* date1, char* date2);
//void pop_status_by_age_virus(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName);
//void pop_status_by_age_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName);
//void pop_status_by_age_virus_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* virusName, char* date1, char* date2);
//void pop_status_by_age_country_dates(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, char* country, char* virusName, char* date1, char* date2);
//void vaccinate_now(HashtableVirus* ht_viruses, HashtableCitizen* ht_citizens, HashtableCountry* ht_countries, int bloomSize, char * citizenID, char * firstName, char * lastName, char * country, char * age, char * virusName);
//void list_nonVaccinated_Persons(HashtableVirus* ht_viruses, char* virusName);

#endif
