#include "stdlib.h"
#include "stdio.h"
#include "pthread.h"
#include "unistd.h"

void* sayhello(int i)
{
  printf("hello %d\n", i);
  pthread_exit(0);
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
