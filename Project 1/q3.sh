#!/bin/bash

<<comment
	Akif Batur - 150111854 - akfbtr@gmail.com
	CSE 333 - OPERATING SYSTEMS
	Programming Assignment # 1
	Marmara University - Computer Engineering
	Copyleft - 2014
comment

clear #Clear screen and print the answer

#Remove temporaryfile.txt if exist
if [ -f temporaryfile.txt ]; then
    rm temporaryfile.txt
fi

#Check the first argument of the program whether it's exist or not.
if [ -z $1 ]; then
	echo "[WARNING]: A pattern must be given"
	echo "Usage: arg1 (pattern) [arg2] (directory)"
	exit
fi
#Set the directory
maindirectory=$(pwd)"/*"
if [ -z $2 ]; then
	directory=$maindirectory
else
	directory=$2"/*"
fi
mainfile=$(pwd)"/temporaryfile.txt"
#Main code
echo "Please wait..."
#Find all files under the directory 
for file in $(find $directory -type f)
do
	echo $file" is processing..."
	#If the current processing file is the temprorayfile.txt then stop processing and continue to the next file.
	if [ $file = $mainfile ]
	then
	  continue
	fi
	#Start to reading each file line by line
	while read line || [ -n "$line" ];
	do
		#Get the each word in the each line
		for word in $line
		do
			#Write the words to the file if it's contains the pattern
			echo $word | grep -e "$1" >> temporaryfile.txt
		done
	done < $file
done
echo "Done!"
echo "                            RESULT"
echo ""
#Read and sort temproraryfile.txt and seperate it to columns
cat temporaryfile.txt | sort | grep -v '\/'| column -c 80
#Remove temproraryfile.txt
rm temporaryfile.txt
