#!/bin/bash

ARGC=$#
MAX_ARGS=3
counter=0

declare -a countries

if [[ $ARGC == $MAX_ARGS ]]; then	#check command line arguments
	if [[ -f $1 ]]; then		#check if first argument is a file
		if [ $3 -gt 0 ]; then	#check values of parameters
			mkdir $2;			#create directory

			# while IFS= read -r line; do 			#read inputFile line by line
			# 	countries[$counter] = ( $(awk -F ' ') );
			# 	echo "countries[$counter]";
			# 	let counter=counter+1;
			# done < "$1"

		else
			echo "Wrong values!!!";
		fi
	else
		echo "$1 is not a file!!!";
	fi
else
	echo "Wrong arguments!!!";
fi