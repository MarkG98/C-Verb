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

/* Define constants for filters */
#define FF 1.0
#define FB 1.0

// Struct for processing buffer
typedef struct
{
	float buffer[CIRC_BUFF_SAMPLES];
	int8_t head;
} ProcessingBuffer;

cbuf_handle_t inputBuff;
ProcessingBuffer pBuff_in;
ProcessingBuffer pBuff_out;

void retrieve_sample (int16_t *retrieved)
{
		circular_buf_get(inputBuff, ((char*) retrieved));
		circular_buf_get(inputBuff, ((char*) retrieved)+1);
}

void buffer_sample(FILE *in_file)
{
		Sample toBuffer;

		fread(&toBuffer.intermediate_sample, sizeof(toBuffer.intermediate), 1, in_file);

		toBuffer.intermediate_sample = (toBuffer.intermediate_sample << 8) | ((toBuffer.intermediate_sample >> 8) & 0xFF);

		circular_buf_put(inputBuff, toBuffer.intermediate[0]);
		circular_buf_put(inputBuff, toBuffer.intermediate[1]);
}

void apply_comb_filter (float *sample_out)
{
		*sample_out = FF * pBuff_in.buffer[pBuff_in.head];
		for (int i = 0; i < 5; i++)
		{
			*sample_out += FB * pBuff_out.buffer[i];
		}
}

void process_data (FILE *in_file, FILE *out_file)
{
		int16_t retrieved;
		float sample_out = 0.0;
		Sample toLoad;

		buffer_sample(in_file);
		retrieve_sample(&retrieved);

		pBuff_in.buffer[pBuff_in.head] = (float) retrieved;

		apply_comb_filter(&sample_out);

		pBuff_out.buffer[pBuff_out.head] = sample_out;

		if (pBuff_in.head < CIRC_BUFF_SAMPLES - 1)
		{
			pBuff_in.head++;
		}
		else
		{
			pBuff_in.head = 0;
		}

		if (pBuff_out.head < CIRC_BUFF_SAMPLES - 2)
		{
			pBuff_out.head++;
		}
		else
		{
			pBuff_out.head = 0;
		}

		toLoad.intermediate_sample = (int16_t) sample_out;
		toLoad.intermediate_sample = (toLoad.intermediate_sample << 8) | ((toLoad.intermediate_sample >> 8) & 0xFF);
		fwrite(&toLoad.intermediate_sample, sizeof(toLoad.intermediate_sample), 1, out_file);
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
		for (int i = 0; i < CIRC_BUFF_SAMPLES; i++)
		{
			pBuff_in.buffer[i] = 0;
		}
		pBuff_in.head = 0;

		/* Instantiate processing buffer */
		for (int i = 0; i < CIRC_BUFF_SAMPLES; i++)
		{
			pBuff_out.buffer[i] = 0;
		}
		pBuff_out.head = 0;

		while (!feof(in_file))
		{
			process_data(in_file, out_file);
		}

		/* Not completly necessary to free header here since the program
		   is about to end, but let's do it for practice */
		free(header);
}
