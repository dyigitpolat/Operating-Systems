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
  unsigned long long val;
  struct ListNode* next;
};

//subroutine that child processes will run
void workerProcess( mqd_t mq, struct mq_attr attr, int i, int n, int fd, int filesize);
//declare wait
int wait();
//compares two unsigned long longs. a > b will return positive.
int comp64( const void* a, const void* b);
//subroutine for main process to merge received messages, returns a linked list of sorted values
struct ListNode* receiveAndMerge( mqd_t* mq_arr, int n, long item_count);
//write linked list contents to file
void writeToFile( int fd, struct ListNode* sortedList);
//free the linked list.
void freeList( struct ListNode* mynode);

void freeList( struct ListNode* mynode)
{
	struct ListNode* prev;
	struct ListNode* cur;
	int i;

	prev = mynode;
	cur = prev;
	i = 0;
	while(cur)
	{
		cur = prev->next;
		free(prev);
		prev = cur;
		i++;
	}

	printf("%d nodes freed.\n", i);
}

void writeToFile( int fd, struct ListNode* sortedList)
{
  char number[128]; //number string buffer
  int i;

  while( sortedList && sortedList->val)
  {
    //string representation of the long long
    sprintf(number, "%llu", sortedList->val);
    i = -1;
    while( number[++i]); //get length of the string seek to the end
    write( fd, number,i); //write num to file
    write( fd, "\n", 1); //append new line
    sortedList = sortedList->next; //next num
  }
}

struct ListNode* receiveAndMerge( mqd_t* mq_arr, int n, long item_count)
{
  //queue heads for freeing
  struct ListNode** static_heads;
  // queue consume
  struct ListNode** heads;
  //queue populate
  struct ListNode** tails;
  //head of final list
  struct ListNode* sortedList;
  //temp ptr to move along the sortedList
  struct ListNode* sortedListCur;

  unsigned long long smallest;
  long items_read; //count of items processed
  int* done_arr; //keep track of finished queues
  int small_i; //where is smallest at?
  int i;
  int flag;
  int flag2;

  //init done_arr
  done_arr = (int*) malloc( n * sizeof( int));
  memset( done_arr, 0, n * sizeof( int));

  //initialize lists and list* arrays.
  sortedList = (struct ListNode*) malloc( sizeof(struct ListNode));
  sortedListCur = sortedList;

  heads = (struct ListNode**) malloc( n * sizeof(struct ListNode*));
  tails = (struct ListNode**) malloc( n * sizeof(struct ListNode*));
  static_heads = (struct ListNode**) malloc( n * sizeof(struct ListNode*));
  for( i = 0; i < n; i++)
  {
    heads[i] = (struct ListNode*) malloc( sizeof(struct ListNode));
  }

  for( i = 0; i < n; i++)
  {
    tails[i] = heads[i];
  }

  for( i = 0; i < n; i++)
  {
    static_heads[i] = heads[i];
  }

