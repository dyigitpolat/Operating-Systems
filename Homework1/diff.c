#include "stdlib.h"
#include "stdio.h"

struct Node
{
	char word[256];
	struct Node* next;

};


//function declarations
int strcmp( char*, char*);
char* strcpy( char*, char*);
void fileToList( struct Node*, FILE*);
void listToFile( struct Node*, FILE*);
void sortList( struct Node*);
void freeList( struct Node*);
void printList( struct Node*);
struct Node* setDifference( struct Node*, struct Node*);

//function definitions//
int main(int argc, char** args)
{
	struct Node* cur;
	struct Node* listA;
	struct Node* listB;
	struct Node* diffAB;
	FILE* in1;
	FILE* in2;
	FILE* out;

	if( argc == 4)
	{

		in1 = fopen( args[1], "r");
		in2 = fopen( args[2], "r");
		out = fopen( args[3], "w");

		if( in1)
			printf("input file one = %s\n", args[1]);
		else
			printf("input file one (%s) could not be read\n", args[1]);

		if( in2)
			printf("input file two = %s\n", args[2]);
		else
			printf("input file two (%s) could not be read\n", args[2]);

		printf("output file = %s\n", args[3]);

		if( !(in1 && in2))
		{
			printf( "one or more input files are missing. exiting program...\n");
			return 1;
		}
	}
	else
	{
		printf("ERROR: invalid arguments! exiting program.\n");
		printf("correct format:\n");
		printf("inputfile1 inputfile2 outputfile\n");
		return 1;
	}

	listA = (struct Node*) malloc(sizeof(struct Node));
	listB = (struct Node*) malloc(sizeof(struct Node));
	fileToList(listA, in1);
	fileToList(listB, in2);
	//printList(listA);
	//printList(listB);
	diffAB = setDifference(listA, listB);
	sortList(diffAB);
	printList( diffAB);
	listToFile(diffAB, out);


	// we should free the file pointers.
	fclose(in1);
	fclose(in2);
	fclose(out);

	//we better free the lists
	freeList(listA);
	freeList(listB);
	freeList(diffAB);

	return 0;
}

//returns A-B
struct Node* setDifference( struct Node* a, struct Node* b)
{
	struct Node* curA;
	struct Node* curB;
	struct Node* diff;
	struct Node* diffCur;

	curA = a;
	diff = (struct Node*) malloc( sizeof( struct Node));
	diffCur = diff;
	while(curA)
	{
		int flag = 0;

		curB = b;
		while(curB && !flag)
		{
			if( !strcmp(curA->word, curB->word))
				flag = 1;
			curB = curB->next;
		}

		if( !flag)
		{
			strcpy( diffCur->word, curA->word);
			diffCur->next = (struct Node*) malloc( sizeof( struct Node));
			diffCur = diffCur->next;
		}

		curA = curA->next;
	}

	diffCur->next = 0;
	return diff;
}

void fileToList( struct Node* mynode, FILE* myfile)
{
	struct Node* cur;
	char c;
	int i;

	cur = mynode;

	i = 0;
	while( (c = getc( myfile)) != EOF)
	{

		(cur->word)[i++] = c;

		if( c == '\n' || c == '\t' || c == ' ')
		{
			cur->next = (struct Node*) malloc( sizeof(struct Node));
			cur->word[--i] = 0;
			i = 0;
			cur = cur->next;
		}

	}

	cur->next = 0;
}

void listToFile( struct Node* mynode, FILE* myfile)
{
	struct Node* cur;

	cur = mynode;
	while(cur)
	{
		fputs( cur->word, myfile);
		putc( '\n', myfile);
		cur = cur->next;
	}
}

void sortList( struct Node* mynode)
{
	char tmp[256];
	struct Node* cur1;
	struct Node* cur2;
	struct Node* prev;
	struct Node* temp;

	prev = 0;
	cur1 = mynode;
	while( cur1)
	{
		cur2 = cur1;
		while( cur2->next)
		{
			//there is this little hack where im checking cur2 is the empty string or not.
			//otherwise strcpy wont copy the empty strings properly.
			if( strcmp(cur2->word, cur2->next->word) > 0 || cur2->word[0] == 0)
			{
				if( prev)
				{
					temp = cur2->next;
					cur2->next = temp->next;
					temp->next = cur2;
					prev->next = temp;
					cur2 = prev->next;
				}
				else
				{
						strcpy(tmp, cur2->word);
						strcpy(cur2->word, cur2->next->word);
						strcpy(cur2->next->word,tmp);
				}
			}

			prev = cur2;
			cur2 = cur2->next;
		}

		prev = cur1;
		cur1 = cur1->next;
	}

}

void freeList( struct Node* mynode )
{
	struct Node* prev;
	struct Node* cur;
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

void printList( struct Node* mynode)
{
	int i;
	struct Node* cur = mynode;

	i = 0;
	while( cur)
	{
		printf( "%d: %s", i++, cur->word);

		if( !cur->word[0])
			printf("(empty)");

		printf("\n");
		cur = cur->next;
	}

	printf("\n");
}
