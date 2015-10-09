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
	echo "[WARNING]: Number of files must be given"
	echo "Usage: arg1 (number of files) [arg2] (directory)"
	exit
fi

if [ $1 -le 0 ]; then
	echo "[WARNING]: Number of files must be a positive integer"
	echo "Usage: arg1 (number of files) [arg2] (directory)"
	exit
fi

#Check the first argument of the program whether it's greater than zero or not.
if ! expr "$1" : '-\?[0-9]\+$' >/dev/null
then
  echo "[WARNING]: Number of files must be an integer"
  echo "Usage: arg1 (number of files) [arg2] (directory)"
  exit
fi

#Check the directory whether it's exist or not
if [ "$#" -eq 2 ]; then
	if [ ! -d "$2" ]; then
  		echo "[WARNING]: Directory $2 does not exist"
  		echo "Usage: arg1 (number of files) [arg2] (directory)"
  		exit
	fi
fi
#Main code
#list the files only (via using grep in the first pipe)
#format it via using awk
#sort the result
#give the first $1 line(s) of the output via using head
ls -al $2 | grep ^- | awk '{print $5,$9}' | sort -n -r | head -n $1
