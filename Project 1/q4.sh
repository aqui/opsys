#!/bin/bash

<<comment
	Akif Batur - 150111854 - akfbtr@gmail.com
	CSE 333 - OPERATING SYSTEMS
	Programming Assignment # 1
	Marmara University - Computer Engineering
	Copyleft - 2014
comment

clear #Clear screen and print the answer

#if there are no arguments
if [ "$#" -eq 0 ]; then
	echo "[WARNING]: A STOP value must be given at least"
	echo "Usage: arg1 (START) [arg2] (STOP) [arg3] (STEP)"
	exit
fi
#if there is one argument is given
if [ "$#" -eq 1 ]; then
	STOP=$1
	START="1"
	STEP="1"
fi
#if there are two arguments are given
if [ "$#" -eq 2 ]; then
	START=$1
	STOP=$2
	STEP=1
fi
#if there are 3 arguments are given
if [ "$#" -eq 3 ]; then
	START=$1
	STOP=$2
	STEP=$3
fi
#if there are more than 3 arguments are given
if [ "$#" -gt 3 ]; then
	echo "[WARNING]: Too much arguments!"
	echo "Usage: arg1 (START) [arg2] (STOP) [arg3] (STEP)"
	exit
fi

#if STEP<=0
if [ "$STEP" -le 0 ]; then
	echo "[WARNING]: STEP must be a positive integer!"
	echo "Usage: arg1 (START) [arg2] (STOP) [arg3] (STEP)"
	exit
fi
#START must be an integer
if ! expr "$START" : '-\?[0-9]\+$' >/dev/null
then
  echo "[WARNING]: START is not an integer"
  echo "Usage: arg1 (START) [arg2] (STOP) [arg3] (STEP)"
  exit
fi
#STOP must be an integer
if ! expr "$STOP" : '-\?[0-9]\+$' >/dev/null
then
  echo "[WARNING]: START is not an integer"
  echo "Usage: arg1 (START) [arg2] (STOP) [arg3] (STEP)"
  exit
fi
#STEP must be an integer
if ! expr "$STEP" : '-\?[0-9]\+$' >/dev/null
then
  echo "[WARNING]: START is not an integer"
  echo "Usage: arg1 (START) [arg2] (STOP) [arg3] (STEP)"
  exit
fi

echo ""
if [ $START -gt $STOP ]; then
	#If STOP is smaller than START subtract STEP at each interval
	while [ $START -gt $STOP ]
	do
		echo $START
		START=$(( START - STEP ))
	done
	exit
elif [ $START -lt $STOP ]; then
	#If STOP is greater than START add STEP at each interval
	while [ $START -lt $STOP ]
	do
		echo $START
		START=$(( START + STEP ))
	done
	exit
fi
