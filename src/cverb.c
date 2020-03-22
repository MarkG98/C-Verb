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
#include <netinet/in.h>

/* Header files */
#include "wav.h"

#define DELAY 600 // [ms]
#define DELAY_SAMPLES DELAY * (header->sample_rate) / 1000
#define NUM_SAMPLES header->data_size/2

/* Define constants for filters */
#define FF 1
#define FB 1

/*
* 	proccess_data takes the out file, the in and out buffers and the wav Header
*		It buffers a sample, retrieves that sample and then applies the comb filter to that sample.
*		The resulting proccessed sample is then stored in the out_file.
*/
void process_data (int16_t *inputBuff, int16_t *outputBuff, WaveHeader *header, FILE *out_file)
{
		int delayValue = DELAY_SAMPLES;
		int16_t to_load = 0;

		for (unsigned int i=delayValue; i < NUM_SAMPLES; i++)
		{
			//printf("Input Value: %d \n", inputBuff[i-delayValue]);
			//printf("Output Value: %d \n", ((int16_t)(FB*((float)inputBuff[i-delayValue]))));
			outputBuff[i] = (FF*inputBuff[i]) + ((int16_t)(FB*((float)inputBuff[i-delayValue])));
			//printf("Output Value :%d \n", outputBuff[i]);
			to_load = htons(outputBuff[i]);
			fwrite(&to_load, sizeof(to_load), 1, out_file);
		}
		//printf("FLOAT: %f\n", sample_out);
		//printf("INT: %d\n", (int16_t) sample_out);
}

int main (int argc, char *argv[])
{
		int16_t toBuffer = 0;
		int Bufferindex = 0;


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

		int16_t *inputBuffer = malloc(header->data_size);
		int16_t *outputBuffer = malloc(header->data_size);


		while (fread(&toBuffer, sizeof(toBuffer), 1, in_file) == 1)
		{
			//process_data(in_file, out_file, pBuff_in, pBuff_out, header);

			// Converts from litle endian to big endian
			//toBuffer = ((toBuffer & 0xFF) << 8) | ((toBuffer & 0xFF00) >> 8);
			inputBuffer[Bufferindex] = ntohs(toBuffer);
			Bufferindex++;
		}

		process_data(inputBuffer, outputBuffer, header, out_file);

		/* Not completly necessary to free here since the program
		   is about to end, but let's do it for practice */
		fclose(in_file);
		fclose(out_file);
		free(header);
		free(inputBuffer);
		free(outputBuffer);
}
