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

/*
		Puts value into the processing buffer at the location of the
		"head" attribute of the struct.

		pbuff: Pointer to ProcessingBuffer object.
		val: Value to put into buffer.
*/
void pbuff_put(ProcessingBuffer *pbuff, float val);

/*
		Gets value from processing buffer at the location of
		the "head" attribute OR at the location of "index" if
		NULL is not passed.

		pbuff: Pointer to ProcessingBuffer object.
		index: Optional parameter to specify index to
		       get value from.
*/
float pbuff_get(ProcessingBuffer *pbuff, int *index);

/*
		Returns the length of the ProcessingBuffer object.

		pbuff: Pointer to ProcessingBuffer object.
*/
int pbuff_get_length(ProcessingBuffer *pbuff);

/*
		Returns the head (index) of the ProcessingBuffer object.

		pbuff: Pointer to ProcessingBuffer object.
*/
int pbuff_get_head(ProcessingBuffer *pbuff);

/*
		Updates the processing buffer's head by iterating it
		by one OR setting it to zero if it is currently on the
		last element in the array.

		pbuff: Pointer to ProcessingBuffer object.
*/
void pbuff_update_head(ProcessingBuffer *pbuff);

/*
		Frees processing buffer object and its contents.

		pbuff: Pointer to ProcessingBuffer object.
*/
void pbuff_free(ProcessingBuffer *pbuff);
