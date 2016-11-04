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

struct call
{
  u64 callerID;
  u64 calledID;
  u64 year;
};

struct callList
{
  u64 callerID;
  u64 calledID;
  u64 year;
  struct callList* next;
};

struct callerList
{
  u64 id;
  long count;
  struct callerList* next;
};

struct calledList
{
  u64 id;
  struct calledList* next;
};

/*
Global variables here
*/
FILE** temp_files;
FILE** temp_files_out;
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
struct caller* countCallers( struct call* call_arr, long count, long* reduced_count);
struct callList* appendToList( FILE* fp, struct callList* myList, long* count);
struct call* sortByCallerId( struct callList* myList, long count);

int compareCalls( const void* a, const void* b)
{
  u64 callerA;
  u64 callerB;
  struct call callA;
  struct call callB;

  callA = *( struct call*)a;
  callB = *( struct call*)b;
  callerA = callA.callerID;
  callerB = callB.callerID;
  if( callerA > callerB)
    return 1;
  else if( callerA == callerB)
    return 0;
  else
    return -1;
}

struct call* sortByCallerId( struct callList* myList, long count)
{
  struct callList* curList;
  struct call* sorted_arr;
  long i;

  sorted_arr = (struct call*) malloc( sizeof( struct call) * count);
  curList = myList;
  for( i = 0; i < count; i++)
  {
    sorted_arr[i].callerID = curList->callerID;
    sorted_arr[i].calledID = curList->calledID;
    sorted_arr[i].year = curList->year;
    curList = curList->next;
  }

  qsort( sorted_arr, count, sizeof( struct call), compareCalls);
  return sorted_arr;
}

struct callList* appendToList( FILE* fp, struct callList* myList, long* count)
{
  u64 callerID;
  u64 calledID;
  u64 year;
  struct callList* curList;

  while( EOF != fscanf(fp, "%llu %llu %llu\n", &callerID, &calledID, &year))
  {
    if( callerID <= endID && callerID >= startID && year <= endYear && year >= startYear)
    {
      curList = (struct callList*) malloc( sizeof( struct callList));
      curList->callerID = callerID;
      curList->calledID = calledID;
      curList->year = year;
      curList->next = myList;
      myList = curList;
      (*count)++;
    }
  }

  return myList;
}

struct caller* countCallers( struct call* call_arr, long count, long* reduced_count)
{
  struct caller* reduced_callers;
  struct callerList* curList;
  struct callerList* myList;
  long newcount;
  long i, j, last, numb;
  int unique;

  //initialize for the first elem.
  myList = (struct callerList*) malloc( sizeof( struct callerList));
  myList->id = (count ? call_arr[0].callerID : 0);
  myList->count = 0;
  myList->next = 0;

  newcount = (count ? 1 : 0);
  last = 0;
  for( i = 0; i < count; i++)
  {
    if( call_arr[i].callerID != call_arr[last].callerID)
    {
      last = i;
      curList = (struct callerList*) malloc( sizeof( struct callerList));
      curList->id = call_arr[i].callerID;
      curList->count = 0;
      curList->next = myList;
      myList = curList;
      newcount++;
    }

    unique = 1;
    for( j = last; j < i; j++) //bigOh(individuals_avg_call_count^2) but very fast loop.
    {
      if( call_arr[i].calledID == call_arr[j].calledID)
      {
        unique = 0;
        break;
      }
    }

    if( !(myList->count) || ( unique))
      myList->count++;
  }

  reduced_callers = ( struct caller*) malloc( sizeof( struct caller) * newcount);
  curList = myList;
  for( i = 0; i < newcount; i++)
  {
    reduced_callers[i].id = curList->id;
    reduced_callers[i].count = curList->count;
    curList = curList->next;
  }

  *reduced_count = newcount;
  return reduced_callers;
}



void* mapperThread( int i)
{
  u64 callerID, calledID, year;

  while( EOF != fscanf(infiles[i], "%llu %llu %llu\n", &callerID, &calledID, &year))
  {
    //printf("caller:%llu -- called:%llu -- year:%llu\n", callerID, calledID, year);
    fprintf( temp_files[i*R + callerID % R], "%llu %llu %llu\n", callerID, calledID, year);
  }

  pthread_exit(0);
}

void* reducerThread( int p)
{
  char tmp_f_name[256];
  struct callList* myList;
  struct call* sorted_arr;
  struct caller* reduced_callers;
  int i;
  long count, reduced_count;

  count = 0;
  myList = 0;
  for( i = 0; i < N; i++)
  {
    myList = appendToList( temp_files[i*R + p], myList, &count);
  }

  sorted_arr = sortByCallerId( myList, count);
  reduced_callers = countCallers( sorted_arr, count, &reduced_count);

  sprintf( tmp_f_name, "temp-out-%d", p );
  temp_files_out[p] = fopen( tmp_f_name, "w");

  for( i = 0; i < reduced_count; i++)
  {
    fprintf(temp_files_out[p], "%llu %ld\n", reduced_callers[i].id, reduced_callers[i].count);
  }

  pthread_exit(0);
}

void mapper()
{
  pthread_t* tids;
  char tmp_f_name[256];
  u64 i;

  temp_files = (FILE**) malloc( sizeof(FILE*) * N * R);
  temp_files_out = (FILE**) malloc( sizeof(FILE*) * N);
  for( i = 0; i < N*R; i++)
  {
    sprintf( tmp_f_name, "temp%llu-%llu", i/R, i%R );
    temp_files[i] = fopen( tmp_f_name, "w");
  }

  tids = (pthread_t*) malloc(sizeof(pthread_t) * N);
  for( i = 0; i < N; i++)
  {
    pthread_create( &(tids[i]), 0, (void*) &mapperThread, (void*) i);
  }

  for( i = 0; i < N; i++)
  {
    pthread_join( tids[i], 0);
  }

  for( i = 0; i < N*R; i++)
  {
    sprintf( tmp_f_name, "temp%llu-%llu", i/R, i%R );
    freopen( tmp_f_name, "r", temp_files[i]);
  }

}

void reducer()
{
  pthread_t* tids;
  u64 i;

  tids = (pthread_t*) malloc(sizeof(pthread_t) * R);
  for( i = 0; i < R; i++)
  {
    pthread_create( &(tids[i]), 0, (void*) &reducerThread, (void*) i);
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
  printf( "mapper() done\n");

  //fire the workers
  reducer();
  printf( "reducer() done\n");


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
