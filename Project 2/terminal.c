/**
	Akif Batur - 150111854 - akfbtr@gmail.com
	CSE 333 - OPERATING SYSTEMS
	Programming Assignment # 2
	Marmara University - Computer Engineering
	Copyleft - 2014
**/

#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <regex.h>

//Function prototypes
char* leftTrim(char* str); //trim left of the string
char* rightTrim(char* str); //trim right of the string
void exitSignal(int sgnl); //for "exit" command
void ctrlcSignal(int sgnl); //for ctrl^c
char* execvPath(char* argument); //used to get the path of the command

char *historyArray[10]; //To keep history of commands previously issued 

int backgorundSize = 0; //To understand if it's a background process or not

//GO GO GO!!!
int main (int argc, char ** argv)
{
	signal(SIGCHLD, exitSignal);//for "exit" command
	signal(SIGINT, ctrlcSignal);//for ctrl^c

	//Some variables
	char buffer[1024]; //To keep current command line
	char bufferBackUp[1024]; //Backup of the current command line                
    char *args[120]; //command and its arguments list
    char **arg; //command arguments
    int  status; //to keep child process status
    pid_t childID, endID; //childID: to keep forked child id. endID: to get ended child process id
	char pwd[1024]; //To keep current working directory

    system("clear"); //Clear screen

	char shellPath[1024]; //To set SHELL environment variable
	readlink("/proc/self/exe", shellPath, 1024); //get the path of the self executable. sth like: /home/Desktop/terminal
    setenv("SHELL",shellPath,1); //Set the SHELL environment variable as self executable that is our bash

    //TOP
    while(1) //To the infinity and beyond!!!
    {  
    	if (feof(stdin)) //ctrl^d
    	{
    		fflush(stdin); //clear standard input
    		fflush(stdout); //clear standard output
    		printf("\nBYE!\n");
        	exit(1); //end process
    	}

    	//Prompt with current working directory	
   		getcwd(pwd, sizeof(pwd)); //get the current working directory (cwd)
   		printf("333sh:%s$ ",pwd); //print cwd with 333sh. prompt will become sth. like this: 333sh:/home/std$

        int argSize = 0; //Set argument counter
        int background = 0; //Set backgorund
        int IO = 0; //For detecting IO operations
        int PIPE = 0; //For detecting PIPE operation

        //Split stdin into tokens
        if(fgets(buffer, 1024, stdin)) //Get command line
        {	
        	//############## HISTORY ARRAY SETTINGS BEGIN ##################
        	
        	int historyProcess = 0; //historyProcess used to understand if it's something like: !n or !! (n is a number)
        	int historyNumber = 0; //the command from the historyArray that is specified with the number: !n
        	int historyLast = 0; //if the command contains (!) then we will get the actual command to place it into historyArray
        	if(strstr(buffer,"!")) //if buffer contains (!)
        	{
        		if(strlen(buffer)>4) //If buffer is something like this: !100, then get the new command
        			continue; //It's more than 4 because of the last '\0' element of the buffer
        		else
        		{
        			//Strip (!) sign from buffer and get the pure command
					char *chur; //temp char pointer to keep tokens
					char temp[120]=""; //temp string to keep the number of the historyArray that is given by the user: !n
					chur = strtok(buffer, "!"); //split buffer from (!)
					while (chur != NULL) 
					{
						strcat(temp,chur);
						chur = strtok(NULL, "!");
					}
					strcpy(temp,rightTrim(leftTrim(temp))); //strip spaces from the begining and the end of the temp
					if(atoi(temp)>0) //It's an integer
					{
						if(0<atoi(temp)&&atoi(temp)<=10) //It should be between [1,10]
						{
							historyProcess = 1; //OK
							historyNumber = atoi(temp); //Got the number: n
							//if a command exist with the given number, then set the buffer to that command
							//so it can be placed into historyArray once again
							if(historyArray[historyNumber-1])
							{
								if(strcmp(historyArray[historyNumber-1]," ")>0)
								{
									strcpy(buffer,historyArray[historyNumber-1]); //the actual command
									printf("[%d].\t%s",historyNumber,buffer);
									historyLast = 1; //we need this to insert the command to historyArray back again
								}
								else
									continue;
							}
							else
								continue;
						}
						else //if it's not an integer, continue to the top to get the new command
							continue;
					}
					else if(!strncmp(buffer, "!!", 2)) //if true, then user is trying to execute the most recent command
					{
						historyProcess = 1; //OK
						historyNumber = 1; //Got the number: n=1
						//if a command exist with #1, then set the buffer to that command
						//so it can be placed into historyArray once again
						if(historyArray[historyNumber-1])
						{
							if(strcmp(historyArray[historyNumber-1]," ")>0)
							{
								strcpy(buffer,historyArray[historyNumber-1]); //the actual command
								printf("[%d].\t%s",historyNumber,buffer);
								historyLast = 1; //we need this to insert the command to historyArray back again
							}
							else
								continue;
						}
						else
							continue;
					}
					else //not a number (!n) or not the (!!) command given
						continue; //go back to the top for a new command
        		}
        	}
        	
        	if(!historyProcess||historyLast) //Trying to insert the command into historyArray
        	{
        		int i, j=0; //some counter variables
	        	for(i=0; i<10; i++) //count the current element numbers of the historyArray
	        	{
	        		if(historyArray[i]) //if exist, increment the counter
	        			++j;
	        	}
	        	if(j==10&&strcmp(buffer," ")>0) //Array is full, we need to shift it for the new element
	        	{
	        		//Kubra's awesome shifting algorithm
	        		char* tempArray[10]; //to keep current historyArray in a temp array
	        		for(i=0;i<10;i++) //start copying historyArray into tempArray
	        		{
		        		tempArray[i] = (char *) malloc(120); //open some space in the memory
		        		strcpy(tempArray[i], historyArray[i]);
	        		}
	        		strcpy(historyArray[0], buffer); //set first element of the historyArray as buffer
	        		for(i=1;i<10;i++) //put back tempArray into historyArray
	        		{
	        			strcpy(historyArray[i], tempArray[i-1]);
	        		}
	        	}
	        	else //historyArray is empty, so set the first element
	        	{
	        		if(!historyArray[0]&&strcmp(buffer," ")>0) //set first element as buffer
	        		{
	        			historyArray[0] = (char *) malloc(120);
	        			strcpy(historyArray[0], buffer);
	        		}
	        		else if(strcmp(buffer," ")>0) //set the other elements on the next time
	        		{
	        			//Kubra's awesome shifting algorithm once again
	        			//After setting the first element we should shift the array
	        			//and add the new element to the top
	        			char* tempArray[j];//to keep current historyArray in a temp array
		        		for(i=0;i<10;i++) //start copying historyArray into tempArray
		        		{
			        		tempArray[i] = (char *) malloc(120); //open some space in the memory
			        		if(historyArray[i]) //check if exist, to avoid from segmentation fault
			        			strcpy(tempArray[i], historyArray[i]);
		        		}
		        		strcpy(historyArray[0], buffer); //set the first element as buffer
		        		for(i=1;i<10;i++)
		        		{
		        			historyArray[i] = (char *) malloc(120);
		        			if(tempArray[i-1]) //check if exist, to avoid from segmentation fault
		        				strcpy(historyArray[i], tempArray[i-1]);
		        		}
	        		}
	        	}        	
        	}
        	else //Trying to execute a command from historyArray
        	{
        		if(historyArray[historyNumber-1])
				{
					if(strcmp(historyArray[historyNumber-1]," ")>0)
					{
						strcpy(buffer,historyArray[historyNumber-1]); //the actual command
						printf("[%d].\t%s",historyNumber,buffer);
					}
					else
						continue;
				}
				else
					continue;
        	}      	

        	//############## HISTORY ARRAY SETTINGS END ##################

        	//Split the command and try to detect IO and PIPE operations
        	strncpy(bufferBackUp, buffer, 1024); //Backup command line
            arg = args;
            *arg++ = strtok(buffer," \t\n");
            while ((*arg++ = strtok(NULL," \t\n")))
            {            	
            	argSize++;
            	if(!strcmp(args[argSize],"<")||!strcmp(args[argSize],">")||!strcmp(args[argSize],">>")) //which means it's an IO
            		IO = 1;
            	else if(!strcmp(args[argSize],"|")) //which means it's a PIPE
            		PIPE = 1;
            }
      	}

//Start cheking

      	if(args[0]!=NULL) //If first argument is not NULL
      	{
      		if(!strcmp(args[argSize],"&")) //If last argument is an &
      		{
				background = 1; //It's a background process
				args[argSize] = NULL; //No need & in argument list
				argSize -=1; //Decrement argument size
      		}

//B. Internal (build-in) commands

//cd
      		if(!strcmp(args[0],"cd")) //Change current working directory
      		{
      			char cwd[1024]; //to keep the current working directory
      			if(args[1]==NULL) //If <directory> is not given print the current working directory
      			{
   					if (getcwd(cwd, sizeof(cwd)) != NULL) //Get current working directory
       					fprintf(stdout, "%s\n", cwd); //Print current working directory
      			}
      			else //Change current working directory
      			{
      				if(chdir(args[1])) //Try to change the current working directory
      					printf("no such directory: %s\n",args[1]); //Failed to change current working directory
      				else //changing the cwd success
      				{
      					if(getcwd(cwd, sizeof(cwd)) != NULL) //Get current working directory
      						setenv("PWD",cwd,1); //Set PWD environment variable as current working directory
      				}
      			}
      		}
//clr
      		else if(!strcmp(args[0],"clr")) //Clear the screen
      		{
      			system("clear"); //Call "clear" system command
      		}
//print
      		else if(!strcmp(args[0],"print")) //Print all of the environment variables or a specific one
      		{
      			if(!args[1]) //If environment variable name not given then print all of them
      				system("env"); //Call env system command
      			else
      			{
      				if(getenv(args[1])) //Get environment variable if exist
      					printf("%s = %s\n",args[1],getenv(args[1])); //Print the value of the given environment variable
      				else
      					printf("no such environment variable: %s\n",args[1]); //If there is no such environment variable
      			}
      		}
//set
      		else if(!strcmp(args[0],"set")) //Try to set an env. var.
      		{
      			if(!args[1]) //If an environment variable name not given
      				printf("usage: set varname = somevalue\n");
      			else
      			{
      				char varAndValue[1024]=""; //To keep variable name and value
      				int i = 1;
      				while(args[i]) //Concatenate arguments after the 0th
      				{
      					strcat(varAndValue,args[i]);
      					i++;
      				}
      				if(!strstr(varAndValue,"=")) //If varAndValue does not contain an = sign
      					printf("usage: set varname = somevalue\n");
      				else
      				{
      					char varname[1024]=""; //to keep variable name
      					char somevalue[1024]=""; //to keep value of the variable
      					char *ch; //a char pointer
      					ch = strtok(varAndValue, "="); //Split arguments from = sign
      					i = 0;
						while (ch != NULL) 
						{
							if(i==0) //First one is variable name
								strcat(varname,ch);
							else //Second one is variable value
								strcat(somevalue,ch);
							i++;
							ch = strtok(NULL, "=");
						}
						if(getenv(varname)) //Try to get environment variable with given name
						{
      						setenv(varname,somevalue,1); //Overwrite it with new value
						}
	      				else
	      					printf("no such environment variable: %s\n",varname); //No such environment variable	
					}
				}
      		}
//where
      		else if(!strcmp(args[0],"where")) //Find the path of the given command
      		{
      			if(args[1]) //if the command name is given sth. like: where ls
      			{
      				char *pathArray = getenv("PATH"); //Get the PATH environment variable
      				char pathBackUp[1024];
      				strcpy(pathBackUp,getenv("PATH")); //Backup the PATH
					if(pathArray!=NULL) //Split the PATH from (:)
					{
					   	char *pathPiece;
						char dummyPath[1024];
						struct stat st;
						pathPiece = strtok(pathArray,":");
						while (pathPiece != NULL)
						{
							strcat(dummyPath,pathPiece); //get a piece of PATH from (:) sth like: /usr/bin
							strcat(dummyPath,"/"); //cat a slash end of it: /usr/bin/
							strcat(dummyPath,args[1]); //cat the command end of it: /usr/bin/gedit
							if(stat(dummyPath, &st)==-1) //an executable not found, set dummyPath empty
								strcpy(dummyPath,"");
							else //an executable is found with the given command, break the for loop
								break;
							pathPiece = strtok (NULL, ":");
						}				
						if(strlen(dummyPath)>0) //print the path of the command
							printf("%s\n", dummyPath);
						else
						{
							//If not found in PATH then search it in the current working directory
							char *currentPath = getcwd(pwd, sizeof(pwd)); //get current working directory
							strcat(currentPath,"/");
							strcat(currentPath,args[1]);
							if (!access(currentPath, X_OK)) //test if it's an executable
							{
							   printf("%s\n",currentPath); //print the path of the command
							}
							else //it's not an executable
							{
							   printf("command not found: %s\n", args[1]);
							}
						}
						setenv("PATH",pathBackUp,1); //Set PATH back
					}
      			}
      			else //a command is not given right after "where"
      			{
      				printf("usage: where <command>\n");
      			}
      		}
//path
      		else if(!strcmp(args[0],"path")) //print PATH env. var. or append/remove some given path to it
      		{
      			if(!args[1]) //Get PATH environment variable
      				printf("PATH = %s\n",getenv("PATH")); //print PATH
      			else if(!strcmp(args[1],"+")) //Trying to append some path to it
      			{
      				if(!args[2]) //if a value is not given
      					printf("usage: path + <value>\n");
      				else //if a value is given
      				{
      					char dummyPath[1024]; //to keep PATH
      					strcpy(dummyPath,getenv("PATH")); //get PATH value to dummyPath
      					strcat(dummyPath,":"); //cat (:) to dummyPath
      					strcat(dummyPath,args[2]); //cat new value to dummyPath
      					setenv("PATH",dummyPath,1); //overwrite dummyPath to PATH
      					printf("PATH = %s\n",getenv("PATH")); //print new PATH
      				}
      			}
      			else if (!strcmp(args[1],"-")) //Trying to remove some path to it
      			{
      				if(!args[2]) //if a value is not given
      					printf("usage: path - <value>\n");
      				else //if a value is given
      				{
      					char dummyPath[1024]; //to keep PATH
      					char newPath[1024]=""; //to keep the new value of the PATH
      					strcpy(dummyPath,getenv("PATH")); //get PATH value to dummyPath
						char *ch; //temp char pointer
						ch = strtok(dummyPath, ":"); //split dummyPath from (:)
						while (ch != NULL) 
						{
							if(strcmp(ch,args[2])) //if PATH piece is not equal to the given path value, cat it to the newPath
							{
								strcat(newPath,ch); //cat the PATH piece that is not equal to given path value
								strcat(newPath,":"); //cat a (:)
							}
							ch = strtok(NULL, ":");
						}
						newPath[strlen(newPath)-1] = 0; //remove the (:) sign from at the end of the PATH
						setenv("PATH",newPath,1); //overwrite newPath to PATH env. var.
						printf("PATH = %s\n",getenv("PATH")); //print new PATH value
						strcpy(newPath,""); //unset newPath
						strcpy(dummyPath,""); //unset dummyPath
      				}
      			}
      			else //not an appropriate process 
      			{
      				printf("usage: path +- <value>\n");
      			}
      		}
//exit
      		else if(!strcmp(args[0],"exit")) //try to exit
      		{
      			if(backgorundSize > 0) //if there is/are background process(es)
      			{
      				if(backgorundSize == 1)
      					printf("there is %d processes is running.\n",backgorundSize);
      				else
      					printf("there are %d processes are running.\n",backgorundSize);
      			}
      			else //if there is no background process, exit
      			{
      				printf("BYE!\n");
      				exit(1);
      			}
      		}
//CTRLC
      		else if(!strcmp(args[0],"CTRLC"))
      		{
      			//This one is not needed since we handled ctrl^c by using signal
      			//See the function: void ctrlcSignal(int sgnl);
      		}

//C. I/O Redirection

      		else if((args[1]!=NULL)&&(IO)) //I/O Redirection
      		{
      			/*
      				Possibilities
					command < fileIn 				 -> option 1 <
					command > fileOut				 -> option 2 >
					command >> fileOut				 -> option 3 >>
					command < fileIn > fileOut 		 -> option 4 <>
					command < fileIn >> fileOut 	 -> option 5 <>>
      			*/

				//Since IO is detected at the top, we need args[2] at minimum possiblity such as: command > fileIn
				if(!args[2]) // If fileIn is not given, go back to top to get the new command
					continue;

				char option[1024]; //to keep the option
				char commandLine[1024]; //same as the buffer but without (<) or (>) sign
				int i = 0; //some counter variable

				/*
					Determine the option and get the full commandLine
					command < fileIn >> fileOut
					commandLine = command < fileIn >> fileOut
					option = <>>
				*/
				while(args[i]!=NULL)
				{
					strcat(commandLine,args[i]);
					strcat(commandLine," ");
					if(!strcmp(args[i],">")||!strcmp(args[i],">>")||!strcmp(args[i],"<"))
						strcat(option,args[i]);
					i++;
				}

				/*
					Split commandLine into "parts" array from (<) and (>) sign
					commandLine = command < fileIn >> fileOut
					parts[0] = command
					parts[1] = fileIn
					parts[2] = fileOut
				*/
				char *parts[120];
    			char **part;
    			part = parts;
	            *part++ = strtok(commandLine,"<>");
	            while ((*part++ = strtok(NULL,"<>")));

	            //there may be spaces at the begining and end of the fileIn and fileOut
	            //so we need to trim them
	            char command[1024]; //to keep the pure command
	            char file1[1024]; //to keep the fileIn STDIN_FILENO
	            char file2[1024]; //to keep the fileOut STDOUT_FILENO
	            if(parts[0]) //copy command
	            	strcpy(command,parts[0]);
	            if(parts[1]) //copy fileIn
	            	strcpy(file1,rightTrim(leftTrim(parts[1]))); //Trim file1 from right and left
	            if(parts[2]) //copy fileOut
	            	strcpy(file2,rightTrim(leftTrim(parts[2]))); //Trim file2 from right and left

	            //Split command into commandParts array from spaces/tabs
	            char *commandParts[120];
    			char **commandPart;
    			commandPart = commandParts;
	            *commandPart++ = strtok(command," \t\n");
	            while ((*commandPart++ = strtok(NULL," \t\n")));
	            /*
	            	dup2() function arguments:
					STDIN_FILENO: Standard input value, stdin. Its value is 0. 
					STDOUT_FILENO: Standard output value, stdout. Its value is 1. 
					STDERR_FILENO: Standard error value, stderr. Its value is 2. 
					open() function arguments:
					O_RDONLY: Open for reading only.
					O_TRUNC: If the file exists and is a regular file, and the file is successfully opened O_RDWR or O_WRONLY, 
					its length shall be truncated to 0, and the mode and owner shall be unchanged.
					O_CREAT: The file shall be created
					O_RDWR: Open for reading and writing. The result is undefined if this flag is applied to a FIFO.
					O_WRONLY: Open for writing only.
					O_APPEND: If set, the file offset shall be set to the end of the file prior to each write. 
	            */
	            int status; 
	            switch (childID=fork()) //fork a child process
	            { 
	              case 0: //Child process
		            if(!strcmp(option,"<")) //command < fileIn
					{
						int fileDescriptor; //fileIn descriptor
			            if ((fileDescriptor = open(file1, O_RDONLY, 0)) < 0) //try to open fileIn
			            {
			              printf("no such file %s\n",file1); //fileIn not found
			              strcpy(file1,""); //unset fileIn
			              strcpy(option,""); //unset option
			              exit(1); //end child process
			            }
			            dup2(fileDescriptor, STDIN_FILENO); //fileIn becomes the standard input
			            char* dummyPath = execvPath(commandParts[0]); //find the path of the command
			            //execv(const char *path, char *const argv[]);
		                execv(dummyPath, commandParts); //try to execute the command
		                perror(commandParts[0]); //execv failed, command not found or something else
						strcpy(option,""); //unset option
						exit(1); //end child process
					}
					else if(!strcmp(option,">")) //command > fileOut
					{
						int fileDescriptor; //fileOut descriptor
			            if ((fileDescriptor = open(file1, O_TRUNC|O_CREAT|O_RDWR, 0666)) < 0) //try to open fileOut
			            {
			              printf("no such file %s\n",file1); //fileOut not found
			              strcpy(file1,""); //unset fileOut
			              strcpy(option,""); //unset option
			              exit(1); //end child process
			            }
			            dup2(fileDescriptor, STDOUT_FILENO); //fileOut becomes the standard output
		                char* dummyPath = execvPath(commandParts[0]); //find the path of the command
		                //execv(const char *path, char *const argv[]);
		                execv(dummyPath, commandParts); //try to execute the command
		                perror(commandParts[0]); //execv failed, command not found or something else
		                strcpy(option,""); //unset option
		                exit(1); //end child process
					}
					else if(!strcmp(option,">>")) //command >> fileOut
					{
						int fileDescriptor; //fileOut descriptor
			            if ((fileDescriptor = open(file1, O_CREAT|O_WRONLY|O_APPEND, 0666)) < 0) //try to open fileOut
			            {
			              printf("no such file %s\n",file1); //fileOut not found
			              strcpy(file1,""); //unset fileOut
			              strcpy(option,""); //unset option
			              exit(1); //end child process
			            }
			            dup2(fileDescriptor, STDOUT_FILENO); //fileOut becomes the standard output 
		                char* dummyPath = execvPath(commandParts[0]); //find the path of the command
		                //execv(const char *path, char *const argv[]);
		                execv(dummyPath, commandParts); //try to execute the command
		                perror(commandParts[0]); //execv failed, command not found or something else
						strcpy(option,""); //unset option
						exit(1); //end child process
					}
					else if(!strcmp(option,"<>")) //command < fileIn > fileOut
					{
						int fileDescriptor1, fileDescriptor2; //fileOut descriptors
			            if ((fileDescriptor1 = open(file1, O_RDONLY, 0)) < 0) //try to open fileIn
			            {
			              printf("no such file %s\n",file1); //fileIn not found
			              strcpy(file1,""); //unset fileIn
			              strcpy(option,""); //unset option
			              exit(1); //end child process
			            }
			            if ((fileDescriptor2 = open(file2, O_TRUNC|O_CREAT|O_RDWR, 0666)) < 0) //try to open fileOut
			            {
			              printf("no such file %s\n",file2); //fileOut not found
			              strcpy(file2,""); //unset fileOut
			              strcpy(option,""); //unset option
			              exit(1); //end child process
			            }
			            dup2(fileDescriptor1, STDIN_FILENO); //fileIn becomes standard input
			            dup2(fileDescriptor2, STDOUT_FILENO); //fileOut becomes standard output
		                char* dummyPath = execvPath(commandParts[0]); //find the path of the command
		                //execv(const char *path, char *const argv[]);
		                execv(dummyPath, commandParts); //try to execute the command
		                perror(commandParts[0]); //execv failed, command not found or something else
						strcpy(option,""); //unset option
						exit(1); //end child process
					}
					else if(!strcmp(option,"<>>")) //command < fileIn >> fileOut
					{
						int fileDescriptor1, fileDescriptor2; //fileOut descriptors
			            if ((fileDescriptor1 = open(file1, O_RDONLY, 0)) < 0) //try to open fileIn
			            {
			              printf("no such file %s\n",file1); //fileIn not found
			              strcpy(file1,""); //unset fileIn
			              strcpy(option,""); //unset option
			              exit(1); //end child process
			            }
			            if ((fileDescriptor2 = open(file2, O_CREAT|O_WRONLY|O_APPEND, 0666)) < 0) //try to open fileOut
			            {
			              printf("no such file %s\n",file2); //fileOut not found
			              strcpy(file2,""); //unset fileOut
			              strcpy(option,""); //unset option
			              exit(1); //end child process
			            }
			            dup2(fileDescriptor1, STDIN_FILENO); //fileIn becomes standard input 
			            dup2(fileDescriptor2, STDOUT_FILENO); //fileOut becomes standard output
		                char* dummyPath = execvPath(commandParts[0]); //find the path of the command
		                //execv(const char *path, char *const argv[]);
		                execv(dummyPath, commandParts); //try to execute the command
		                perror(commandParts[0]); //execv failed, command not found or something else         
						strcpy(option,""); //unset option
						exit(1); //end child process
					}
					else //Other possibilities not accepted
					{
						printf("not possible: %s\n",commandLine);
						exit(1); //end child process
					}
	                break;            
	              default: //Parent process
	              	//WUNTRACED: return status information for a specified process that has either stopped or terminated.
	                waitpid(childID, &status, WUNTRACED);
	                break; 
	              case -1: 
	                perror("fork failed!\n"); //failed to create a child process
	                break;
	            }

				strcpy(commandLine,""); //unset commandLine
				strcpy(option,""); //unset option
      		}

//D. PIPE
			else if((args[1]!=NULL)&&(PIPE)) //PIPE
			{
				/*
	            	dup2() function arguments:
					STDIN_FILENO: Standard input value, stdin. Its value is 0. 
					STDOUT_FILENO: Standard output value, stdout. Its value is 1. 
					STDERR_FILENO: Standard error value, stderr. Its value is 2. 
				*/

				//if the second command is not given at minimum possibility such as: sort -r file |
				if(!args[2])
					continue; //go back to the top for the new command

				if((childID=fork())==0) //create a child process
				{									
					int pipeArray[2]; //descriptors array
					
					int i = 0; //some counter variable
					char commandLine[1024]=""; //to keep the full command line

					//Get the command line
					while(args[i]!=NULL)
					{
						strcat(commandLine,args[i]); //cat argument
						strcat(commandLine," "); //cat a space
						i++;
					}
					/*
						Assume that:
						commandLine = ls -a -l / | wc -l
					*/

					//Seperate commandLine into tokens from (|)
					char *commandParts[1024];
					char **commandPart;
					commandPart = commandParts;
		            *commandPart++ = strtok(commandLine,"|");
		            while ((*commandPart++ = strtok(NULL,"|")));
		            /*
						commandParts[0] = ls -a -l /
						commandParts[1] = wc -l
					*/

		            //First command part (the left side of the pipe)
		            char *firstParts[1024];
					char **firstPart;
					firstPart = firstParts;
		            *firstPart++ = strtok(commandParts[0]," \t\n");
		            while ((*firstPart++ = strtok(NULL," \t\n")));
		            /*
						firstParts[0] = ls
						firstParts[1] = -a
						firstParts[2] = -l
						firstParts[3] = /
					*/

		            //Second command part (the right side of the pipe)
		            char *secondParts[1024];
					char **secondPart;
					secondPart = secondParts;
		            *secondPart++ = strtok(commandParts[1]," \t\n");
		            while ((*secondPart++ = strtok(NULL," \t\n")));
		            /*
						secondParts[0] = wc
						secondParts[1] = -l
					*/

					pipe(pipeArray); //create a pipe and place two descriptors
					
					if(fork() == 0)
					{
						dup2(pipeArray[0], STDIN_FILENO); //pipeArray[0] becomes standard input part of the pipe
						close(pipeArray[1]); //close pipeArray[1]
						char* dummyPath = execvPath(secondParts[0]); //get the path of the command
						//execv(const char *path, char *const argv[]);
						execv(dummyPath, secondParts); //try to execute the command
						perror(secondParts[0]); //execv failed, command is not found or something else
						exit(1); //end child process
					}
					else
					{
						dup2(pipeArray[1], STDOUT_FILENO); //pipeArray[1] becomes standard output part of the pipe
						close(pipeArray[0]); //close pipeArray[0]
						char* dummyPath = execvPath(firstParts[0]); //get the path of the command
						//execv(const char *path, char *const argv[]);
						execv(dummyPath, firstParts); //try to execute the command
						perror(firstParts[0]); //execv failed, command is not found or something else
						exit(1); //end child process
					}
					exit(1);
				}
				else //parent process
				{
					usleep(20000); //Wait for 20 milliseconds to prompt
					//WNOHANG: return status information immediately, without waiting for the specified process to terminate.
					waitpid(childID, &status, WNOHANG); //wait for the child process with ID of childID
					usleep(20000); //Wait for 20 milliseconds to prompt
					continue; //go to top for the new command
				}
				PIPE = 0; //unset PIPE
			}

//A. System commands

      		else //System commands
      		{      			
				if ((childID = fork()) == -1) //create a child process
	      		{  
		        	exit(EXIT_FAILURE); //failed to create a child process
		     	}
		     	else if(childID == 0) //child process
		     	{				    
				    char filePath[1024]; //to keep the path of the command
				    strcpy(filePath,args[argSize]); //the last argument is the command path
				    int exl = 0; //to determine if the command will be executed with execl or execv
					char *ch; //a char pointer

					/*
						Assume that:
						command line = ls -a -l /bin/ls -> will be executed with execl
						command line = ls -a -l -> will be executed with execv
						command line = ls -a -l /home/somefile -> will be executed with execv
					*/

					//try to determine the appropriate exec function: execl or execv
					if(strstr(filePath, "/") != NULL) //if the last argument contains a slash
					{
						ch = strtok(filePath, "/");
						while (ch != NULL) 
						{
							if(!strcmp(args[0],ch)) //if last part of the last argument contains the same program name
							{
								exl = 1; //it's an execl process
								break;
							}
							ch = strtok(NULL, "/");
						}
					}
					strcpy(filePath,args[argSize]); //filePath is broken at the above while loop. restore it...

				    if(exl) //It's an execl process
				    {
				    	args[argSize]=NULL; //not needed the last element of the args since we copied it into filePath
				    	exl = 0; //unset exl
				    	//execl(const char *path, const char *arg0, ..., const char *argn, (char *)0);
				    	execl(filePath, args[0], args[1], args[2], args[3], args[4], args[5]
									  , args[6], args[7], args[8], args[9], args[10], args[11]
									  , args[12], args[13], args[14], args[15], args[16], args[17]
									  , args[18], args[19], args[20], args[21], args[22], args[23]
									  , args[24], args[25], args[26], args[27], args[28], args[29]
									  , args[30], args[31], NULL);
				    	perror(args[0]); //execl failed, command is not found or something else
				    }
				    else //It's a command, we should find the path to that command
				    {
				    	char* dummyPath = execvPath(args[0]); //try to get the path of the command
				    	//execv(const char *path, char *const argv[]);
						execv(dummyPath, args); //try to execute the command
						perror(args[0]); //execv failed, command not found or something else
				    }
        			exit(1); //end child process
		     	}
		     	else //Parent process. to wait or not to wait...
		     	{   
		     		if(background) //Don't wait it's a BACKGROUND process
					{
						backgorundSize++; //increment the background counter
						usleep(10000); //Wait for 10 milliseconds to prompt
					}
					else //Wait, it's a FOREGROUND process
					{
						usleep(20000); //Wait for 20 milliseconds to prompt
						//WUNTRACED: return status information for a specified process that has either stopped or terminated.
						waitpid(childID, &status, WUNTRACED); //wait for the child process with ID of childID
						usleep(20000); //Wait for 20 milliseconds to prompt
					}
		        }
	     	}
      	}
    }
}

