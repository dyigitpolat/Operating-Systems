#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define u64 unsigned long long

//will be useful while sorting.
struct caller
{
  u64 id;
  long count;
};

/*
Global variables here
*/
FILE** temp_files;
int N;
int R;
char** inFileNames;
char* outFileName;
u64 startYear;
u64 endYear;
u64 startID;
u64 endID;
FILE** infiles;
FILE* outfile;

void mapper();
void reducer();
void* mapperThread( int i);
void* reducerThread( int i);

void* mapperThread( int i)
{
  u64 callerID, calledID, year;

  while( EOF != fscanf(infiles[i], "%llu %llu %llu\n", &callerID, &calledID, &year))
  {
    //printf("caller:%llu -- called:%llu -- year:%llu\n", callerID, calledID, year);
    fprintf( temp_files[i + callerID % R], "%llu %llu %llu\n", callerID, calledID, year);
  }

  pthread_exit(0);
}

void* reducerThread( int p)
{
  int i;
  for( i = 0; i < N; i++)
  {
    //do for...
    temp_files[i*R + p];
  }

  pthread_exit(0);
}

void mapper()
{
  pthread_t* tids;
  char tmp_f_name[256];
  int i;

  temp_files = (FILE**) malloc( sizeof(FILE*) * N * R);
  for( i = 0; i < N*R; i++)
  {
    sprintf(tmp_f_name, "temp%d-%d", i/R, i%R );
    temp_files[i] = fopen(tmp_f_name, "w");
  }

  tids = (pthread_t*) malloc(sizeof(pthread_t) * N);
  for( i = 0; i < N; i++)
  {
    pthread_create( &(tids[i]), 0, &mapperThread, (void*) i);
  }

  for( i = 0; i < N; i++)
  {
    pthread_join( tids[i], 0);
  }

}

void reducer()
{
  pthread_t* tids;
  int i;

  tids = (pthread_t*) malloc(sizeof(pthread_t) * N);
  for( i = 0; i < R; i++)
  {
    pthread_create( &(tids[i]), 0, &mapperThread, (void*) i);
  }

  for( i = 0; i < R; i++)
  {
    pthread_join( tids[i], 0);
  }
}




int main( int argc, char *argv[])
{
  char buffer[256];
  int i, j, k; // loop variables

  //validate input format
  if( argc < 2)
  {
    //instant failure k3k0
    printf( "bad input. no arguments?\n");
    return 1;
  }
  else
  {
    N = atoi( argv[1]);
    R = atoi(argv[2]);
    if( argc != (N+8) || N > 10 || N < 1 || R < 1)
    {
      //bad input message
      printf( "bad input.\n");
      return 1;
    }
    else
    {
      inFileNames = (char**) malloc( sizeof(char*) * N );
      for( i = 0; i < N; i++)
      {
        inFileNames[i] = argv[ 3 + i];
      }
      outFileName = argv[ 3 + N];
      startYear = atoll(argv[ 4 + N]);
      endYear = atoll(argv[ 5 + N]);
      startID = atoll(argv[ 6 + N]);
      endID = atoll(argv[ 7 + N]);
    }
  }

  //report input interpretation
  printf( "N = %d\n", N);
  printf( "R = %d\n", R);
  for( i = 0; i < N; i++)
  {
    printf( "input file %d = %s\n", i, inFileNames[i]);
  }
  printf( "output file = %s\n", outFileName);
  printf( "start year = %llu\n", startYear);
  printf( "end year = %llu\n", endYear);
  printf( "start ID = %llu\n", startID);
  printf( "end ID = %llu\n", endID);

  //input file contents:
  infiles = (FILE**) malloc( sizeof(FILE*) * N);
  for( i = 0; i < N; i++)
  {
    infiles[i] = fopen( inFileNames[i], "r");
  }

  //start the game :))
  printf( "proceed to execution...\n");

  //do reduce
  mapper();

  //fire the workers
  //reducer();


  //out file...

  //remove files
  for( i = 0; i < N*R; i++)
  {
    fclose( temp_files[i]);
    sprintf( buffer, "temp%d-%d", i/R, i%R);
    remove( buffer);
  }

	return 0;
}
