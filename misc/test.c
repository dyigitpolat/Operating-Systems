#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

int main( int argc, char** args)
{
	int i;
	for(i = 0; i < 8; i++)
	{
		int n = fork();
		if( n == 0)
		{
			while(1);
		}

	}

	wait();

	return 0;
}
