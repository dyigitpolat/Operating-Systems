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

void workerProcess( mqd_t mq, struct mq_attr attr, int proc, int n, int fd, int filesize)
{
  long int longs_count;
  long int item_count;
  long int offset;
  long int read_len;
  long int i;
  char* buffer;
  long long int c;

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

  //check.
  printf( "proc: %d reads -> %s\n", proc, buffer);

  //TODO: what is item count??

  //TODO: qsort array, TODO: comp func.

  //TODO: fill LL

  //TODO: send messages.
  c = proc;
  printf("message sending c = %lld\n", c);
  mq_send(mq, &c, 8, 0);
  printf("message SENT. c = %lld\n", c);

  //bye bye
  free( buffer);

}

int main(int argc, char** argv)
{
  mqd_t* mq_arr;
  struct mq_attr* attr_arr;
  pid_t* pid_arr;
  int n;
  int i;
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
      printf("I have done my job: %d\n", i);
      exit(0);
    }
  }

  //wait for the messages
  //TODO: receive messages... merge while receiving
  //loop until no msgs
  for ( i = 0; i < n; i++)
  {
    mq_receive( mq_arr[i], &c[i], attr_arr[i].mq_msgsize, 0);
    printf("RECEIVED msg: %lld\n", c[i]);
  }
  //merge run
  //endwhile


  //TODO: write sorted list to file.

  //better wait.
  for( i = 0; i < n; i++)
  {
    wait();
  }

  //nothing to do here, set everything on fire.
  for( i = 0; i < n; i++)
  {
    mq_close( mq_arr[i]);
  }
  close( in_fd);
  close( out_fd);

  free( pid_arr);
  free( mq_arr);
  free( attr_arr);

  return 0;
}
