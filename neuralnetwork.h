#ifndef _NEURAL_NETWORK_H_
#define _NEURAL_NETWORK_H_

#include <stdint.h>
#include <stdbool.h>

#include "dataset.h"

void nn_process_clear();

void nn_process_config(int16_t *weights_level1, int16_t *weights_level2,
		int16_t *constants_recode);

void nn_process_frames(uint8_t *frames, uint32_t *results,
		int16_t *constants_recode_level2);
void nn_process_frames_for_demo(int32_t* digit);

void nn_hardware(uint8_t *frames, uint32_t *results,
		int16_t *weights_level1,
		int16_t *weights_level2,
		int16_t *constants_recode_level1,
		int16_t *constants_recode_level2);

void nn_software(uint8_t *frames, uint32_t *results,
		int16_t *weights_level1,
		int16_t *weights_level2,
		int16_t *constants_recode_level1,
		int16_t *constants_recode_level2);

int64_t cut(int64_t in);
void classify(uint32_t *results, uint32_t *classification);

/*
 * Timings
 */
extern double hard_time;
extern double soft_time;

enum level {
	LEVEL1,
	RECODE1,
	LEVEL2,
	RECODE2,
	LEVEL_NUMBER
};

void nn_config_level(void *weights, enum level level);

#endif /* _NEURAL_NETWORK_H_ */
