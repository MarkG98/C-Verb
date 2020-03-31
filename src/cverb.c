/*
	Authors: Mark Goldwater, Nathaniel Tan

	This code processes a monochannel .wav file in order
	to add the effect of Schroeder Reverb to the sound. It
	outputs a file called 'C-Verb.wav'.

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
#include "pbuff.h"
#include "constants.h"

// Input circular buffer
cbuf_handle_t inputBuff;

/*
		Retrieves a sample from the global circular buffer.

		retreived: Pointer to variable which will
							 store the retrieved sample.
*/
void retrieve_sample (int16_t *retrieved)
{
		circular_buf_get(inputBuff, retrieved);
}

/*
		Reads sample from the .wav file and stores it
		in the circular buffer.

		in_file: File object for the input .wav file.
*/
void buffer_sample(FILE *in_file)
{
		int16_t toBuffer;

		fread(&toBuffer, sizeof(toBuffer), 1, in_file);

		circular_buf_put(inputBuff, toBuffer);
}


/*
		Filter which calculates the output of NUM_COMB_FILTERS comb filters
		for the sample that is marked by the head of the input buffer (pBuff_in).

		sample_out: Variable to hold output of parellel comb filters.
		pBuff_in: Processing buffer which stores input of comb filters.
		pBuff_out Processing buffer which stores output of comb filters.
		header: Struct that stores metadata of the sound file. Used in the #defined constants
		        (see in constants.h).
*/
void apply_comb_filter (float *sample_out, ProcessingBuffer *pBuff_in, ProcessingBuffer *pBuff_out, WaveHeader *header)
{
		int index;
		*sample_out = FF_C * pbuff_get(pBuff_in, NULL);

		// Each iteration of the for loop is applying a separate comb filter
		for (int i = 0; i < NUM_COMB_FILTERS; i++)
		{
			// Index takes into account the delay from the pBuff head
			index = pbuff_get_head(pBuff_out) - (int) ((i+1) * DELAY_SAMPLES);

			// If the index goes beyond the lower bound of the array, wrap to the end
			if(index < 0)
			{
				index = pbuff_get_length(pBuff_out) + index;
			}

			// Applies feedback
			*sample_out += FB_C * pbuff_get(pBuff_out, &index);
		}
}

/*
		Applies a singular all pass filter at the sample pointed to by the head of the
		input buffer.

		sample_out: Variable to hold output of parellel comb filters.
		pBuff_in: Processing buffer which stores input of comb filters.
		pBuff_out Processing buffer which stores output of comb filters.
		header: Struct that stores metadata of the sound file. Used in the #defined constants
						(see in constants.h).
*/
void apply_all_pass_filter(float *output, ProcessingBuffer *pBuff_in, ProcessingBuffer *pBuff_out, WaveHeader *header)
{
	int index;
	*output = 0;

	// Index takes into account the delay from the pBuff head
	index = pbuff_get_head(pBuff_out) - (int) (DELAY_SAMPLES);

	// If the index goes beyond the lower bound of the array, wrap to the end
	if(index < 0)
	{
		index = pbuff_get_length(pBuff_out) + index;
	}

	// Applies feedback
	*output += (-1)*FF_A*pbuff_get(pBuff_in, NULL);
	*output += pbuff_get(pBuff_in, &index);
	*output += FB_A*pbuff_get(pBuff_out, &index);
}


