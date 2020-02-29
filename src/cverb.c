/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

/* Header files */
#include "circular_buffer.h"

//#define DEBUG
#define CIRC_BUFF_SAMPLES 6

// Define Coeffecients
//#define

//  WAVE file header struct
typedef struct
{
	unsigned char riff[4];									// RIFF string
	unsigned int overall_size	;							// overall size of file in bytes
	unsigned char wave[4];									// WAVE string
	unsigned char fmt_chunk_marker[4];			// fmt string with trailing null char
	unsigned int length_of_fmt;							// length of the format data
	unsigned int format_type;								// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned int channels;									// no.of channels
	unsigned int sample_rate;								// sampling rate (blocks per second)
	unsigned int byterate;									// SampleRate * NumChannels * BitsPerSample/8
	unsigned int block_align;								// NumChannels * BitsPerSample/8
	unsigned int bits_per_sample;						// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header [4];		// DATA string or FLLR string
	unsigned int data_size;									// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} waveheader;

// Union for reading sample from .wav file
typedef union
{
	char intermediate[2];
	int16_t intermediate_sample;
} sample;

// Struct for processing buffer
typedef struct
{
	int16_t buffer[CIRC_BUFF_SAMPLES];
	int8_t head;
} processingBuffer;

sample sample_union;
waveheader header;
cbuf_handle_t inputBuff;
processingBuffer pBuff;


// Parses the wav file and fills a waveheader struct with the resulting information
// The code in this function was taken from http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
/* Function which reads through a wav file header

	 Reads wav file header and populates waveheader struct
	 and writes the header to a new wave file which will
	 contain the filtered signal.

	 in_file: wav file to be filtered
	 out_file: wav file to contain filtered data
*/
void parse_wav (FILE *in_file, FILE *out_file)
{
	 int read = 0;
	 int write = 0;

	 // read header parts
	 unsigned char buffer4[4];
	 unsigned char buffer2[2];

	 read = fread(header.riff, sizeof(header.riff), 1, in_file);
	 write = fwrite(header.riff, sizeof(header.riff), 1, out_file);

	 #ifdef DEBUG
	 printf("(1-4): %s \n", header.riff);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 // convert little endian to big endian 4 byte int
	 header.overall_size  = buffer4[0] |
							(buffer4[1]<<8) |
							(buffer4[2]<<16) |
							(buffer4[3]<<24);

	 #ifdef DEBUG
	 printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header.overall_size, header.overall_size/1024);
	 #endif

	 read = fread(header.wave, sizeof(header.wave), 1, in_file);
	 write = fwrite(header.wave, sizeof(header.wave), 1, out_file);
	 #ifdef DEBUG
	 printf("(9-12) Wave marker: %s\n", header.wave);
	 #endif

	 read = fread(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, in_file);
	 write = fwrite(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, out_file);
	 #ifdef DEBUG
	 printf("(13-16) Fmt marker: %s\n", header.fmt_chunk_marker);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif
	 // convert little endian to big endian 4 byte integer
	 header.length_of_fmt = buffer4[0] |
								(buffer4[1] << 8) |
								(buffer4[2] << 16) |
								(buffer4[3] << 24);
	 #ifdef DEBUG
	 printf("(17-20) Length of Fmt header: %u \n", header.length_of_fmt);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);

	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header.format_type = buffer2[0] | (buffer2[1] << 8);
	 #ifdef DEBUG
	 char format_name[10] = "";
	 if (header.format_type == 1)
	   strcpy(format_name,"PCM");
	 else if (header.format_type == 6)
	  strcpy(format_name, "A-law");
	 else if (header.format_type == 7)
	  strcpy(format_name, "Mu-law");

	 printf("(21-22) Format type: %u %s \n", header.format_type, format_name);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header.channels = buffer2[0] | (buffer2[1] << 8);
	 #ifdef DEBUG
	 printf("(23-24) Channels: %u \n", header.channels);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 header.sample_rate = buffer4[0] |
							(buffer4[1] << 8) |
							(buffer4[2] << 16) |
							(buffer4[3] << 24);

	 #ifdef DEBUG
	 printf("(25-28) Sample rate: %u\n", header.sample_rate);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 header.byterate  = buffer4[0] |
							(buffer4[1] << 8) |
							(buffer4[2] << 16) |
							(buffer4[3] << 24);
	 #ifdef DEBUG
	 printf("(29-32) Byte Rate: %u , Bit Rate:%u\n", header.byterate, header.byterate*8);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header.block_align = buffer2[0] |
						(buffer2[1] << 8);
	 #ifdef DEBUG
	 printf("(33-34) Block Alignment: %u \n", header.block_align);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header.bits_per_sample = buffer2[0] |
						(buffer2[1] << 8);
	 #ifdef DEBUG
	 printf("(35-36) Bits per sample: %u \n", header.bits_per_sample);
	 #endif

	 read = fread(header.data_chunk_header, sizeof(header.data_chunk_header), 1, in_file);
	 write = fwrite(header.data_chunk_header, sizeof(header.data_chunk_header), 1, out_file);
	 #ifdef DEBUG
	 printf("(37-40) Data Marker: %s \n", header.data_chunk_header);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 header.data_size = buffer4[0] |
					(buffer4[1] << 8) |
					(buffer4[2] << 16) |
					(buffer4[3] << 24 );

	 #ifdef DEBUG
	 printf("(41-44) Size of data chunk: %u \n", header.data_size);
	 #endif
}

void retrieve_sample (int16_t *retrieved)
{
		circular_buf_get(inputBuff, ((char*) retrieved));
		circular_buf_get(inputBuff, ((char*) retrieved)+1);
}

void buffer_sample(FILE *in_file, FILE *out_file)
{
		int read;
		int write;

		read = fread(&sample_union.intermediate_sample, sizeof(sample_union.intermediate), 1, in_file);

		sample_union.intermediate_sample = (sample_union.intermediate_sample << 8) | ((sample_union.intermediate_sample >> 8) & 0xFF);

		circular_buf_put(inputBuff, sample_union.intermediate[0]);
		circular_buf_put(inputBuff, sample_union.intermediate[1]);
}

void load_data (FILE *in_file, FILE*out_file)
{
		int16_t retrieved;

		buffer_sample(in_file, out_file);
		retrieve_sample(&retrieved);

		sample_union.intermediate_sample = retrieved;
		sample_union.intermediate_sample = (sample_union.intermediate_sample << 8) | ((sample_union.intermediate_sample >> 8) & 0xFF);
		fwrite(&sample_union.intermediate_sample, sizeof(sample_union.intermediate_sample), 1, out_file);

		pBuff.buffer[pBuff.head] = retrieved;
		if (pBuff.head < CIRC_BUFF_SAMPLES)
		{
			pBuff.head++;
		}
		else
		{
			pBuff.head = 0;
		}

		printf("%i",pBuff.buffer[0]);
}

int main (int argc, char *argv[])
{
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

    FILE *in_file = fopen(argv[0], "rb");
		FILE *out_file = fopen("C-Verb.wav", "w");

		parse_wav(in_file, out_file);

		uint8_t buffOnStack[CIRC_BUFF_SAMPLES * (header.bits_per_sample / 8)];
		inputBuff = circular_buf_init(buffOnStack, CIRC_BUFF_SAMPLES * (header.bits_per_sample / 8));

		// Initialize proessing buffer
		for (int i = 0; i < CIRC_BUFF_SAMPLES; i++)
		{
			pBuff.buffer[i] = 0;
		}
		pBuff.head = 0;

		while (!feof(in_file))
		{
			load_data(in_file, out_file);
		}
}
