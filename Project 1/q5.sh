#!/bin/bash

<<comment
	Akif Batur - 150111854 - akfbtr@gmail.com
	CSE 333 - OPERATING SYSTEMS
	Programming Assignment # 1
	Marmara University - Computer Engineering
	Copyleft - 2014
comment
#Main code
#Infinite loop
while :
do
	#Start reading numbers
	read number
	case "$number" in
		#Echo out the total and exit if the number is an empty line
		"") echo "Total is "$sum"."
			exit;;
		#The number is must be an integer. if so, then add it to the sum.
		#Do nothing otherwise
		*) 	if expr "$number" : '-\?[0-9]\+$' >/dev/null
			then
				sum=$(expr $sum + $number)
			fi
		;;
	esac
done
