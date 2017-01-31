
#include <stdint.h>
#include <stdbool.h>


void nn_process_clear();

void nn_process_config();
void nn_process_frames();

void nn_soft(void);

// To be used for MNIST application
#define ROWS 28
#define COLUMNS 28
/*
 * Extern declarations of weights and constants.
 */
extern int16_t w1[100][28][28];
extern int16_t w2[10][100];
extern int16_t b1[100];
extern int16_t b2[10];

/*
 * Extern declarations of frames and labels.
 */
extern uint8_t frames[1000][784];
extern uint8_t labels[1000];
