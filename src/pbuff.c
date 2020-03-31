// Include libraries
#include <stdio.h>
#include <stdlib.h>

// Include header files
#include "pbuff.h"
#include "constants.h"

/*
  Function that contructs and returns a pointer to a ProcessingBuffer struct

  header: pointer to WaveHeader struct containing info on sound file

  returns: Pointer to ProcessingBuffer struct

*/
ProcessingBuffer *construct_processing_buffer(WaveHeader *header)
{
		ProcessingBuffer *new = malloc(sizeof(ProcessingBuffer));
		new->head = 0;
		new->memSize = sizeof(float) * PBUFF_LENGTH;
		new->length = PBUFF_LENGTH;
		new->buffer = malloc(new->memSize);

    /* Initialize processing buffer */
    for (int i = 0; i < new->length; i++)
    {
      new->buffer[i] = 0;
    }

		return new;
}

/*
		Puts value into the processing buffer at the location of the
		"head" attribute of the struct.

		pbuff: Pointer to ProcessingBuffer object.
		val: Value to put into buffer.
*/
void pbuff_put(ProcessingBuffer *pbuff, float val)
{
    pbuff->buffer[pbuff->head] = val;
}

/*
		Gets value from processing buffer at the location of
		the "head" attribute OR at the location of "index" if
		NULL is not passed.

		pbuff: Pointer to ProcessingBuffer object.
		index: Optional parameter to specify index to
		       get value from.
*/
float pbuff_get(ProcessingBuffer *pbuff, int *index)
{
    if (index == NULL)
    {
        return pbuff->buffer[pbuff->head];
    }
    return pbuff->buffer[*index];
}

/*
		Returns the length of the ProcessingBuffer object.

		pbuff: Pointer to ProcessingBuffer object.
*/
int pbuff_get_length(ProcessingBuffer *pbuff)
{
    return pbuff->length;
}

/*
		Returns the head (index) of the ProcessingBuffer object.

		pbuff: Pointer to ProcessingBuffer object.
*/
int pbuff_get_head(ProcessingBuffer *pbuff)
{
    return pbuff->head;
}

/*
		Updates the processing buffer's head by iterating it
		by one OR setting it to zero if it is currently on the
		last element in the array.

		pbuff: Pointer to ProcessingBuffer object.
*/
void pbuff_update_head(ProcessingBuffer *pbuff)
{
    if (pbuff->head < pbuff->length - 1)
    {
      pbuff->head++;
    }
    else
    {
      pbuff->head = 0;
    }
}

/*
		Frees processing buffer object and its contents.

		pbuff: Pointer to ProcessingBuffer object.
*/
void pbuff_free(ProcessingBuffer *pbuff)
{
    free(pbuff->buffer);
    free(pbuff);
}
