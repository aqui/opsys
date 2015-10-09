/**
    Akif Batur - 150111854 - akfbtr@gmail.com
    CSE 333 - OPERATING SYSTEMS
    Programming Assignment # 3
    Marmara University - Computer Engineering
    Copyleft - 2014
**/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

void errorCheck(int argc, char *argv[]); //Check arguments
void fileCheck(int argc, char *argv[]); //Check files
void merge(int firstArray[], int firstSize, int secondArray[], int secondSize, int mergedArray[]); //Merge function
void readAndMergeBuffers(); //Read buffers and merge
void readAndMergeFiles(int threadId); //Read files and merge them into buffers

pthread_mutex_t rw_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex for writer threads
pthread_mutex_t reader_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex for reader threads
int num_readers = 0; //number of reader threads

struct node //linked list nodes for buffers
{
    int data;
    struct node* next;
};

struct node* buffer1 = NULL; //buffer
struct node* buffer2 = NULL; //buffer
struct node* buffer3 = NULL; //buffer
struct node* buffer4 = NULL; //buffer
struct node* buffer5 = NULL; //buffer
struct node* buffer6 = NULL; //buffer
struct node* outputBuffer = NULL; //not in use

char outputfile[256]; //output file name
int stopRead = 0; //to decide if all of the reader threads are finished or not

//Threads
int thread1 = 0; //writer thread 1
int thread2 = 0; //writer thread 2
int thread3 = 0; //writer thread 3
int thread4 = 0; //writer thread 4
int thread5 = 0; //reader thread 1 -> waits for thread1 and thread2
int thread6 = 0; //reader thread 2 -> waits for thread3 and thread4
int thread7 = 0; //reader thread 3 -> waits for thread5 and thread6

//File names
char file1[256],file2[256],file3[256],file4[256],file5[256],file6[256],file7[256],file8[256];

void insert(struct node** head_ref, int num) //insert number to a specified buffer
{
    int c=0;
    struct node *temp;
    temp=(*head_ref);
    if(temp==NULL) //if head is null create new node
    {
        struct node *temp;
        temp=(struct node *)malloc(sizeof(struct node));
        temp->data=num;
        if ((*head_ref)== NULL)
        {
        (*head_ref)=temp;
        (*head_ref)->next=NULL;
        }
    }
    else //if head is not null then go to end  of the list then create a new node and connect it
    {
        struct node *temp,*right;
        temp= (struct node *)malloc(sizeof(struct node));
        temp->data=num;
        right=(struct node *)(*head_ref);
        while(right->next != NULL) //go to end of the linked list
        	right=right->next;
        right->next =temp;
        right=temp;
        right->next=NULL;
    }
}

