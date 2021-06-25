OBJS = travelMonitor.o vaccineMonitor.o help_functions.o skiplist.o hashtable_virus.o hashtable_citizen.o hashtable_country.o hashtable_monitor.o hashtable_filenames.o hashtable_request.o BF.o date.o commands_vaccinemonitor.o commands_travelmonitor.o citizen.o
OUT = travelMonitor vaccineMonitor

OBJSTRAVEL = travelMonitor.o help_functions.o skiplist.o hashtable_virus.o hashtable_citizen.o hashtable_country.o hashtable_monitor.o hashtable_filenames.o hashtable_request.o BF.o date.o commands_travelmonitor.o commands_vaccinemonitor.o citizen.o
SOURCETRAVEL = travelMonitor.c help_functions.c skiplist.c hashtable_virus.c hashtable_citizen.c hashtable_country.c hashtable_monitor.c hashtable_filenames.c hashtable_request.c BF.c date.c commands_travelmonitor.c commands_vaccinemonitor.c citizen.c
OUTTRAVEL = travelMonitor

OBJSVACCINE = vaccineMonitor.o help_functions.o skiplist.o hashtable_virus.o hashtable_citizen.o hashtable_country.o hashtable_monitor.o hashtable_filenames.o BF.o date.o commands_vaccinemonitor.o citizen.o
SOURCEVACCINE = vaccineMonitor.c help_functions.c skiplist.c hashtable_virus.c hashtable_citizen.c hashtable_country.c hashtable_monitor.c hashtable_filenames.c BF.c date.c commands_vaccinemonitor.c citizen.c
OUTVACCINE = vaccineMonitor

CC = gcc
FLAGS = -g -c -Wall

all: travelMonitor vaccineMonitor

travelMonitor: $(OBJSTRAVEL)
	$(CC) -g $(OBJSTRAVEL) -o $(OUTTRAVEL)
	
vaccineMonitor:  $(OBJSVACCINE)
	$(CC) -g $(OBJSVACCINE) -o $(OUTVACCINE)
	
travelMonitor.o: travelMonitor.c
	$(CC) $(FLAGS) travelMonitor.c

vaccineMonitor.o: vaccineMonitor.c
	$(CC) $(FLAGS) vaccineMonitor.c

help_functions.o: help_functions.c
	$(CC) $(FLAGS) help_functions.c
	
skiplist.o: skiplist.c
	$(CC) $(FLAGS) skiplist.c

hashtable_virus.o: hashtable_virus.c
	$(CC) $(FLAGS) hashtable_virus.c

hashtable_citizen.o: hashtable_citizen.c
	$(CC) $(FLAGS) hashtable_citizen.c

hashtable_country.o: hashtable_country.c
	$(CC) $(FLAGS) hashtable_country.c
	
hashtable_monitor.o: hashtable_monitor.c
	$(CC) $(FLAGS) hashtable_monitor.c
	
hashtable_filenames.o : hashtable_filenames.c
	$(CC) $(FLAGS) hashtable_filenames.c

hashtable_request.o: hashtable_request.c
	$(CC) $(FLAGS) hashtable_request.c
	
BF.o: BF.c
	$(CC) $(FLAGS) BF.c

date.o: date.c
	$(CC) $(FLAGS) date.c
	
commands_vaccinemonitor.o: commands_vaccinemonitor.c
	$(CC) $(FLAGS) commands_vaccinemonitor.c
	
commands_travelmonitor.o: commands_travelmonitor.c
	$(CC) $(FLAGS) commands_travelmonitor.c
	
citizen.o: citizen.c
	$(CC) $(FLAGS) citizen.c
	
clean:
	rm -f $(OBJS) $(OUT)
