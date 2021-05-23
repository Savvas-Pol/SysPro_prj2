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
				    countries[counter]=${temp_array[3]};						#store new countries in array
				fi
				counter=$((counter+1));
			done < "$1"

			for i in ${countries[@]}; do 			#for each country
				mkdir ./$2/$i; 						#create new subdirectory
				for(( j=1; j<=$3; j++ )); do
					touch ./$2/$i/${i}-${j}.txt;	#create 	
				done
				touch ./$2/$i/${i}.txt;	#create temp file for each country's records
			done

			while read line; do 			#read inputFile line by line
				temp_array=($line);		#copy line in temp_array
				echo "$line" >> ./$2/${temp_array[3]}/${temp_array[3]}.txt;		#store lines in temp file
			done < "$1"
			
			for i in ${countries[@]}; do 			#for each country
				counter=1;							#counter for round robin
				while read line; do 
					echo "$line" >> ./$2/$i/${i}-${counter}.txt;		#append line to file
					counter=$((counter+1));
					if [[ $counter > $3 ]]; then	#if counter reaches numFilesPerDirectory, reset to 1
						counter=1;
					fi
				done < ./$2/$i/$i.txt;			#read from temp file
				rm ./$2/$i/${i}.txt;			#delete temp file
			done

		else
			echo "Wrong values!!!";
		fi
	else
		echo "$1 is not a file!!!";
	fi
else
	echo "Wrong arguments!!!";
fi