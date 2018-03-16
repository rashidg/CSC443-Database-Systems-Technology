//free all allocated memory

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct record {
	int uid1;
	int uid2;
} Record;

typedef struct inputbuffer {
	Record *buffer;
	int capacity;
	int currPositionInBuffer;
	FILE *file;
} InputBuffer;

int cmp(const void * a, const void * b)
{
   Record *Ra = (Record *)a;
   Record *Rb = (Record *)b;
   return Ra->uid2 - Rb->uid2;
}
int cmp2(const void * a, const void * b)
{
   int *Ra = (int *)a;
   int *Rb = (int *)b;
   return *Ra - *Rb;
}

Record getNext(InputBuffer *IBs, int K)
{
	int minn, idx=-1; // index and IBs[idx].buffer[].uid2

	for (int i=0;i<K;i++)
		if (IBs[i].currPositionInBuffer != IBs[i].capacity)
		{
			idx = i;
			minn = IBs[i].buffer[IBs[i].currPositionInBuffer].uid2;
			break;
		}

	for (int i=0;i<K;i++)
	{
		if (IBs[i].currPositionInBuffer == IBs[i].capacity)
			continue; // We should have filled it. If it is still empty,
					  // then we probably finished this run file.
		if (IBs[i].buffer[IBs[i].currPositionInBuffer].uid2 < minn)
		{
			minn = IBs[i].buffer[IBs[i].currPositionInBuffer].uid2;
			idx = i;
		}
	}

	if (idx == -1)
	{
		Record err;
		err.uid2 = -1; err.uid1 = -1;
		return err;
	}

	// The record with minimum uid2 is in buffer #idx

	Record result = IBs[idx].buffer[IBs[idx].currPositionInBuffer++];
	if (IBs[idx].currPositionInBuffer == IBs[idx].capacity)
	{ // refill InputBuffer_idx
		IBs[idx].currPositionInBuffer = 0;
		IBs[idx].capacity = fread( 	IBs[idx].buffer,
									sizeof(Record),
									IBs[idx].capacity,
									IBs[idx].file );
	}
	return result;
}

int main(int argc, char *argv[]){
	
	if (argc != 5){
		printf("Usage: phaseI <name of the input file> <total mem in bytes> <block size> <number of runs>\n");
		return 0;
	}
	char *filename = argv[1];
	int total_memory = atoi(argv[2]);
	int block_size = atoi(argv[3]);
	int K = atoi(argv[4]);
	FILE *fp;
	fp = fopen("edges.dat", "rb");
	fseek(fp, 0L, SEEK_END);
	int filesize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	int N = filesize / sizeof(Record); // number of records in the file
	int M = total_memory / sizeof(Record); // # of records that fit in mem
	if (N > K * M)
	{
		printf("Not enough memory.\n");
		return 0;
	}

	// ------------------------- PHASE I (sort) ---------------------------------

	M = ceil((N*1.0)/(K*1.0)); // let each file get almost the same amount
							   // of memory. to have K files.
	int Nread, k = 0;
	Record *Records = malloc(M * sizeof(Record));
	while ( (Nread = (fread(Records, sizeof(Record), M, fp))) )
	{
		qsort(Records, Nread, sizeof(Record), cmp);
		/*char str[4];
		str[0] = (char)(k + 48);
		str[1] = '.';
		str[2] = 'x';
		str[3] = '\0';*/


		char str[32];   // Use an array which is large enough 
		snprintf(str, sizeof(str), "%i.x", k);

		FILE *out = fopen(str, "wb");
		fwrite(Records, sizeof(Record), Nread, out);
		fclose(out);
		k++;
	}
	fclose(fp);
	free(Records);
	// ------------------------- PHASE 2 (merge) --------------------------------
	total_memory = total_memory - (0.8) * 1024 * 1024;
	InputBuffer *IBs = malloc(K * sizeof(InputBuffer));

	int N_limit = total_memory / sizeof(Record);
	M = floor((N_limit*1.0)/(K*1.0+1)); // M records for each of K buffers
										 // +1 additional buffer for output
	for (int i=0;i<K;i++) // init and fill the input buffers
	{

		char str[32];   // Use an array which is large enough 
		snprintf(str, sizeof(str), "%i.x", i);

		IBs[i].file = fopen(str, "rb");
		IBs[i].currPositionInBuffer = 0;
		IBs[i].buffer = malloc(M * sizeof(Record));
		IBs[i].capacity = fread(IBs[i].buffer,
								sizeof(Record),
								M,
								IBs[i].file );
	}

	FILE *output = fopen("edges_sorted.dat", "wb");
	Record *outputBuffer = malloc(M * sizeof(Record));
	int output_position = 0;
	for (int rec=0;rec<N;rec++)
	{
		Record next = getNext(IBs, K);
		outputBuffer[output_position++] = next;
		if (output_position == M){
			output_position = 0;
			fwrite(outputBuffer, sizeof(Record), M, output);
			fflush(output);
		}
	}
	if (output_position > 0)
	{
		fwrite(outputBuffer, sizeof(Record), output_position, output);
		fflush(output);
	}

	free(outputBuffer);
	fclose(output);
	for (int i=0;i<K;i++)
	{
		free(IBs[i].buffer);
		fclose(IBs[i].file);

		char str[32];   // Use an array which is large enough 
		snprintf(str, sizeof(str), "%i.x", i);
		remove(str);
	}
	free(IBs);
	return 0;
}