int main(int argc, char *argv[])
{
	errorCheck(argc, argv); //Check if there is an error on usage
	fileCheck(argc, argv); //Check files
	void *reader(); //Readers
	void *writer(); //Writers

	if(argc==7) // ./merge -n 2 file1 file2 -o outputfile -> argc = 7
	{
		stopRead = 2; //stop and write buffer to output file
		//get file names
		strcpy(outputfile,argv[6]);
		strcpy(file1,argv[3]);
		strcpy(file2,argv[4]);
		//Threads
		pthread_t writer_thread1;
		//Create threads
		pthread_create(&writer_thread1,NULL,writer,(void *)1);
		//Join threads
		pthread_join(writer_thread1,NULL);
	}
	if(argc==9) // ./merge -n 4 file1 file2 file3 file4 -o outputfile -> argc = 9
	{
		stopRead = 4; //stop and write buffer to output file
		//get file names
		strcpy(outputfile,argv[8]);
		strcpy(file1,argv[3]);
		strcpy(file2,argv[4]);
		strcpy(file3,argv[5]);
		strcpy(file4,argv[6]);
		//Thread
		pthread_t writer_thread1;
		pthread_t writer_thread2;
		pthread_t reader_thread1;
		//Create threads
		pthread_create(&writer_thread1,NULL,writer,(void *)1);
		pthread_create(&writer_thread2,NULL,writer,(void *)2);
		pthread_create(&reader_thread1,NULL,reader,(void *)1);
		//Join threads
		pthread_join(writer_thread1,NULL);
		pthread_join(writer_thread2,NULL);
		pthread_join(reader_thread1,NULL);
	}
	if(argc==13) // ./merge -n 8 file1 file2 file3 file4 file5 file6 file7 file8 -o outputfile -> argc = 13
	{
		//get file names
		strcpy(outputfile,argv[12]);
		strcpy(file1,argv[3]);
		strcpy(file2,argv[4]);
		strcpy(file3,argv[5]);
		strcpy(file4,argv[6]);
		strcpy(file5,argv[7]);
		strcpy(file6,argv[8]);
		strcpy(file7,argv[9]);
		strcpy(file8,argv[10]);

		//Threads
		pthread_t writer_thread1;
		pthread_t writer_thread2;
		pthread_t writer_thread3;
		pthread_t writer_thread4;
		pthread_t reader_thread1;
		pthread_t reader_thread2;
		pthread_t reader_thread3;
		//Create threads
		pthread_create(&writer_thread1,NULL,writer,(void *)1);
		pthread_create(&writer_thread2,NULL,writer,(void *)2);
		pthread_create(&writer_thread3,NULL,writer,(void *)3);
		pthread_create(&writer_thread4,NULL,writer,(void *)4);
		pthread_create(&reader_thread1,NULL,reader,(void *)1);
		pthread_create(&reader_thread2,NULL,reader,(void *)2);
		pthread_create(&reader_thread3,NULL,reader,(void *)3);
		//Join threads
		pthread_join(writer_thread1,NULL);
		pthread_join(writer_thread2,NULL);
		pthread_join(writer_thread3,NULL);
		pthread_join(writer_thread4,NULL);
		pthread_join(reader_thread1,NULL);
		pthread_join(reader_thread2,NULL);
		pthread_join(reader_thread3,NULL);
	}
}

int count(struct node** head_ref) //count nodes on a buffer
{
    struct node *temp;
    int counter = 0;
    temp=(*head_ref);
    while(temp!=NULL)
    {
    	counter++;
	    temp=temp->next;
    }
    return counter;
}

