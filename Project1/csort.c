#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_WORKERS 5
#define MIN_WORKERS 1

struct ListNode
{
  long long int val;
  struct ListNode* next;
};

void workerProcess( mqd_t mq, struct mq_attr attr, int i, int n, int fd, int filesize);
int wait();
int comp64( const void* a, const void* b);

// a > b
int comp64( const void* a, const void* b)
{
  long long int aval;
  long long int bval;

  aval = *( long long int*)a;
  bval = *( long long int*)b;

  if( aval > bval)
  {
    return 1;
  }
  else if( aval < bval)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

void workerProcess( mqd_t mq, struct mq_attr attr, int proc, int n, int fd, int filesize)
{
  long int longs_count;
  long int item_count;
  long int offset;
  long int read_len;
  long int i;
  char* buffer;
  long long int c;
  struct ListNode* queueHead;
  struct ListNode* queueCur;

  //say hi
  printf( "worker:%d/%d, mqid:%d, fd:%d, fsize:%d\n", proc+1, n, mq, fd, filesize);

  longs_count = filesize / 8;
  offset = 8 * proc * (longs_count / n);

  // distribute evenly..
  if( proc == n-1) //last worker case...
  {
    read_len = filesize-offset;
  }
  else
  {
    read_len =  8 * (longs_count / n);
  }
  printf( "proc: %d offset -> %ld, read_len -> %ld\n", proc, offset, read_len);

  buffer = (char*) malloc( read_len * sizeof(char));

  //offset read.
  lseek(fd, offset, 0);
  read(fd, buffer, read_len);

  //check. (no null termination in this print function; too bad.)
  //printf( "proc: %d reads -> %s\n", proc, buffer);

  item_count = read_len / 8;

  printf( "item count at %d : %ld\n", proc, item_count);


  // debug purposed code..
  printf("BEFORE SORT:\n");
  for( i = 0; i < item_count; i++)
  {
    printf( "%lld, ", ((long long int*)buffer)[i]);
  }
  printf( "\n");

  qsort( buffer, item_count, sizeof( long long int), comp64);


  // debug purposed code..
  printf("AFTER SORT:\n");
  for( i = 0; i < item_count; i++)
  {
    printf( "%lld, ", ((unsigned long long int*)buffer)[i]);
  }
  printf( "\n");


  //TODO: fill LL,
  queueHead = (struct ListNode*) malloc( sizeof( struct ListNode));
  queueCur = queueHead;

  for( i = 0; i < item_count; i++)
  {
    queueCur->val = ((long long*)buffer)[i];
    queueCur->next = (struct ListNode*) malloc( sizeof( struct ListNode));
    queueCur = queueCur->next;
  }
  queueCur->val = 0; //last is zero

  //DEBUG LL
  queueCur = queueHead;
  for( i = 0; i < item_count; i++)
  {
    printf( "PROC: %lld, node %ld: %lld \n", proc, i, queueCur->val);
    queueCur = queueCur->next;
  }


  //TODO: send messages.
  queueCur = queueHead;
  for( i = 0; i < item_count; i++)
  {
    mq_send(mq, (char*) &queueCur->val, 8, 0);
    queueCur = queueCur->next;
  }

  //bye bye
  free( buffer);

  //freelist.

}

int main(int argc, char** argv)
{
  //arrays
  mqd_t* mq_arr;
  struct mq_attr* attr_arr;
  pid_t* pid_arr;
  int* done_arr;
  long long int* merge_buffer;
  long long int* sorted_arr;

  //other variables
  int n;
  long long int i;
  long long int j;
  long long int received;
  int in_fd;
  int out_fd;
  size_t filesize;

  long long int c[5];

  //usage check
  if( argc != 4)
  {
    printf("invalid arguments\nusage: <n> <bin_in> <text_out>\n");
    return 1;
  }

  n = atoi(argv[1]); //get worker count = n


  // check if valid worker count
  if( n < MIN_WORKERS || n > MAX_WORKERS)
  {
    printf("invalid worker count: 1 < n < 5\n");
    return 1;
  }

  //allocate arrays, dont forget to free!!
  pid_arr = (pid_t*) malloc( sizeof( pid_t) * n);
  mq_arr = (mqd_t*) malloc( sizeof( mqd_t) * n);
  attr_arr = (struct mq_attr*) malloc( sizeof( struct mq_attr) * n);

  //file init...
  in_fd = open( argv[2], O_RDONLY, 0);
  out_fd = creat( argv[3], 0666);

  // file exceptions...
  if( !in_fd)
  {
    printf("could not open input file, terminating...\n");
    return 1;
  }

  if( !out_fd)
  {
    printf("could not create output file, terminating...\n");
    return 1;
  }

  //input file length??
  filesize = lseek(in_fd, 0, SEEK_END);

  //init message queues, pass them to workers
  for( i = 0; i < n; i++)
  {
    switch (i) {
      case 0:
        mq_arr[i] = mq_open("/mqname1", O_RDWR | O_CREAT, 0666, NULL);
        break;
      case 1:
        mq_arr[i] = mq_open("/mqname2", O_RDWR | O_CREAT, 0666, NULL);
        break;
      case 2:
        mq_arr[i] = mq_open("/mqname3", O_RDWR | O_CREAT, 0666, NULL);
        break;
      case 3:
        mq_arr[i] = mq_open("/mqname4", O_RDWR | O_CREAT, 0666, NULL);
        break;
      case 4:
        mq_arr[i] = mq_open("/mqname5", O_RDWR | O_CREAT, 0666, NULL);
        break;
    }

    if( mq_arr[i] == -1) //mq failure..
    {
      printf( "damn we have a problem here, terminating...\n");
      exit(1);
    }

    printf("mq created, mq id = %d\n", (int) mq_arr[i]);
    mq_getattr( mq_arr[i], &attr_arr[i]);

    pid_arr[i] = fork(); // create worker
    if( !pid_arr[i]) //worker only zone.
    {
      workerProcess(mq_arr[i], attr_arr[i], i, n, in_fd, filesize); // go gog o

      exit(0);
    }
  }



  printf( "no problemo\n");
  //wait for the messages
  //TODO: receive messages... merge while receiving
  //loop until no msgs
  done_arr = (int*) malloc( n * sizeof(int) );
  merge_buffer = ( long long int*) malloc( n * sizeof(long long));
  sorted_arr = ( long long int*) malloc( filesize);
  memset( done_arr, 0, n * sizeof(int));

  j = 0;
  for( received = 0; received < filesize / 8;)
  {
    for ( i = 0; i < n; i++)
    {
      if( !done_arr[i])
        mq_receive( mq_arr[i], (char*) &merge_buffer[i], attr_arr[i].mq_msgsize, 0);
      else
        printf("DONE: PROC:%lld\n", i);

      printf( "%lld: received %lld from %lld\n", received, merge_buffer[i], i);
      if( merge_buffer[i])
        received++;
      else
        done_arr[i] = 1;
    }

    qsort( merge_buffer, n, sizeof( long long int), comp64);

    for ( i = 0; i < n; i++)
    {
      if( merge_buffer[i])
        sorted_arr[j++] = merge_buffer[i];
    }
  }
  //merge run
  //endwhile

  for( i = 0; i < filesize / 8; i++)
  {
    printf( "%lld, ", sorted_arr[i]);
  }
  printf( "\n");


  //TODO: write sorted list to file.
  //
  //
  //

  //better wait.
  for( i = 0; i < n; i++)
  {
    wait();
  }

  //nothing to do here, set everything on fire.
  mq_unlink("/mqname1");
  mq_unlink("/mqname2");
  mq_unlink("/mqname3");
  mq_unlink("/mqname4");
  mq_unlink("/mqname5");
  for( i = 0; i < n; i++)
  {
    mq_close( mq_arr[i]);
  }
  //
  close( in_fd);
  close( out_fd);

  //dealloc arrays.
  free( merge_buffer);
  free( sorted_arr);
  free( done_arr);
  free( pid_arr);
  free( mq_arr);
  free( attr_arr);

  return 0;
}
