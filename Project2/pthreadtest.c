#include "stdlib.h"
#include "stdio.h"
#include "pthread.h"
#include "unistd.h"

#define u64 unsigned long long


struct listNode
{
  struct listNode* next;
  u64 value;
};


void* sayhello(int i)
{
  u64 test;
  test = 10;
  printf("hello %d %llu\n", i, test);
  pthread_exit(0);
}

void readFile( FILE* fp, struct listNode* callerIDs, struct listNode* calledIDs, struct listNode* years)
{
  char buffer[512];
  u64 callerID;
  u64 calledID;
  u64 year;

  struct listNode* curCallers;
  struct listNode* curCalleds;
  struct listNode* curYears;


  printf( "INPUT FILE START\n");
  curCallers = callerIDs;
  curCalleds = calledIDs;
  curYears = years;

  while( EOF != fscanf(fp, "%llu %llu %llu\n", &callerID, &calledID, &year))
  {
    printf("caller:%llu -- called:%llu -- year:%llu\n", callerID, calledID, year);

    curCallers->value = callerID;
    curCallers->next = (struct listNode*) malloc( sizeof(struct listNode));
    curCallers = curCallers->next;

    curCalleds->value = calledID;
    curCalleds->next = (struct listNode*) malloc( sizeof(struct listNode));
    curCalleds = curCalleds->next;

    curYears->value = year;
    curYears->next = (struct listNode*) malloc( sizeof(struct listNode));
    curYears = curYears->next;
  }

  curCallers->value = 0;
  curCallers->next = 0;

  curCalleds->value = 0;
  curCalleds->next = 0;

  curYears->value = 0;
  curYears->next = 0;
}

int main()
{
  pthread_t tid;
  int par;
  int ret;

  par = 3;

  ret = pthread_create( &(tid), 0, &sayhello, (void*) par);

  if( ret)
  {
    printf("mq thread failed.");
    return 1;
  }


  ret = pthread_join(tid, 0);
  return 0;
}