  /*
  * THIS PART MAY BE VERY CONFUSING!
  * READ THE COMMENTS CAREFULLY
  * WHAT IT DOES IS BASICALLY CHOOSING THE SMALLEST RECEIVED
  * VALUE AND PUTTING IT INTO OUR LIST
  * RECEIVED VALES ARE STORED IN QUEUES (LL)
  */
  while( 1) //loop until
  {
    for( i = 0; i < n; i++)
    {
      if( !done_arr[i]) // if done dont try to receivw
      {
        int bytes = 8;
        while( bytes) //ensure that all bytes received.
        {
          //receive 8 bytes as set in the attributes
          //read into linked list val section.
          bytes -= mq_receive( mq_arr[i], (char*) &( tails[i]->val), 8, 0);
        }
      }

      if( tails[i]->val) //if non zero received, accept
      {
        printf("RECEIVED:::%lld FROM:::%d\n", tails[i]->val, i);
        tails[i]->next = (struct ListNode*) malloc( sizeof(struct ListNode)); //add node
        tails[i] = tails[i]->next; //wait for next number
        tails[i]->val = 0;
      }
      else //if zero is received
      {
        if( !done_arr[i])
          printf("RECEIVED:::%lld FROM:::%d\n", tails[i]->val, i);
        done_arr[i] = 1; //last element, turn this queue off.
      }
    }

    smallest = 0;
    flag = 0;
    items_read = 0;
    small_i = -1; //-1 means no elements are available in that current run.
    for( i = 0; i < n; i++) //find minimum
    {
      if( !flag) //first elem case.
      {
        if( heads[i]->val)
        {
          smallest = heads[i]->val;
          small_i = i;
          flag = 1;
        }
      }
      else if( heads[i]->val < smallest && heads[i]->val)
      {
        smallest = heads[i]->val;
        small_i = i;
      }
    }

    if( smallest && small_i != -1) //if valid smallest...
    {
      printf("INSERTED::: %lld\n", smallest);
      sortedListCur->val = smallest;
      sortedListCur->next = (struct ListNode*) malloc( sizeof(struct ListNode));
      sortedListCur = sortedListCur->next;
      sortedListCur->val = 0;

      //we have read one more...
      items_read++;

      heads[small_i] = heads[small_i]->next;
      //free(cur);
    }
    else //could process no more
    {
      break;
    }
  }

  /**
    *debug purposed code.
  for( i = 0; i < n; i++)
  {
    printf( "%d: ", i);
    cur = heads[i];
    while(cur)
    {
      printf( "%lld, ", cur->val);
      cur = cur->next;
    }
    printf("\n");
  }
  **/

  //
  //
  //we should add remaining in sorted order.
  // same as above but no receive this time..
  flag = 1; //not done
  while( flag) //loop while not done.
  {
    flag = 0; //set done
    memset( done_arr, 0, n * sizeof(int));
    for( i = 0; i < n; i++)
    {
      if( !heads[i]->val)
      {
        done_arr[i] = 1;
      }
      else
      {
        flag = 1; //not done.
      }
    }

    smallest = 0;
    flag2 = 0;
    items_read = 0;
    small_i = -1;
    for( i = 0; i < n; i++) //find minimum
    {
      if( !flag2)
      {
        if( heads[i]->val)
        {
          smallest = heads[i]->val;
          small_i = i;
          flag2 = 1;
        }
      }
      else if( heads[i]->val < smallest && heads[i]->val)
      {
        smallest = heads[i]->val;
        small_i = i;
      }
    }

    if( smallest && small_i != -1) //insert
    {
      printf("INSERTED::: %lld\n", smallest);
      sortedListCur->val = smallest;
      sortedListCur->next = (struct ListNode*) malloc( sizeof(struct ListNode));
      sortedListCur = sortedListCur->next;
      sortedListCur->val = 0;

      //we have read one more...
      items_read++;

      heads[small_i] = heads[small_i]->next;
      //free(cur);
    }

  }
  //
  //
  //

  //pprint sorted
  printf("SORTED LIST: \n" );
  sortedListCur = sortedList;
  while(sortedListCur)
  {
    printf( "%lld, ", sortedListCur->val);
    sortedListCur = sortedListCur->next;
  }
  printf("\n");

  //free lists that are not used
  for( i = 0; i < n; i++)
  {
    freeList(static_heads[i]);
  }
  free(tails);
  free(heads);
  free(static_heads);
  free(done_arr);

  return sortedList;
}