void readAndMergeBuffers(int threadId) //read bufferX & bufferY and merge them to a bufferZ
{
	struct node** head_ref;

	if(threadId==1) //if it's the first reader thread
	{
		struct node *temp;
		int firstSize = count(&buffer1); //count nodes on buffer1
		int secondSize = count(&buffer2); //count nodes on buffer2
		//create arrays to keep the elements of the buffer1 and buffer2
		int firstArray[firstSize];
		int secondArray[secondSize];
		int i = 0; //some counter
		head_ref = &buffer1;
		temp=(*head_ref);
		while(temp!=NULL) //write numbers to first array
		{
			firstArray[i] = temp->data;
			temp = temp->next;
			i++;
		}
		i = 0;
		head_ref = &buffer2;
		temp=(*head_ref);
		while(temp!=NULL) //write numbers to second array
		{
			secondArray[i] = temp->data;
			temp = temp->next;
			i++;
		}
		int mergedArray[firstSize+secondSize]; //create an array to keep the merged numbers
		merge(firstArray,firstSize,secondArray,secondSize,mergedArray); //merge buffer1 and buffer2

		for(i=0;i<(firstSize+secondSize);i++) //write merged file into buffer5
		{
			insert(&buffer5, mergedArray[i]);
		}

		if(stopRead==4) //We can stop here for 4 files
		{
			struct node** head_ref = &buffer5;
			FILE *out = fopen(outputfile, "w");
			while((*head_ref)!=NULL) //write elements of the buffer5 into output file
			{
				fprintf(out, "%d\n",(*head_ref)->data);
				(*head_ref) = (*head_ref)->next;
			}
		}
	}
	if(threadId==2) //if it's reader thread 2
	{
		struct node *temp;
		int firstSize = count(&buffer3); //get size of buffer3
		int secondSize = count(&buffer4); //get size of buffer4
		int firstArray[firstSize]; //create array for elements of the buffer3
		int secondArray[secondSize]; //create array for elements of the buffer4
		int i = 0;
		head_ref = &buffer3;
		temp=(*head_ref);
		while(temp!=NULL) //write elements of the buffer3 to array
		{
			firstArray[i] = temp->data;
			temp = temp->next;
			i++;
		}
		i = 0;
		head_ref = &buffer4;
		temp=(*head_ref);
		while(temp!=NULL) //write elements of the buffer4 to array
		{
			secondArray[i] = temp->data;
			temp = temp->next;
			i++;
		}
		int mergedArray[firstSize+secondSize]; //create an array for merged elements
		merge(firstArray,firstSize,secondArray,secondSize,mergedArray); //merge buffer3 and buffer4

		for(i=0;i<(firstSize+secondSize);i++) //write merged elements to buffer6
		{
			insert(&buffer6, mergedArray[i]);
		}
	}
	if(threadId==3) //if it's reader thread 3
	{
		struct node *temp;
		int firstSize = count(&buffer5); //count size of buffer5
		int secondSize = count(&buffer6); //count size of buffer6
		int firstArray[firstSize];
		int secondArray[secondSize];
		int i = 0;
		head_ref = &buffer5;
		temp=(*head_ref);
		while(temp!=NULL) //get elements of the buffer5 into array
		{
			firstArray[i] = temp->data;
			temp = temp->next;
			i++;
		}
		i = 0;
		head_ref = &buffer6;
		temp=(*head_ref);
		while(temp!=NULL) //get elements of the buffer6 into array
		{
			secondArray[i] = temp->data;
			temp = temp->next;
			i++;
		}
		int mergedArray[firstSize+secondSize];
		merge(firstArray,firstSize,secondArray,secondSize,mergedArray); //merge arrays
		FILE *out = fopen(outputfile, "w");
		for(i=0;i<(firstSize+secondSize);i++) //write merged elements into output file
		{
			fprintf(out, "%d\n",mergedArray[i]);
		}
		fclose(out);
	}
}

