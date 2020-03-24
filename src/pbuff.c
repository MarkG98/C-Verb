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

void pbuff_put(ProcessingBuffer *pbuff, float val)
{
    pbuff->buffer[pbuff->head] = val;
}

float pbuff_get(ProcessingBuffer *pbuff, int *index)
{
    if (index == NULL)
    {
        return pbuff->buffer[pbuff->head];
    }
    return pbuff->buffer[*index];
}

int pbuff_get_length(ProcessingBuffer *pbuff)
{
    return pbuff->length;
}

int pbuff_get_head(ProcessingBuffer *pbuff)
{
    return pbuff->head;
}

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

void pbuff_free(ProcessingBuffer *pbuff)
{
    free(pbuff->buffer);
    free(pbuff);
}
