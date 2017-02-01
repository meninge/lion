#ifndef _DATASET_H_
#define _DATASET_H_

/*
 * Comment this next line to use the data below instead of official MNIST frames
 * If you choose to use MNIST, you will have to import into the project the
 * MNIST data arrays frames and labels and give weights.
 * Make sure that the configuration matches with the hardware.
 */
#define MNIST

#ifndef MNIST

/*
 * User defined configuration
 * If you change FSIZE, NEU1 and NEU2, they have to be changed in the hardware
 * aswell.
 */
#define FRAMES_NB 2
#define FSIZE     64
#define NEU1      4
#define NEU2      3

// Weights for neurons layer 1
const int32_t config_neu1[NEU1][FSIZE] = {
	{1, -50, 10541, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -20},
	{2, 50, -1152, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -444},
	{3, 514, 453, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 666},
	{4, 1248, 14000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 777}
};

// Constants for recode after layer 1
const int32_t config_recode1[NEU1] = {1, 2, 3, 4};

// Weights for neurons layer 2
const int32_t config_neu2[NEU2][NEU1] = {
	{ 3, 1, 1, 1},
	{ 2, 1, 1, 1},
	{ 1, 1, 1, 1}
};

// Constants for recode after layer 2
const int32_t config_recode2[NEU2] = {0, 0, 0};

// Frames to send
const uint8_t data_frames[FRAMES_NB][FSIZE] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

#else

/*
 * MNIST data
 */

#define FRAMES_NB 1000
#define FSIZE     784
#define NEU1      100
#define NEU2      10

/*
 * Extern declarations of weights and constants.
 * They are in file net.c
 */
extern int16_t w1[NEU1][FSIZE];
extern int16_t w2[NEU2][NEU1];
extern int16_t b1[NEU1];
extern int16_t b2[NEU2];

/*
 * Extern declarations of frames and labels.
 * They are in file frames.c
 */
extern uint8_t frames[FRAMES_NB][FSIZE];
extern uint8_t labels[FRAMES_NB];

#endif

#endif /* _DATASET_H_ */
