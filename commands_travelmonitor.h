#ifndef COMMANDSTV_H
#define COMMANDSTV_H

#include "record.h"
#include "hashtable_virus.h"
#include "hashtable_citizen.h"
#include "hashtable_country.h"
#include "hashtable_monitor.h"
#include "hashtable_request.h"

void travel_request(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, HashtableRequest* ht_requests, int bloomSize, int bufferSize, char * citizenID, char* date, char* countryFrom, char* countryTo, char* virusName, int requestID, int* totalAccepted, int* totalRejected);
void travel_stats(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, char* virusName, char* date1, char* date2);
void travel_stats_country(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, char* virusName, char* date1, char* date2, char* country);
void add_vaccination_records(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize,  char* country);
void search_vaccination_status(HashtableVirus* ht_viruses, HashtableCountry* ht_countries, HashtableMonitor* ht_monitors, int bloomSize, int bufferSize, int numMonitors, char* citizenID);

#endif
