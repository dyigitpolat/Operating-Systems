#include <stdlib.h>
#include <stdio.h>

void writelonglongs( long long int* arr, long long int n, FILE* fp)
{
  int i;
  long long int num;
  char c;
  for( i = 0; i < 8 * n; i++)
  {
    num = arr[ i / 8];
    c = num >> ( 64 - (8 * (i%8)) );
    putc( c, fp);
  }
}

int main( int argc, char** argv)
{
  long long int arr[15] = { 2, 3, 5, 1, 4, 6, 15, 12, 13, 11, 10, 7, 8, 9, 14};

  FILE* fp = fopen( argv[1], "w");
  writelonglongs(arr, 15, fp);
  fclose(fp);
  return 0;
}