void readAndMergeFiles(int threadId) //read fileX and fileY and merge them into a buffer
{
	char dummyFile1[256];
	char dummyFile2[256];

	if(threadId==1) //if writer thread 1 then merge first and second files
	{
		strcpy(dummyFile1,file1);
		strcpy(dummyFile2,file2);
	}
	else if(threadId==2) //if writer thread 2 then merge third and fourth files
	{
		strcpy(dummyFile1,file3);
		strcpy(dummyFile2,file4);
	}
	else if(threadId==3) //if writer thread 3 then merge fifth and sixth files
	{
		strcpy(dummyFile1,file5);
		strcpy(dummyFile2,file6);
	}
	else if(threadId==4) //if writer thread 4 then merge seventh and eighth files
	{
		strcpy(dummyFile1,file7);
		strcpy(dummyFile2,file8);
	}

	char number[256];
	int firstSize;
	int secondSize;

	FILE *file1Ptr;
	FILE *file2Ptr;
	int fileSize;

	file1Ptr = fopen(dummyFile1, "r"); //open file1
	fseek(file1Ptr, 0L, SEEK_END); //seek end of the file
	fileSize = ftell(file1Ptr); //get size of the file
	if(fileSize==0) //if file is empty then exit
	{
		printf("%s is empty. please fix it.\n",dummyFile1);
		exit(1);
	}
	fseek(file1Ptr, 0L, SEEK_SET);
  	while(fgets(number, 256, file1Ptr)!=NULL) //get first line of the file1
  	{
  		firstSize = atoi(number);
  		if(firstSize<0) //if first item of the file1 is less than zero then exit
  		{
			printf("number of items less than zero in %s. please fix it.\n",dummyFile1);
			exit(1);
  		}
  		break;
  	}
	fclose(file1Ptr);
	file2Ptr = fopen(dummyFile2, "r"); //open file2
	fseek(file2Ptr, 0L, SEEK_END); //seek end of the file
	fileSize = ftell(file2Ptr); //get size of the file
	if(fileSize==0) //if file is empty then exit
	{
		printf("%s is empty. please fix it.\n",dummyFile2);
		exit(1);
	}
	fseek(file1Ptr, 0L, SEEK_SET);
	while(fgets(number, 256, file2Ptr)!=NULL) //get first line of the file2
  	{
  		secondSize = atoi(number);
  		if(secondSize<0) //if first item of the file2 is less than zero then exit
  		{
			printf("number of items less than zero in %s. please fix it.\n",dummyFile2);
			exit(1);
  		}
  		break;
  	}
	fclose(file2Ptr);
	//create arrays
	int firstArray[firstSize];
	int secondArray[secondSize];
	int mergedArray[firstSize+secondSize];

	int i = 0;
	int j = 0;
	file1Ptr = fopen(dummyFile1, "r");
  	while(fgets(number, 256, file1Ptr)!=NULL) //read elements of the file1
  	{
  		if(i==0)
  		{
  			i++;
  			continue;
  		}
  		if(strcmp(number," ")==-1) //if there is an empty line then exit
  		{
  			printf("there is an empty line in %s\n",dummyFile1);
  			exit(1);
  		}
  		firstArray[j] = atoi(number); //add element into first array
  		i++;
  		j++;
  	}
  	if(firstSize!=i-1) //if first line is not equal to the number of items then exit
  	{
  		printf("number of items not equal in %s. please fix it.\n",dummyFile1);
  		exit(1);
  	}
	fclose(file1Ptr);
	j = 0;
	i = 0;
	file2Ptr = fopen(dummyFile2, "r");
	while(fgets(number, 256, file2Ptr)!=NULL) //read elements of the file2
  	{
  		if(i==0)
  		{
  			i++;
  			continue;
  		}
  		if(strcmp(number," ")==-1) //if there is an empty line then exit
  		{
  			printf("there is an empty line in %s\n",dummyFile1);
  			exit(1);
  		}
  		secondArray[j] = atoi(number); //add element into second array
  		i++;
  		j++;
  	}
  	if(secondSize!=i-1) //if first line is not equal to the number of items then exit
  	{
  		printf("number of items not equal in %s. please fix it.\n",dummyFile2);
  		exit(1);
  	}
	fclose(file2Ptr);
	for(i=0;i<firstSize;i++) //check if file1 is sorted correctly
	{
		if(i+1!=firstSize)
		{
			if(firstArray[i]>firstArray[i+1])
			{
				printf("%s is not sorted correctly. please fix it.\n",dummyFile1);
				exit(1);
			}
		}
	}
	for(i=0;i<secondSize;i++) //check if file2 is sorted correctly
	{
		if(i+1!=secondSize)
		{
			if(secondArray[i]>secondArray[i+1])
			{
				printf("%s is not sorted correctly. please fix it.\n",dummyFile2);
				exit(1);
			}
		}
	}
	//merge file1 and file2
	merge(firstArray,firstSize,secondArray,secondSize,mergedArray);
	for(i=0;i<(firstSize+secondSize);i++)
	{
		if(threadId==1) //if it's writer thread 1 then write merged array into buffer1
			insert(&buffer1, mergedArray[i]);
		else if(threadId==2) //if it's writer thread 2 then write merged array into buffer2
			insert(&buffer2, mergedArray[i]);
		else if(threadId==3) //if it's writer thread 3 then write merged array into buffer3
			insert(&buffer3, mergedArray[i]);
		else if(threadId==4) //if it's writer thread 4 then write merged array into buffer4
			insert(&buffer4, mergedArray[i]);
	}
}

