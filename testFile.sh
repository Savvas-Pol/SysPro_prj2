#!/bin/bash

> inputFile
ARGC=$#
MAX_ARGS=4
counter=0
arr[0]="YES"
arr[1]="NO"

declare -a viruses
declare -a countries
declare -a uniqueIDs


if [[ $ARGC == $MAX_ARGS ]]; then	#check command line arguments
	if [[ $3 > 0 && ($4 == 0 || $4 == 1) ]]; then	#check values of numbers given
		while IFS= read -r line; do 			#read virusesFile line by line
			viruses[$counter]=$line;
			let counter=counter+1;
		done < "$1"
		counter=0
		while IFS= read -r line; do 			#read countriesFile line by line
			countries[$counter]=$line;
			let counter=counter+1;
		done < "$2"
		
		for(( i=0; i<$3; i++ ))
		do
			if [[ $4 == 1 ]]; then	#if duplicatesAllowed
				temp_rand=$(($RANDOM % 4));		#random duplicates in some cases
				
				id=$((1 + $RANDOM % 9999));
				firstname=$(tr -dc A-Za-z </dev/urandom | head -c $((3 + $RANDOM % 10)))
				lastname=$(tr -dc A-Za-z </dev/urandom | head -c $((3 + $RANDOM % 10)))
				country=${countries[$RANDOM % ${#countries[@]} ]}
				age=$((1 + $RANDOM % 120))
				virus=${viruses[$RANDOM % ${#viruses[@]} ]}
				rand=$(($RANDOM % 2));
				yesno=${arr[$rand]}
						
				if [[ $rand == 0 ]]; then				#if YES
					rand=$[ $RANDOM % 4 ];
					if [[ $rand == 0 ]]; then
						echo "$id $firstname $lastname $country $age $virus $yesno " >> inputFile;		#25% print without date
					else
						echo -n "$id $firstname $lastname $country $age $virus $yesno " >> inputFile
						echo -n $((1 + $RANDOM % 30)) >> inputFile; echo -n "-" >> inputFile; 
						echo -n $((1 + $RANDOM % 12)) >> inputFile; echo -n "-" >> inputFile;
						echo $((1900 + $RANDOM % 120)) >> inputFile;
					fi
				else		#NO
					echo "$id $firstname $lastname $country $age $virus $yesno " >> inputFile
				fi
				for(( j=0; j<$temp_rand; j++ ))		#print duplicate records in some cases
				do
					if [[ $i == $3 ]]; then
						break
					fi
					virus=${viruses[$RANDOM % ${#viruses[@]} ]}
					rand=$(($RANDOM % 2));
					yesno=${arr[$rand]}

					rand=$[ $RANDOM % 4 ];
					if [[ $rand == 0 ]]; then
						echo "$id $firstname $lastname $country $age $virus $yesno " >> inputFile;		#25% print without date
					else
						echo -n "$id $firstname $lastname $country $age $virus $yesno " >> inputFile
						echo -n $((1 + $RANDOM % 30)) >> inputFile; echo -n "-" >> inputFile; 
						echo -n $((1 + $RANDOM % 12)) >> inputFile; echo -n "-" >> inputFile;
						echo $((1900 + $RANDOM % 120)) >> inputFile;
					fi
					i=$((i+1))
				done
				if [[ $i == $3 ]]; then
					break
				fi
			else		#duplicates not allowed
				id=$((1 + $RANDOM % 9999));
				
				while [[ " ${uniqueIDs[@]} " =~ " ${id} " ]]; do 	#check if array contains value
					id=$((1 + $RANDOM % 9999));
				done
				
				id=$((1 + $RANDOM % 9999));
				firstname=$(tr -dc A-Za-z </dev/urandom | head -c $((3 + $RANDOM % 10)))
				lastname=$(tr -dc A-Za-z </dev/urandom | head -c $((3 + $RANDOM % 10)))
				country=${countries[$RANDOM % ${#countries[@]} ]}
				age=$((1 + $RANDOM % 120))
				virus=${viruses[$RANDOM % ${#viruses[@]} ]}
				rand=$(($RANDOM % 2));
				yesno=${arr[$rand]}
				
				rand=$[ $RANDOM % 4 ];
				if [[ $rand == 0 ]]; then
					echo "$id $firstname $lastname $country $age $virus $yesno " >> inputFile;		#25% print without date
				else
					echo -n "$id $firstname $lastname $country $age $virus $yesno " >> inputFile
					echo -n $((1 + $RANDOM % 30)) >> inputFile; echo -n "-" >> inputFile;
					echo -n $((1 + $RANDOM % 12)) >> inputFile; echo -n "-" >> inputFile;
					echo $((1900 + $RANDOM % 120)) >> inputFile;
				fi
			fi
		done
	else
		printf "Wrong values!!!\n";
	fi
else
	printf "Wrong arguments!!!\n";
fi

