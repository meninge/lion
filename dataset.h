#ifndef _DATASET_H_
#define _DATASET_H_

/*
 * DEMO VERSION : only use 10 digits in frame.c
 *
 * Comment this next line to use the data below instead of official MNIST frames
 * If you choose to use MNIST, you will have to import into the project the
 * MNIST data arrays frames and labels and give weights.
 * Make sure that the configuration matches with the hardware.
 */

/*
 * MNIST data
 */

#define FRAMES_NB 10
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

#endif /* _DATASET_H_ */
