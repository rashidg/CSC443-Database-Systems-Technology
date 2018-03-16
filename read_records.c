#include <stdio.h>
#include <stdlib.h>

typedef struct record {
	int uid1;
	int uid2;
} Record;

int main()
{
	//FILE *file = fopen("edges_sorted.dat", "rb");
	FILE *file = fopen("0.x", "rb");
	Record *R = malloc(sizeof(Record));
	int N = 85331845;
	for (int i=0;i<N;i++)
	{
		fread(R, sizeof(Record), 1, file);
	}
	printf("85331845: %d,%d\n", R->uid1, R->uid2);
	free(R);
	return 0;
}