//Functions

//needed to find the command path which will be executed with execv
char* execvPath(char* argument)
{
	char *pathArray = getenv("PATH"); //Get PATH environment variable
    char pathBackUp[1024]; //used to backup PATH value
    strcpy(pathBackUp,getenv("PATH")); //backup PATH
    char dummyPath[1024]; //to keep the PATH of the command if it's exist
	if(pathArray!=NULL) //Split the PATH from (:)
	{
	   	char *pathPiece; //a char pointer to point each PATH parts
		struct stat st; //to understand if the command file is exist or not in the path
		pathPiece = strtok(pathArray,":");
		while (pathPiece != NULL)
		{
			strcat(dummyPath,pathPiece); //cat a PATH piece: /bin
			strcat(dummyPath,"/"); //cat a slash to end of it: /bin/
			strcat(dummyPath,argument); //cat the argument /bin/ls
			if(stat(dummyPath, &st)==-1) //on error, -1 is returned, which means command file is not exist
				strcpy(dummyPath,""); //command not found, unset dummyPath for next PATH piece
			else //command file is found, no need to continue
				break;
			pathPiece = strtok (NULL, ":");
		}
		setenv("PATH",pathBackUp,1); //the original PATH value is broken at above while loop. restore it..
	}
	char *str = (char *) malloc(120); //open some memory area and point it with the str char pointer
	strcpy(str,dummyPath); //copy the command path to the memory
	return str; //return the command file path
}

