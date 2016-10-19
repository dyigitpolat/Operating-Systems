#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

int main()
{
  int n = fork();

  if( n == 0)
  {
    printf("hi from child\n");
  }
  else
  {
    printf("hi from parent\n");
  }

  return 0;
}
