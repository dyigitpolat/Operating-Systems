#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int readLongs( FILE* fp, long int* arr)
{
  char c;
  int i;

  // linked list agen??? :////////
  i = 0;
  while( ( c = getc(fp)) != EOF)
  {
    arr[i/4] = ( arr[i/4] << ( 8 * ( i % 4))) | c;
  }
  return 0;
}

int childProcess( mqd_t mq)
{
  char* message;

  // allocate space for our message!
  message = (char*) malloc( 8192);
  strcpy( message, "selamun aleyküm bilal emmi\n"); // whats gonna be send there?
  printf( "sending into mq: %d\n", mq); // double cheeck
  mq_send( mq, message, 8192, 0); // launch!
  free(message); //bye bye!
}

int main()
{
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
  printf("message received: %s\n", message);

  //we are turning stuff off...
  free(message);
  mq_close(mq);
  return 0;
}
