
#include <stdint.h>
#include <stdbool.h>

#include "dataset.h"


void nn_process_clear();

void nn_process_config();
void nn_process_frames();
void nn_process_frames_for_demo(int32_t* digit);


void nn_soft(void);
int64_t cut(int64_t in);

/*
 * Results per frame per neuron
 */
extern int32_t result_hard[FRAMES_NB][NEU2];
extern int32_t result_soft[FRAMES_NB][NEU2];
extern double hard_time;
extern double soft_time;

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