//trim the space at the beginning of the string
char* leftTrim(char *str)
{
	char* original = str;
	char *p = original;
	int trimmed = 0;
	do
	{
		if (*original != ' ' || trimmed)
		{
			trimmed = 1;
			*p++ = *original;
		}
	}
	while (*original++ != '\0');
	return str;
}

//trim the space at the end of the string
char* rightTrim(char* str)
{
	char* original = str + strlen(str);
	while(*--original == ' ');
		*(original + 1) = '\0';
	return str;
}

//triggered when a process is terminated
void exitSignal(int sgnl)
{
	int pid, status;
	//WNOHANG: return status information immediately, without waiting for the specified process to terminate.
	//WAIT_ANY: specifies that waitpid should return status information about any child process. its value is -1
	pid = waitpid (WAIT_ANY, &status, WNOHANG);
	if(pid > 0) //a background process is terminated
	{
		if(backgorundSize > 0) //do not fall into negative values
			backgorundSize--; //decrement bacground process counter
	}//do nothing for the foreground processes

	signal(SIGCHLD, exitSignal);
}

//triggered when the user press ctrl^c
void ctrlcSignal(int sgnl)
{
	int i; //some counter variable
	printf("\nhistory of commands:");
    for(i=0;i<10;i++)
    {
    	if(historyArray[i]) //if a command is exist in historyArray with given index
       	{
       		if(i==0 && strcmp(historyArray[i]," ")>0) //print the historyArray with a new line at first time
        		printf("\n[%d].\t%s", i+1, historyArray[i]);
        	else if(strcmp(historyArray[i]," ")>0) //no need a new line
        		printf("[%d].\t%s", i+1, historyArray[i]);
       	}
    }
	char pwd[1024]; //to keep the current working directory
    getcwd(pwd, sizeof(pwd)); //get the current working directory
    if(!historyArray[0]) //if historyArray is empty which means there is no command previously issued
    {
    	printf(" history is empty");
   		printf("\n333sh:%s$ ",pwd); //prompt with current working directory with a new line
    }
   	else
   		printf("333sh:%s$ ",pwd); //prompt with current working directory
   	fflush(stdout); //clear standard output
    signal(SIGINT, ctrlcSignal);
}