void fileCheck(int argc, char *argv[]) //check files if they are exist or not
{
    if(argc == 7)
    {
        int file1 = access( argv[3], F_OK );
        int file2 = access( argv[4], F_OK );

        if(file1==-1 || file2==-1) //if any of the file is not exist
        {
            printf("[WARNING] error code 11: input file(s) not exist\n");
            exit(1);
        }
    }
    if(argc == 9) //if any of the file is not exist
    {
        int file1 = access( argv[3], F_OK );
        int file2 = access( argv[4], F_OK );
        int file3 = access( argv[5], F_OK );
        int file4 = access( argv[6], F_OK );

        if(file1==-1 || file2==-1 || file3==-1 || file4==-1)
        {
            printf("[WARNING] error code 12: input file(s) not exist\n");
            exit(1);
        }
    }
    if(argc == 13) //if any of the file is not exist
    {
        int file1 = access( argv[3], F_OK );
        int file2 = access( argv[4], F_OK );
        int file3 = access( argv[5], F_OK );
        int file4 = access( argv[6], F_OK );
        int file5 = access( argv[7], F_OK );
        int file6 = access( argv[8], F_OK );
        int file7 = access( argv[9], F_OK );
        int file8 = access( argv[10], F_OK );

        if(file1==-1 || file2==-1 || file3==-1 || file4==-1 || file5==-1 || file6==-1 || file7==-1 || file8==-1)
        {
            printf("[WARNING] error code 13: input file(s) not exist\n");
            exit(1);
        }
    }
}

void errorCheck(int argc, char *argv[]) //check arguments
{
    /*
        Valid inputs:
        ./merge -n 8 file1 file2 file3 file4 file5 file6 file7 file8 -o outputfile -> argc = 13
        ./merge -n 4 file1 file2 file3 file4 -o outputfile -> argc = 9
        ./merge -n 2 file1 file2 -o outputfile -> argc = 7
    */

    if(argc < 7) //minimum argument size = 7
    {
        printf("[WARNING] error code 1: missing argument(s)\n");
        exit(1);
    }
    else if(argc == 7 || argc == 9 || argc == 13) //argument size must be 7, 9 or 13
    {
        if(strcmp(argv[1], "-n")!=0) //"-n" not given
        {
            printf("[WARNING] error code 2: \"-n\" not given\n");
            exit(1);
        }
        else
        {
            if(!(atoi(argv[2]) == 2 || atoi(argv[2]) == 4 || atoi(argv[2]) == 8)) //wrong number of files
            {
                printf("[WARNING] error code 3: wrong number of files\n");
                exit(1);
            }
            else
            {
                if(argc == 7)
                {
                    if(atoi(argv[2]) != 2)
                    {
                        printf("[WARNING] error code 4: wrong number of files\n");
                        exit(1);
                    }
                    else if(strcmp(argv[5], "-o")!=0) //"-o" not given
                    {
                        printf("[WARNING] error code 5: \"-o\" not given\n");
                        exit(1);
                    }
                }
                else if(argc == 9)
                {
                    if(atoi(argv[2]) != 4)
                    {
                        printf("[WARNING] error code 6: wrong number of files\n");
                        exit(1);
                    }
                    else if(strcmp(argv[7], "-o")!=0) //"-o" not given
                    {
                        printf("[WARNING] error code 7: \"-o\" not given\n");
                        exit(1);
                    }
                }
                else if(argc == 13)
                {
                    if(atoi(argv[2]) != 8)
                    {
                        printf("[WARNING] error code 8: wrong number of files\n");
                        exit(1);
                    }
                    else if(strcmp(argv[11], "-o")!=0) //"-o" not given
                    {
                        printf("[WARNING] error code 9: \"-o\" not given\n");
                        exit(1);
                    }
                }
            }
        }
    }
    else
    {
        printf("[WARNING] error code 10: something wrong about arguments\n");
        exit(1);
    }
}
//merge two sorted arrays into another array
void merge(int firstArray[], int firstSize, int secondArray[], int secondSize, int mergedArray[])
{
	int i, j = 0, k = 0;
	for (i = 0; i < firstSize + secondSize;)
	{
		if (j < firstSize && k < secondSize)
		{
			if (firstArray[j] < secondArray[k])
			{
				mergedArray[i] = firstArray[j];
				j++;
			}
			else
			{
				mergedArray[i] = secondArray[k];
				k++;
			}
			i++;
		}
		else if (j == firstSize)
		{
			while(i<(firstSize+secondSize))
			{
				mergedArray[i] = secondArray[k];
				k++;
				i++;
			}
		}
		else
		{
			while(i<(firstSize+secondSize))
			{
				mergedArray[i] = firstArray[j];
				j++;
				i++;
			}
		}
	}
}
//for reader threads
void *reader(void *threadId)
{
	int i = 0;
	long tid = (long) threadId;
	while(1)
	{
		//num_readers = 0 initially
		pthread_mutex_lock(&reader_mutex); //lock reader mutex
		num_readers++; //increment number_readers
		if (num_readers == 1) //if a reader came then incremented the num_readers by one
		{
			pthread_mutex_lock(&rw_mutex); //lock writer mutex
		}
		pthread_mutex_unlock(&reader_mutex); //unlock reader mutex

		if(thread1==1&&thread2==1&&tid==1) //if buffer1 and buffer2 is ready merge them with reader thread 1
		{
			printf("[READER] 1 executing:\n");
			readAndMergeBuffers(tid); //merge buffer1 and buffer2 -> buffer7
			thread5 = 1; //buffer5 is ready
			thread1 = 0;
			thread2 = 0;
			pthread_mutex_unlock(&rw_mutex); //unlock writer mutex
			pthread_exit(0); //terminate the reader thread 1
		}
		if(thread3==1&&thread4==1&&tid==2) //if buffer3 and buffer4 is ready merge them with reader thread 2
		{
			printf("[READER] 2 executing:\n");
			readAndMergeBuffers(tid); //merge buffer3 and buffer4 -> buffer6
			thread6 = 1; //buffer6 is ready
			thread3 = 0;
			thread4 = 0;
			pthread_mutex_unlock(&rw_mutex); //unlock writer mutex
			pthread_exit(0); //terminate the reader thread 2
		}
		if(thread5==1&&thread6==1&&tid==3) //if buffer5 and buffer6 is ready merge them with reader thread 3
		{
			//this thread will write the merged buffer into output file at the end of
			//the readAndMergeBuffers function for 8 files
			printf("[READER] 3 executing:\n");
			readAndMergeBuffers(tid); //merge buffer 5 and buffer6 -> output file
			thread5 = 0;
			thread6 = 0;
			pthread_mutex_unlock(&rw_mutex); //unlock writer mutex
			pthread_exit(0); //terminate the reader thread 3
		}
		pthread_mutex_lock(&reader_mutex); //lock reader mutex
		num_readers--; //decrement the num_reader by one
		if (num_readers == 0)
		pthread_mutex_unlock(&rw_mutex); //unlock writer mutex
		pthread_mutex_unlock(&reader_mutex); //unlock reader mutex
		i++;
	}
}

