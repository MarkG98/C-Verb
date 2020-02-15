# DSP for Guitar Peddle Effects

## Goals
Our lower bound goal is to record or source audio samples of a guitar and apply post-processing effects as if it were a real-time signal. We would  like to implement a reverb (echo) effect as well as one other depending on how much time we have left in the project.

If this goes well, our stretch goal is to utilize a [Feather M4 board](https://www.adafruit.com/product/3857) to apply our DSP in real time. This should be a port of the DSP process we used in our lower bound goal, but with different libraries that are relevant to the hardware.

## Learning Goals

### C DSP Libraries
Our learning goals for this project is to learn about and experience low-level signal processing with C and learn how to use various C libraries that are used for DSP. Both of us have had experience doing DSP in MATLAB, and would like some experience writing DSP algorithms in C and potentially connect our algorithms to hardware using the [Feather M4 board](https://www.adafruit.com/product/3857) if time allows. By the end of the project, we intend to achieve at least the development of one guitar effect which will be applied to a prerecorded signal.

### Real-Time DSP
In addition, we would like to gain experience with real-time signal processing which is why we want to handle the prerecorded signal as if it were real-time. This will allow for an easy transition between a pre-recorded signal and the hardware if we decide to achieve our stretch goal.

## First Steps

### Step 1: Design and Solidify a DSP Architecture for Reverb

**People:** Mark and Nathaniel\
**Definition of Done:** Create a drawn out block diagram of our effect's architecture including potential implementation methods.

### Step 2: Identify DSP Library Candidates

**People:** Mark and Nathaniel\
**Definition of Done:** Have a list of functionalities we need libraries for and a complementary list of libraries that implement those functionalities.

### Step 3: Figure out how to Load and Manipulate Sound Files in C

**People:** Mark\
**Definition of Done:** We are able to load and play a sound file in a C program.
