#!/bin/bash

ARGC=$#
MAX_ARGS=3
counter=0

declare -a countries

if [[ $ARGC == $MAX_ARGS ]]; then	#check command line arguments
	if [[ -f $1 ]]; then			#check if first argument is a file
		if [ $3 -gt 0 ]; then		#check values of parameters
			if [ -d $2 ]; then		#create directory if it does not exist
			    echo "Error!!! Directory $2 exists!!!";
			    exit;
			else
				mkdir $2;
			fi

			while read line; do 			#read inputFile line by line
				temp_array=($line);		#copy line in temp_array
				if [[ ! " ${countries[@]} " =~ " ${temp_array[3]} " ]]; then 	#check if countries array contains the third element of temp_array (country)
				    countries[counter]=${temp_array[3]};
				fi
				counter=$((counter+1));
			done < "$1"
		else
			echo "Wrong values!!!";
		fi
	else
		echo "$1 is not a file!!!";
	fi
else
	echo "Wrong arguments!!!";
fi