#!/bin/bash

ARGC=$#
MAX_ARGS=3

if [[ $ARGC == $MAX_ARGS ]]; then	#check command line arguments
	if [ $3 -gt 0 ]; then	#check values of parameters
		mkdir $2;			#create directory

	else
		echo "Wrong values!!!\n";
	fi
else
	echo "Wrong arguments!!!\n";
fi