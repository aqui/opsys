#!/bin/bash

<<comment
	Akif Batur - 150111854 - akfbtr@gmail.com
	CSE 333 - OPERATING SYSTEMS
	Programming Assignment # 1
	Marmara University - Computer Engineering
	Copyleft - 2014
comment

clear #Clear screen and print the answer

for d in ${*:2}
do 
	if [ ! -d  $d ]
	then
		echo "There is no such directory $d"
		echo "Usage: [arg1] (pattern) [arg2]...[arg(n)] (directories)"
		exit
	fi
done
#Main code
#If there is no argument
if [ "$#" -eq 0 ]; then
	#For all files in the current directory
	for f in $(find . -type f)
	do 
		#Count the lines starting with a character of each file 
		i=$(grep -c ^ $f)
		echo $i": "$f
		#Add the number of lines to the total
		totalNumberOfLines=$(expr $totalNumberOfLines + $i)
	done
	#Echo out the total number of lines
	if [ -z $totalNumberOfLines ]
	then
		echo "Total is 0."
	else
		echo "+_____________"
  		echo "Total is "$totalNumberOfLines"."
  	fi
fi
#If there is one argument
if [ "$#" -eq 1 ]; then
	#For all files in the current directory with a pattern
	for f in $(find . -type f -iname "*$1*")
	do 
		#Count the lines starting with a character of each file
		i=$(grep -c ^ $f)
		echo $i": "$f
		#Add the number of lines to the total
		totalNumberOfLines=$(expr $totalNumberOfLines + $i)
	done
	#Echo out the total number of lines
	if [ -z $totalNumberOfLines ]
	then
		echo "Total is 0."
	else
		echo "+_____________"
  		echo "Total is "$totalNumberOfLines"."
  	fi
fi
#If there are more than one argument
if [ "$#" -gt 1 ]; then
	#For all files in the given drectories with a pattern
	for f in $(find ${*:2} -type f -iname "*$1*")
	do 
		#Count the lines starting with a character of each file
		i=$(grep -c ^ $f)
		echo $i": "$f
		#Add the number of lines to the total
		totalNumberOfLines=$(expr $totalNumberOfLines + $i)
	done
	#Echo out the total number of lines
	if [ -z $totalNumberOfLines ]
	then
		echo "Total is 0."
	else
		echo "+_____________"
  		echo "Total is "$totalNumberOfLines"."
  	fi
fi
