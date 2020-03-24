/* Define system constants */
#define CIRC_BUFF_SAMPLES 6 // Circular buffer capacity
#define CIRC_BUFF_SIZE CIRC_BUFF_SAMPLES * (header->bits_per_sample / 16) // Calculate size of circular buffer
#define DELAY 32 // Length of delay [ms]
#define FF 1.0 // Feedfoward gain
#define FB 0.8 // Feedback gain
#define PBUFF_LENGTH ((5 * DELAY * (header->sample_rate) / 1000) + 50) // Calculate length of proecessing buffer
#define DELAY_SAMPLES  DELAY * header->sample_rate / 1000 // Calculate amount of samples for one delay length
