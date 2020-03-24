// Include libraries
#include <stdio.h>

#include "wav.h"

/* Struct to define a processing buffer object */
typedef struct
{
	float *buffer;
	int memSize; // Amout of bytes allocated for the buffer
	int length;  // Amount of indexes the buffer has
	int head;
} ProcessingBuffer;

/*
  Function that contructs and returns a pointer to a ProcessingBuffer struct

  header: pointer to WaveHeader struct containing info on sound file

  returns: Pointer to ProcessingBuffer struct

*/
ProcessingBuffer *construct_processing_buffer(WaveHeader *header);

void pbuff_put(ProcessingBuffer *pbuff, float val);

float pbuff_get(ProcessingBuffer *pbuff, int *index);

int pbuff_get_length(ProcessingBuffer *pbuff);

int pbuff_get_head(ProcessingBuffer *pbuff);

void pbuff_update_head(ProcessingBuffer *pbuff);

void pbuff_free(ProcessingBuffer *pbuff);
