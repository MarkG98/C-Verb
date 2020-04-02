# DSP Implementation of Schroeder Reverb

## Team Members
**Mark Goldwater** and **Nathaniel Tan**

## Goals
Our lower bound goal was to record or source audio samples and apply post-processing effects as if it were a real-time signal. We planned to implement a reverb (echo) effect as well as other audio effects depending on how much time we had in the project.

Our stretch goal was to utilize a [Feather M4 board](https://www.adafruit.com/product/3857) to apply our DSP in real time. This would have been a port of the DSP process we used in our lower bound goal, but with different libraries that are relevant to the hardware, but we unfortunately did not have enough time to do this.

## Learning Goals

### C DSP Libraries
Our learning goals for this project were to learn about and experience low-level signal processing with C and learn how to use various C libraries that are used for DSP. Both of us have had experience doing DSP in MATLAB, and would like some experience writing DSP algorithms in C and potentially connect our algorithms to hardware using the [Feather M4 board](https://www.adafruit.com/product/3857) if time allows. By the end of the project, we intend to achieve at least the development of one guitar effect which will be applied to a prerecorded signal.

### Real-Time DSP
In addition, we wanted to gain experience with real-time signal processing which is why we want to handle the prerecorded signal as if it were real-time. This will allow for an easy transition between a pre-recorded signal and the hardware if we decide to achieve our stretch goal.

## End Result
By the end of the project, we implemented a Schroeder Reverb effect that takes an input mono-channel .wav file and outputs a .wav file with the reverb effect applied. An example output from processing the Cantina Band song from Star Wars can be heard [here](https://soundcloud.com/mark-goldwater/c-verb-cantina).

It sounds a bit metallic which papers we read on the algorithm warned us may be an effect. More tuning and optimization of the system parameters would likely help mitigate these effects if time alloted.

The .wav files we are able to process with our code contain single channel 16-bit/sample audio. This was the lower bound goal of our project. Due to the unfortunate circumstances surrounding this semester and COVID-19, we were unable to devote sufficient enough time to achieve our upper bound goal. If we had more time and were able to meet in person, we feel that the upper bound goal would have been much more reachable.

Despite this, we feel that we have achieved our learning goals. We did treat the audio we post-processed as if we were recieving it in real time by using a circular buffer in our implementation. This data structure is commonly used in real-time signal processing applications, and we were able to gain experience in using it and even implemented a variant on it (``pbuff.c``) for processing the received signal.

## System Design

### Software Architecture

We decided that a good way to start the project would be to write code that would take in a .wav file and then write the contents of that .wav file, unedited, to a new one. This would act as a stepping stone to ensure that we are able to reliably process the sound file type that we want to be able to add effects to. In order to do this, we utilized a [tutorial on truelogic.org](http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/) in order to parse the header data from the wave file and then implemented the reading of the actual audio data ourselves.

Moreover, in order to simulate receiving samples from an actual piece of hardware we load samples into a circular buffer from the wave file. Note that this is not completely necessary for our post-processing software implementation, but we made the design decision to include it in order to simulate hardware. In an actual hardware implementation, the circular buffer has the useful feature of overwriting old data when the buffer fills in order to ensure that the processing algorithms are receiving are the most recent. Then, we move samples from the circular buffer into a processing buffer, one-at-a-time, which is large enough to reach back 5 samples at appropriate delays for our filtering algorithm (architecture below). A rough architecture for our software flow of our audio samples can be seen below.

![Software Flow Diagram](../img/SoftwareFlow.png)

### DSP Architecture

One of the first things that we did in creating our software was a block diagram of out DSP architecture which would be used to add a reverb effect to the input signal. Our design is based on the reverb implementation that is detailed out in
the data sheet for the [TMS320C672x component](https://www.ti.com/lit/an/spraaa5/spraaa5.pdf) by Texas Instruments. Our block diagram can be seen below.

![DSP Arcitecture Block Diagram](../img/ShroederReverb.png)

Essentially, our architecture utilizes parallel Comb Filters to take a given sample from the wav file and add it together with six delayed samples where DELAY is about 32 ms. This output is then put through four All Pass Filters in series which pass all frequencies equally in gain, but change the phase relationships among the various frequencies. The spaces between the All Pass Filters are then tapped and these samples are added together to produce a single output value. Below are the difference equations we use for each filter (*Note: FF is the feed forward gain, and FB is the feedback gain*).

Comb Filter: **y[n] = FF * x[n] + FB * y[n-DELAY]**. \
All Pass Filter: **y[n] = - FF * x[n] + x[n-DELAY] + FB * y[n-DELAY]**.

### File Structure

Throughout the project we occasionally refactored our code into separate files to increase readability. For some code which we took from other sources, this was a clear choice to make. Having a separate file made it easy to include credit at the top of the relevant file. For our own code, we had to make intentional decisions to move code blocks to additional files.

For example, after we felt that our processing buffer code was stable, we made the design decision to factored it out into a separate ``pbuff.c`` and ``pbuff.h``, which reduced clutter in the main file. In ``pbuff.c``, we wrote wrapper functions which handled operations of the processing buffer struct such that in the main ``cverb.c`` file, we could simply invoke them.

We did this because our processing buffers were structured in a very object oriented-esque way, with a structure defining the processing buffer and a collection of functions associated with the structure. Moving all of this code to separate files made it easier to find and read the code we had written.

## Implementation Specifics

### Comb Filter

```C
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
```

The implementation of our comb filter can be seen above. The comb filter difference equation adds a scaled version of the current sample with a scaled version of a previous output, and it's difference equation is as below.

**y[n] = FF_C * x[n] + FB_C * y[n-DELAY]**

This function is applied iteratively to each sample *n*. With it, we first calculate the scaled version of the current sample and then apply as many comb filters as we have in parallel at once in the for loop. For each parellel comb filter we calculate the index in the processing buffer which would correspond to the proper delay in time. Note the constant ```DELAY_SAMPLES``` and all others are defined in the file ```constants.h```.

Next, we do a check on the index to see of the delayed sample has an index which is less than zero. If this is the case, we simply wrap around to the beginning of our buffer because it is circular.

Lastly, we simple get the sample at the proper index, scale it, and add it to our output variable: ```sample_out```. Then, the function returns.

### All Pass Filter

```C
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

```
The implementation of our all pass filter can be seen above. An all pass filter is a filter which creates a phase offset in the signal. It is implemented by the following difference equation and is in the latter half of our architecture:

**y[n] = - FF_A * x[n] + x[n-DELAY] + FB_A * y[n-DELAY]**

This function is applied iteratively at each sample *n* of our audio file as it is read out of the circular buffer and put into the processing buffer. First we calculate the index in the processing buffer which would correspond to the proper delay in time and account for a possible negative index value in the same way that we do with the comb filter. Then we simply assign ```output``` to the calculated difference equation, and the function returns.  

## Helpful Resources

- [.Wav file parsing tutorial on truelogic.org](http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/)
- [Circular buffer implementation](https://github.com/embeddedartistry/embedded-resources/tree/master/examples/c/circular_buffer)
- [Audio processing device data sheet from TI - has helpful block diagrams](https://www.ti.com/lit/an/spraaa5/spraaa5.pdf)
- [Shroeder reverberator paper - has a nice overview and difference equations](http://www.paulwittschen.com/files/schroeder_paper.pdf)

## Links: [GitHub](https://github.com/MarkG98/C-Verb), [Trello](https://trello.com/b/D3GjNhy6/c-verb)
*Note that although Mark made a lot of the commits, most of our work was done together via teletype followed by Mark pushing.*
