#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>

typedef void* (*fun)(void*);
#define FILEPATH "all_is_well.txt"
#define M 1
#define R 1
static pthread_mutex_t mutex[M+R] = {PTHREAD_MUTEX_INITIALIZER};
static pthread_cond_t mapper_over[M] = {PTHREAD_COND_INITIALIZER};
static pthread_cond_t reducer_over[R] = {PTHREAD_COND_INITIALIZER};
int M=1;
int R=1;
void* InputThread(void * arg);
void* OutputThread(void *arg);
struct MapperArg
{
	char *mapPointer;
	int start;
	int end;
	int id;
	struct Counter *counter;
};
struct ReducerArg
{
	int id;
	struct Counter *mapOutputAll;
	struct Counter *counter;
};
struct Counter
{
	int count[26];
};
int main(int argc, char *argv[])
{
    int i,j;
    int fd;
    char *map;  /* mmapped array of char's */
    int filesize=0;
    int interval=0;
    struct MapperArg mapperArg[M];
    struct ReducerArg reducerArg[R];
    pthread_t mapper[M];
    pthread_t reducer[R];
    struct stat file_status;
    struct Counter mapOutput[M];
    struct Counter reduceOutput[R];
    int final[26];


    /*
     * Get the size of target file
     */
    if(stat(FILEPATH, &file_status) != 0){
    perror("could not stat");
    }
    filesize=file_status.st_size;
    interval=(filesize%M==0)?filesize/M:filesize/M+1;

    fd = open(FILEPATH, O_RDONLY);

    if (fd == -1) {
	perror("Error opening file for reading");
	exit(EXIT_FAILURE);
    }
    /*
     * MMAP maps the file to memory
     */
    map = (char *)mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
    mapperArg->mapPointer=map;
    if (map == MAP_FAILED) {
	close(fd);
	perror("Error mmapping the file");
	exit(EXIT_FAILURE);
    }

    /*
     * Open thread
     */
    for (i=0;i<M;i++)
    {
    	mapperArg[i].start=i*interval;
    	mapperArg[i].end=(i+1)*interval;
    	if(mapperArg[i].end>filesize)
    	{
    		mapperArg[i].end=filesize;
    	}
    	mapperArg[i].counter=&mapOutput[i];
    	mapperArg[i].id=i;
    	pthread_create(&mapper[i],NULL,InputThread,&mapperArg[i]);
    }
    for(i=0;i<R;i++)
    {
    	reducerArg[i].id=i;
    	reducerArg[i].counter=&reduceOutput[i];
    	reducerArg[i].mapOutputAll=mapOutput;
    	pthread_create(&reducer[i],NULL,OutputThread,&reducerArg[i]);
    }
    for(i=0;i<R;i++)
    {
    	pthread_cond_wait(&reducer_over[i],&mutex[M+i]);
    }
    /*
     * Initialize final result
     */
    for(i=0;i<26;i++)
    {
    	final[i]=0;
    }
    for(j=0;j<26;j++)
    {
    	for(i=0;i<R;i++)
    	{
    		final[j]=final[j]+reduceOutput[i].count[j];
    	}
    	printf("%c %d\n",(char)(j+65),final[j]);
    }
    if (munmap(map, filesize) == -1) {
	perror("Error un-mmapping the file");
    }
    close(fd);
    return 0;
}
void* InputThread(void * arg)
{
	struct MapperArg *InputArg=(struct MapperArg*)arg;
	struct Counter *mapOutput=InputArg->counter;
	int i,letter;
	/*
	 * Initialize the counter
	 */
	for(i=0;i<26;i++)
	{
		mapOutput->count[i]=0;
	}
	for(i=InputArg->start;i<InputArg->end;i++)
	{
		if((int)InputArg->mapPointer[i]>96 && (int)InputArg->mapPointer[i]<123)
		{
			letter=(int)InputArg->mapPointer[i]-97;
		}
		else if((int)InputArg->mapPointer[i]>64 && (int)InputArg->mapPointer[i]<91)
		{
			letter=(int)InputArg->mapPointer[i]-65;
		}
		mapOutput->count[letter]=mapOutput->count[letter]+1;
	}
		pthread_cond_signal(&mapper_over[InputArg->id]);
		return NULL;
}
void* OutputThread(void *arg)
{
	struct ReducerArg *InputArg=(struct ReducerArg*)arg;
	struct Counter mapOutput;
	struct Counter *reduceOutput=InputArg->counter;
	int i,j;
	for(i=0;i<26;i++)
	{
		reduceOutput->count[i]=0;
	}
	for(i=0;i<M;i++)
	{
		if(i%R==InputArg->id)
		{
		pthread_cond_wait(&mapper_over[i], &mutex[i]);
		mapOutput=InputArg->mapOutputAll[i];
		for(j=0;j<26;j++)
		{
			reduceOutput->count[j]=reduceOutput->count[j]+mapOutput.count[j];
		}
		}
	}
	pthread_cond_signal(&reducer_over[InputArg->id]);
	return NULL;
}
