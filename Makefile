all: travelMonitor

travelMonitor: travelMonitor.o vaccineMonitor.o help_functions.o skiplist.o hashtable_virus.o hashtable_citizen.o hashtable_country.o hashtable_monitor.o hashtable_filenames.o BF.o date.o commands_vaccinemonitor.o commands_travelmonitor.o citizen.o
	gcc travelMonitor.o vaccineMonitor.o help_functions.o skiplist.o hashtable_virus.o hashtable_citizen.o hashtable_country.o hashtable_monitor.o hashtable_filenames.o BF.o date.o commands_vaccinemonitor.o citizen.o commands_travelmonitor.o -o travelMonitor

travelMonitor.o: travelMonitor.c
	gcc -c travelMonitor.c

vaccineMonitor.o: vaccineMonitor.c
	gcc -c vaccineMonitor.c

help_functions.o: help_functions.c
	gcc -c help_functions.c
	
skiplist.o: skiplist.c
	gcc -c skiplist.c

hashtable_virus.o: hashtable_virus.c
	gcc -c hashtable_virus.c

hashtable_citizen.o: hashtable_citizen.c
	gcc -c hashtable_citizen.c

hashtable_country.o: hashtable_country.c
	gcc -c hashtable_country.c
	
hashtable_monitor.o: hashtable_monitor.c
	gcc -c hashtable_monitor.c
	
hashtable_filenames.o : hashtable_filenames.c
	gcc -c hashtable_filenames.c
	
BF.o: BF.c
	gcc -c BF.c

date.o: date.c
	gcc -c date.c
	
commands_vaccinemonitor.o: commands_vaccinemonitor.c
	gcc -c commands_vaccinemonitor.c
	
commands_travelmonitor.o: commands_travelmonitor.c
	gcc -c commands_travelmonitor.c
	
citizen.o: citizen.c
	gcc -c citizen.c
	
clean:
	rm -f travelMonitor travelMonitor.o vaccineMonitor.o help_functions.o skiplist.o hashtable_virus.o hashtable_citizen.o hashtable_country.o hashtable_monitor.o hashtable_filenames.o BF.o date.o commands_vaccinemonitor.o commands_travelmonitor.o citizen.o
