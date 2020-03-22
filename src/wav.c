/*
	Authors: Mark Goldwater, Nathaniel Tan, amit (TrueLogic Blog Username)

	Base code for this file was created by LogicBlog user amit
	at http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/.
	This code is to process the header of a .wav file before the actual
	sound sampes are processed.
*/

/* Libraries */
#include <stdio.h>
#include <string.h>

/* Header files*/
#include "wav.h"

#define DEBUG

/* Function which reads through a wav file header

	 Reads wav file header and populates waveheader struct
	 and writes the header to a new wave file which will
	 contain the filtered signal.

	 in_file: wav file to be filtered
	 out_file: wav file to contain filtered data
*/
void parse_wav (FILE *in_file, FILE *out_file, WaveHeader *header)
{
	 int read = 0;
	 int write = 0;

	 // read header parts
	 unsigned char buffer4[4];
	 unsigned char buffer2[2];

	 read = fread(header->riff, sizeof(header->riff), 1, in_file);
	 write = fwrite(header->riff, sizeof(header->riff), 1, out_file);

	 #ifdef DEBUG
	 printf("(1-4): %s \n", header->riff);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 // convert little endian to big endian 4 byte int
	 header->overall_size  = buffer4[0] |
							(buffer4[1]<<8) |
							(buffer4[2]<<16) |
							(buffer4[3]<<24);

	 #ifdef DEBUG
	 printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header->overall_size, header->overall_size/1024);
	 #endif

	 read = fread(header->wave, sizeof(header->wave), 1, in_file);
	 write = fwrite(header->wave, sizeof(header->wave), 1, out_file);
	 #ifdef DEBUG
	 printf("(9-12) Wave marker: %s\n", header->wave);
	 #endif

	 read = fread(header->fmt_chunk_marker, sizeof(header->fmt_chunk_marker), 1, in_file);
	 write = fwrite(header->fmt_chunk_marker, sizeof(header->fmt_chunk_marker), 1, out_file);
	 #ifdef DEBUG
	 printf("(13-16) Fmt marker: %s\n", header->fmt_chunk_marker);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif
	 // convert little endian to big endian 4 byte integer
	 header->length_of_fmt = buffer4[0] |
								(buffer4[1] << 8) |
								(buffer4[2] << 16) |
								(buffer4[3] << 24);
	 #ifdef DEBUG
	 printf("(17-20) Length of Fmt header: %u \n", header->length_of_fmt);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);

	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header->format_type = buffer2[0] | (buffer2[1] << 8);
	 #ifdef DEBUG
	 char format_name[10] = "";
	 if (header->format_type == 1)
	   strcpy(format_name,"PCM");
	 else if (header->format_type == 6)
	  strcpy(format_name, "A-law");
	 else if (header->format_type == 7)
	  strcpy(format_name, "Mu-law");

	 printf("(21-22) Format type: %u %s \n", header->format_type, format_name);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header->channels = buffer2[0] | (buffer2[1] << 8);
	 #ifdef DEBUG
	 printf("(23-24) Channels: %u \n", header->channels);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 header->sample_rate = buffer4[0] |
							(buffer4[1] << 8) |
							(buffer4[2] << 16) |
							(buffer4[3] << 24);

	 #ifdef DEBUG
	 printf("(25-28) Sample rate: %u\n", header->sample_rate);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 header->byterate  = buffer4[0] |
							(buffer4[1] << 8) |
							(buffer4[2] << 16) |
							(buffer4[3] << 24);
	 #ifdef DEBUG
	 printf("(29-32) Byte Rate: %u , Bit Rate:%u\n", header->byterate, header->byterate*8);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header->block_align = buffer2[0] |
						(buffer2[1] << 8);
	 #ifdef DEBUG
	 printf("(33-34) Block Alignment: %u \n", header->block_align);
	 #endif

	 read = fread(buffer2, sizeof(buffer2), 1, in_file);
	 write = fwrite(buffer2, sizeof(buffer2), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u \n", buffer2[0], buffer2[1]);
	 #endif

	 header->bits_per_sample = buffer2[0] |
						(buffer2[1] << 8);
	 #ifdef DEBUG
	 printf("(35-36) Bits per sample: %u \n", header->bits_per_sample);
	 #endif

	 read = fread(header->data_chunk_header, sizeof(header->data_chunk_header), 1, in_file);
	 write = fwrite(header->data_chunk_header, sizeof(header->data_chunk_header), 1, out_file);
	 #ifdef DEBUG
	 printf("(37-40) Data Marker: %s \n", header->data_chunk_header);
	 #endif

	 read = fread(buffer4, sizeof(buffer4), 1, in_file);
	 write = fwrite(buffer4, sizeof(buffer4), 1, out_file);
	 #ifdef DEBUG
	 printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
	 #endif

	 header->data_size = buffer4[0] |
					(buffer4[1] << 8) |
					(buffer4[2] << 16) |
					(buffer4[3] << 24 );

	 #ifdef DEBUG
	 printf("(41-44) Size of data chunk: %u \n", header->data_size);
	 #endif
}