void *writer(void *threadId)
{
	long tid = (long) threadId;
	//pthread_mutex_lock(&rw_mutex);
	if(tid==1) //if it's writer thread 1 then merge file1 and file2
	{
		printf("[WRITER] %d executing:\n",tid);
		readAndMergeFiles(tid); //merge file1 and file2
		if(stopRead==2) //We can stop here for two files
		{
			struct node** head_ref = &buffer1;
			FILE *out = fopen(outputfile, "w"); //write buffer1 to the output file
			while((*head_ref)!=NULL)
			{
				fprintf(out, "%d\n",(*head_ref)->data);
				(*head_ref) = (*head_ref)->next;
			}
		}
		thread1 = 1;
	}
	if(tid==2) //if it's writer thread 2 then merge file3 and file4
	{
		printf("[WRITER] %d executing:\n",tid);
		readAndMergeFiles(tid); //merge file3 and file4
		thread2 = 1;
	}
	if(tid==3) //if it's writer thread 3 then merge file5 and file6
	{
		printf("[WRITER] %d executing:\n",tid);
		readAndMergeFiles(tid); //merge file5 and file6
		thread3 = 1;
	}
	if(tid==4) //if it's writer thread 4 then merge file7 and file8
	{
		printf("[WRITER] %d executing:\n",tid);
		readAndMergeFiles(tid); //merge file7 and file8
		thread4 = 1;
	}
	//pthread_mutex_unlock(&rw_mutex);
	pthread_exit(NULL); //terminate the writer thread
}
