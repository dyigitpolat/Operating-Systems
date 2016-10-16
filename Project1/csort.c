#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_WORKERS 5
#define MIN_WORKERS 1

void workerProcess( mqd_t mq, struct mq_attr attr, int i, int n, int fd, int filesize);
int wait();

void childProcess( mqd_t mq)
{
  char* message;

  // allocate space for our message!
  message = (char*) malloc( 8192);
  strcpy( message, "selamun aleyküm bilal emmi\n"); // whats gonna be send there?
  printf( "sending into mq: %d\n", mq); // double cheeck
  mq_send( mq, message, 8192, 0); // launch!
  free(message); //bye bye!

}

void workerProcess( mqd_t mq, struct mq_attr attr, int proc, int n, int fd, int filesize)
{
  long int longs_count;
  long int offset;
  long int read_len;
  long int i;
  char* buffer;
  long long int* arr;

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

  buffer = (long long int*) malloc( read_len * sizeof(char));

  //offset read.
  lseek(fd, offset, 0);
  read(fd, buffer, read_len);

  //check.
  printf( "proc: %d reads -> %s\n", proc, buffer);

  //TODO: what is item count??
  item_count = 0;

  //TODO: init. array
  arr = ( long long int*) malloc( sizeof( long long int) * item_count);

  //TODO: fill array
  for(i = 0; buffer[i]; i++)
  {

  }

  //TODO: qsort array, TODO: comp func.

  //TODO: fill LL

  //TODO: send messages.

  //bye bye
  free( buffer);
  free( arr);

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
    mq_arr[i] = mq_open("/mqname1", O_RDWR | O_CREAT, 0666, NULL);

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

  //wait for the messages
  //TODO: receive messages...
  // do together...
  //TODO: merge messages...

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


  /*
  *
  * BELOW ARE TO BE DELETED
  *
  */

  mqd_t mq;
  struct mq_attr mq_attr;
  char* message;
  int a;
  /* just shown one message queue creation. More needed if more than
  one child needs to be created.
  */
  mq = mq_open("/mqname1", O_RDWR | O_CREAT, 0666, NULL);

  // is there any problem boy?
  if (mq == -1)
  {
    perror("can not create msg queue\n");
    exit(1);
  }

  // info about the message queue created...
  printf("mq created, mq id = %d\n", (int) mq);
  mq_getattr(mq, &mq_attr);
  printf("mq maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);

  //lets create our child process and give her a job.
  a = fork();
  if( a == 0)
  {
    childProcess(mq);
    exit(0);
  }

  //allocate space for the message and wait for it...
  message = (char*) malloc (8192);
  // this is how you receive the message, simple enough!
  mq_receive(mq, message, 8192, 0);
  wait(); //not really necessary but we should keep things neat.

  //lets see if our message is correctly received.
  printf("message received from mq:%d -> %s\n", mq, message);


  //we are turning stuff off...
  free(message);
  mq_close(mq);
  return 0;
}