// a > b
int comp64( const void* a, const void* b)
{
  unsigned long long aval;
  unsigned long long bval;

  aval = *( unsigned long long*)a;
  bval = *( unsigned long long*)b;

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
  long longs_count;
  long item_count;
  long offset;
  long read_len;
  long i;
  char* buffer;
  struct ListNode* queueHead;
  struct ListNode* queueCur;

  //say hi
  //printf( "worker:%d/%d, mqid:%d, fd:%d, fsize:%d\n", proc+1, n, mq, fd, filesize);

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
  //printf( "proc: %d offset -> %ld, read_len -> %ld\n", proc, offset, read_len);

  buffer = (char*) malloc( read_len * sizeof(char));

  //offset read.
  lseek(fd, offset, 0);
  read(fd, buffer, read_len);

  //check. (no null termination in this print function; too bad.)
  //printf( "proc: %d reads -> %s\n", proc, buffer);

  item_count = read_len / 8;

  //printf( "item count at %d : %ld\n", proc, item_count);

  /*
  // debug purposed code..
  printf("BEFORE SORT:\n");
  for( i = 0; i < item_count; i++)
  {
    printf( "%lld, ", ((unsigned long long*)buffer)[i]);
  }
  printf( "\n");
  */

  qsort( buffer, item_count, sizeof( unsigned long long), comp64);

  /*
  // debug purposed code..
  printf("AFTER SORT:\n");
  for( i = 0; i < item_count; i++)
  {
    printf( "%lld, ", ((unsigned long long*)buffer)[i]);
  }
  printf( "\n");
  */

  //Tfill LL,
  queueHead = (struct ListNode*) malloc( sizeof( struct ListNode));
  queueCur = queueHead;

  for( i = 0; i < item_count; i++) // put teh nums in tha queue
  {
    queueCur->val = ((unsigned long long*)buffer)[i];
    queueCur->next = (struct ListNode*) malloc( sizeof( struct ListNode));
    queueCur = queueCur->next;
  }
  queueCur->val = 0; //last is zero

  //send messages.
  queueCur = queueHead;
  for( i = 0; i <= item_count; i++) // send from tha queue
  {
    mq_send(mq, (char*) &queueCur->val, 8, 0);
    queueCur = queueCur->next;
  }

  //bye bye
  free( buffer);

  // freelist.
  freeList( queueHead);

}

int main(int argc, char** argv)
{
  //arrays
  mqd_t* mq_arr;
  struct mq_attr* attr_arr;
  pid_t* pid_arr;

  //other variables
  int n;
  unsigned long long i;
  int in_fd;
  int out_fd;
  size_t filesize;

  //sorted list;
  struct ListNode* sortedList;

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

  //set msg q attributes accordingly msgsize = 8 is very important.
  struct mq_attr attrib;
  attrib.mq_flags = 0;
  attrib.mq_maxmsg = 8;
  attrib.mq_msgsize = 8;
  attrib.mq_curmsgs = 0;

  //kernel should reset links.. otherwise any failure results with death.
  mq_unlink("/mqname1");
  mq_unlink("/mqname2");
  mq_unlink("/mqname3");
  mq_unlink("/mqname4");
  mq_unlink("/mqname5");

  //init message queues, pass them to workers
  for( i = 0; i < n; i++)
  {
    switch (i) {
      case 0:
        mq_arr[i] = mq_open("/mqname1", O_RDWR | O_CREAT, 0666, &attrib);
        break;
      case 1:
        mq_arr[i] = mq_open("/mqname2", O_RDWR | O_CREAT, 0666, &attrib);
        break;
      case 2:
        mq_arr[i] = mq_open("/mqname3", O_RDWR | O_CREAT, 0666, &attrib);
        break;
      case 3:
        mq_arr[i] = mq_open("/mqname4", O_RDWR | O_CREAT, 0666, &attrib);
        break;
      case 4:
        mq_arr[i] = mq_open("/mqname5", O_RDWR | O_CREAT, 0666, &attrib);
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


  //if we can reach here...
  printf( "no problemo\n");

  sortedList = receiveAndMerge( mq_arr, n, filesize / 8);
  writeToFile( out_fd, sortedList);

  //better wait.
  for( i = 0; i < n; i++)
  {
    wait();
  }

  //nothing to do here, set everything on fire. we are all done.
  for( i = 0; i < n; i++)
  {
    mq_close( mq_arr[i]);
  }
  //
  close( in_fd);
  close( out_fd);

  //dealloc arrays.
  free( pid_arr);
  free( mq_arr);
  free( attr_arr);

  //free list
  freeList( sortedList);

  return 0;
}