/*
  	Proccess_data takes the in and out files, the in and out processing buffers and the wav Header
		It buffers a sample, retrieves that sample and then applies the comb filter to that sample.
		The resulting proccessed sample is then stored in the out_file.

		in_file: Input .wav sound file.
		out_file: Output .wav sound file with processed data.
		pBuff_in: Circular processing buffer for the input data from in_file
		pBuff_comb_out: Circular processing buffer to store the output of the parellel comb filters.
		pBuff_all_pass_1: Circular processing buffer for the output of the first all pass filter.
		pBuff_all_pass_2: Circular processing buffer for the output of the second all pass filter.
		pBuff_all_pass_3: Circular processing buffer for the output of the third all pass filter.
		pBuff_all_pass_4: Circular processing buffer for the output of the fourth all pass filter.
		pBuff_out: Circular processing buffer for the output of the DSP system.
		header: Struct that stores metadata of the sound file. Used in the #defined constants
						(see in constants.h).
*/
void process_data (FILE *in_file, FILE *out_file, ProcessingBuffer *pBuff_in, ProcessingBuffer *pBuff_comb_out, ProcessingBuffer *pBuff_all_pass_1, ProcessingBuffer *pBuff_all_pass_2, ProcessingBuffer *pBuff_all_pass_3, ProcessingBuffer *pBuff_all_pass_4, ProcessingBuffer *pBuff_out, WaveHeader *header)
{
		int16_t retrieved;
		float sample_out;
		int16_t to_load;
		float comb_output;
		float all_pass_output_1, all_pass_output_2, all_pass_output_3, all_pass_output_4;

		// Buffering a sample and then immediately retrieving it seems unnecessary, but it is to simulate
		// getting samples from a live input (a guitar) instead of a wav file.
		buffer_sample(in_file);
		retrieve_sample(&retrieved);

		pbuff_put(pBuff_in, (float) retrieved);

		apply_comb_filter(&comb_output, pBuff_in, pBuff_comb_out, header);
		pbuff_put(pBuff_comb_out, comb_output);

		apply_all_pass_filter(&all_pass_output_1, pBuff_comb_out, pBuff_all_pass_1, header);
		pbuff_put(pBuff_all_pass_1, all_pass_output_1);

		apply_all_pass_filter(&all_pass_output_2, pBuff_all_pass_1, pBuff_all_pass_2, header);
		pbuff_put(pBuff_all_pass_2, all_pass_output_2);

		apply_all_pass_filter(&all_pass_output_3, pBuff_all_pass_2, pBuff_all_pass_3, header);
		pbuff_put(pBuff_all_pass_3, all_pass_output_3);

		apply_all_pass_filter(&all_pass_output_4, pBuff_all_pass_3, pBuff_all_pass_4, header);
		pbuff_put(pBuff_all_pass_4, all_pass_output_4);

		sample_out = comb_output + all_pass_output_1 + all_pass_output_2 + all_pass_output_3 + all_pass_output_4;

		pbuff_put(pBuff_out, sample_out);

		// Updates pointer to head in circular processing buffer (input)
		pbuff_update_head(pBuff_in);

		// Update pointer to head in all pass filter functions
		pbuff_update_head(pBuff_comb_out);
		pbuff_update_head(pBuff_all_pass_1);
		pbuff_update_head(pBuff_all_pass_2);
		pbuff_update_head(pBuff_all_pass_3);
		pbuff_update_head(pBuff_all_pass_4);

		// Updates pointer to head in circular processing buffer (output)
		pbuff_update_head(pBuff_out);

		to_load = (int16_t) sample_out;

		fwrite(&to_load, sizeof(to_load), 1, out_file);
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
		int16_t buffOnStack[CIRC_BUFF_SIZE];
		inputBuff = circular_buf_init(buffOnStack, CIRC_BUFF_SIZE);

		/* Instantiate in and out sample buffers*/
		ProcessingBuffer *pBuff_in = construct_processing_buffer(header);
		ProcessingBuffer *pBuff_out = construct_processing_buffer(header);
		ProcessingBuffer *pBuff_comb_out = construct_processing_buffer(header);
		ProcessingBuffer *pBuff_all_pass_1 = construct_processing_buffer(header);
		ProcessingBuffer *pBuff_all_pass_2 = construct_processing_buffer(header);
		ProcessingBuffer *pBuff_all_pass_3 = construct_processing_buffer(header);
		ProcessingBuffer *pBuff_all_pass_4 = construct_processing_buffer(header);


		while (!feof(in_file))
		{
			process_data(in_file, out_file, pBuff_in, pBuff_comb_out, pBuff_all_pass_1, pBuff_all_pass_2, pBuff_all_pass_3, pBuff_all_pass_4, pBuff_out, header);
		}

		// Close files
		fclose(in_file);
		fclose(out_file);
				
		/* Not completly necessary to free here since the program
		   is about to end, but let's do it for practice */
		free(header);
		pbuff_free(pBuff_in);
		pbuff_free(pBuff_out);
}
