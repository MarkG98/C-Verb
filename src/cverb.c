/*
	Authors: Mark Goldwater, Nathaniel Tan

	Code to process a .wav file in order to add reverb.
*/

/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

/* Header files */
#include "circular_buffer.h"
#include "wav.h"

#define CIRC_BUFF_SAMPLES 6

// // Struct for processing buffer
// typedef struct
// {
// 	int16_t buffer[CIRC_BUFF_SAMPLES];
// 	int8_t head;
// } ProcessingBuffer;

cbuf_handle_t inputBuff;
// ProcessingBuffer pBuff;

void retrieve_sample (int16_t *retrieved)
{
		circular_buf_get(inputBuff, ((char*) retrieved));
		circular_buf_get(inputBuff, ((char*) retrieved)+1);
}

void buffer_sample(FILE *in_file, FILE *out_file)
{

		Sample toBuffer;

		fread(&toBuffer.intermediate_sample, sizeof(toBuffer.intermediate), 1, in_file);

		toBuffer.intermediate_sample = (toBuffer.intermediate_sample << 8) | ((toBuffer.intermediate_sample >> 8) & 0xFF);

		circular_buf_put(inputBuff, toBuffer.intermediate[0]);
		circular_buf_put(inputBuff, toBuffer.intermediate[1]);
}

void load_data (FILE *in_file, FILE *out_file)
{
		int16_t retrieved;
		Sample toLoad;

		buffer_sample(in_file, out_file);
		retrieve_sample(&retrieved);

		toLoad.intermediate_sample = retrieved;
		toLoad.intermediate_sample = (toLoad.intermediate_sample << 8) | ((toLoad.intermediate_sample >> 8) & 0xFF);
		fwrite(&toLoad.intermediate_sample, sizeof(toLoad.intermediate_sample), 1, out_file);

		// pBuff.buffer[pBuff.head] = retrieved;
		// if (pBuff.head < CIRC_BUFF_SAMPLES)
		// {
		// 	pBuff.head++;
		// }
		// else
		// {
		// 	pBuff.head = 0;
		// }
		//
		// printf("%i",pBuff.buffer[0]);
}

int main (int argc, char *argv[])
{

		/* Handle command line arguments */
		char ch;
    while ((ch = getopt(argc, argv, ":") != EOF)) {
        switch(ch) {
            default:
                fprintf(stderr, "invalid option '%s' \n", optarg);
                break;
        }
    }

    argc -= optind;
    argv += optind;

		/* Open input file and create a new output file for the procesed signal */
		FILE *in_file = fopen(argv[0], "rb");
		FILE *out_file = fopen("C-Verb.wav", "w");

		/* Instantiate struct to hold header data of .wav file */
		WaveHeader *header = malloc(sizeof(WaveHeader));

		/* Parse header data from the .wav file */
		parse_wav(in_file, out_file, header);

		/* Create instance of circular buffer */
		uint8_t buffOnStack[CIRC_BUFF_SAMPLES * (header->bits_per_sample / 8)];
		inputBuff = circular_buf_init(buffOnStack, CIRC_BUFF_SAMPLES * ((header->bits_per_sample) / 8));

		/* Instantiate processing buffer */
		// for (int i = 0; i < CIRC_BUFF_SAMPLES; i++)
		// {
		// 	pBuff.buffer[i] = 0;
		// }
		// pBuff.head = 0;

		while (!feof(in_file))
		{
			load_data(in_file, out_file);
		}

		/* Not completly necessary to free header here since the program
		   is about to end, but let's do it for practice */
		free(header);
}
