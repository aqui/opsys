#!/bin/bash

<<comment
	Akif Batur - 150111854 - akfbtr@gmail.com
	CSE 333 - OPERATING SYSTEMS
	Programming Assignment # 1
	Marmara University - Computer Engineering
	Copyleft - 2014
comment

clear #Clear screen and print the answer

#Check the first argument of the program whether it's exist or not.
if [ -z $1 ]; then
	echo "[WARNING]: A pattern must be given"
	echo "Usage: arg1 (pattern)"
	exit
fi
#Main code
#find the file with given pattern and format the output
#sort the output
#re-format the output with awk
find -iname "*$1*" -type f -printf "%TY-%Tm-%Td %TH:%TM %f\n" | sort -n | awk '{print "The file " $3 " was modified on " $1, "at " $2"."}'
