#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sleep.h>
#include <time.h>

#include "platform.h"
#include "xparameters.h"
#include "xuartps_hw.h"
#include "xtime_l.h"

#include "xil_cache.h"
#include "xil_cache_l.h"

#include "main.h"
#include "neuralnetwork.h"
#include "accregs.h"

double hard_time = 0;
double soft_time = 0;

/*
 * Compute the results with the IP
 * WARNING: arrays sizes must match the following:
 *	- frames: uint8_t[FRAMES_NB][FSIZE]
 *	- results: uint32_t[FRAMES_NB][NEU2]
 *	- weights_level1: int32_t[NEU1][FSIZE]
 *	- weights_level2: int32_t[NEU2][NEU1]
 *	- constants_recode_level1: int32_t[NEU1]
 *	- constants_recode_level2: int32_t[NEU2]
 */
void nn_hardware(uint8_t **frames, uint32_t **results,
		int32_t **weights_level1,
		int32_t **weights_level2,
		int32_t *constants_recode_level1,
		int32_t *constants_recode_level2)
{
	nn_process_config(weights_level1, weights_level2,
			constants_recode_level1);
	nn_process_frames(frames, results, constants_recode_level2);
}

/*
 * Compute the results only with the software, without using the IP.
 * We use this as a reference.
 * WARNING: arrays sizes must match the following:
 *	- frames: uint8_t[FRAMES_NB][FSIZE]
 *	- results: uint32_t[FRAMES_NB][NEU2]
 *	- weights_level1: int32_t[NEU1][FSIZE]
 *	- weights_level2: int32_t[NEU2][NEU1]
 *	- constants_recode_level1: int32_t[NEU1]
 *	- constants_recode_level2: int32_t[NEU2]
 */
void nn_software(uint8_t **frames, uint32_t **results,
		int32_t **weights_level1,
		int32_t **weights_level2,
		int32_t *constants_recode_level1,
		int32_t *constants_recode_level2)
{
	int64_t out1[NEU1] = {0};
	int64_t out2[NEU2] = {0};
	int32_t i, n, image;
	XTime oldtime;

	XTime_GetTime(&oldtime);
	for (image = 0; image < FRAMES_NB; image++) {
		/*
		 * Initialization
		 */
		for (n = 0; n < NEU1; n++) {
			out1[n] = 0;
		}
		for (n = 0; n < NEU2; n++) {
			out2[n] = 0;
		}
		/*
		 * First layer computation
		 */
		for (i = 0; i < FSIZE; i++) {
			for (n = 0; n < NEU1; n++) {
				out1[n] +=
					frames[image][i] * weights_level1[n][i];
			}
		}
		/*
		 * Cut to 32 bits values and apply activation function
		 */
		for (n = 0; n < NEU1; n++) {
			out1[n] = cut(out1[n]);
			out1[n] += constants_recode_level1[n];
			out1[n] = (out1[n] > 0) ? out1[n] : 0;
			out1[n] = cut(out1[n]);
		}
		/*
		 * Second layer computation
		 */
		for (i = 0; i < NEU1; i++) {
			for (n = 0; n < NEU2; n++) {
				out2[n] += out1[i] * weights_level2[n][i];
			}
		}
		/*
		 * Apply second layer constants.
		 */
		for (n = 0; n < NEU2; n++) {
			out2[n] = cut(out2[n]);
			out2[n] += constants_recode_level2[n];
			out2[n] = (out2[n] > 0) ? out2[n] : 0;
			out2[n] = cut(out2[n]);
			results[image][n] = out2[n];
		}
	}
	soft_time = XTime_DiffCurrReal_Double(&oldtime);
}

/*
 * Send RESET signal to the IP.
 */
void nn_process_clear()
{
	// Reset the accelerator
	accreg_clear();
	while(accreg_check_clear());
}

/*
 * Configure the whole network.
 * WARNING: array sizes must match the following:
 *	- weights_level1: int32_t[NEU1][FSIZE]
 *	- weights_level2: int32_t[NEU2][NEU1]
 *	- constants_recode_level1: int32_t[NEU1]
 */
void nn_process_config(int32_t **weights_level1, int32_t **weights_level2,
		int32_t *constants_recode_level1)
{
	nn_config_level(weights_level1, LEVEL1);
	nn_config_level(weights_level2, LEVEL2);
	nn_config_level(constants_recode_level1, RECODE1);
}

/*
 * Send the frames to the IP and get back the results.
 * We need to pass the second layer recode constants to add them in the
 * software as there is not hardware layer for that.
 * WARNING: arrays sizes must match the following:
 *	- frames: uint8_t[FRAMES_NB][FSIZE]
 *	- results: uint32_t[FRAMES_NB][NEU2]
 *	- constants_recode_level2: uint32_t[NEU2]
 */
void nn_process_frames(uint8_t **frames, uint32_t **results,
		int32_t *constants_recode_level2)
{
	XTime oldtime;
	int i, j;
	uint32_t* frames_buffer_alloc = NULL;
	uint32_t* frames_buffer = NULL;
	int32_t* out_buffer_alloc = NULL;
	int32_t* out_buffer = NULL;
	const unsigned frames_bufsize =
		FRAMES_NB * FSIZE * sizeof(*frames_buffer) +
		16 * 4;
	const unsigned out_bufsize = FRAMES_NB * NEU2 * sizeof(*out_buffer) +
		16 * 4;

	nn_process_clear();
	frames_buffer_alloc = malloc_check(frames_bufsize);
	out_buffer_alloc = malloc_check(out_bufsize);
	frames_buffer = (void*)uint_roundup((long)frames_buffer_alloc, 16 * 4);
	out_buffer = (void*)uint_roundup((long)out_buffer_alloc, 16 * 4);
	for (j = 0; j < FRAMES_NB; j++) {
		for (i = 0; i < FSIZE; i++) {
			frames_buffer[j * FSIZE + i] = ((uint8_t *)frames)[j * FSIZE + i];
		}
	}
	// Flush cached data to DDR memory
	Xil_DCacheFlush();
	XTime_GetTime(&oldtime);

	// Set the number of results to get
	accreg_set_nboutputs(FRAMES_NB * NEU2);

	// Send the frames, receive results
	accreg_set_wmode_frame();

	accreg_wr(10, (long)frames_buffer);
	accreg_wr(12, MINIMUM_BURSTS(FRAMES_NB * FSIZE));

	accreg_wr(11, (long)out_buffer);
	accreg_wr(13, MINIMUM_BURSTS(FRAMES_NB * NEU2));

	while(accreg_check_busyr());
	while(accreg_check_busyw());

	// Force the cache to get data from DDR
	Xil_DCacheInvalidateRange((unsigned)out_buffer, out_bufsize);

	/*
	 * This is not done in the IP so we do it in the software.
	 */
	for (i = 0; i < FRAMES_NB * NEU2; i++) {
		out_buffer[i] += constants_recode_level2[i % NEU2];
		out_buffer[i] = (out_buffer[i] > 0) ? out_buffer[i] : 0;
	}
	hard_time = XTime_DiffCurrReal_Double(&oldtime);

	for (j = 0; j < FRAMES_NB; j++) {
		for (i = 0; i < NEU2; i++) {
			results[j][i] = out_buffer[j * NEU2 + i];
		}
	}

	free(frames_buffer_alloc);
	free(out_buffer_alloc);
	nn_process_clear();
}

/*
 * Cut and keep only the 32 low-order bits from in.
 * Keep signed bits for 32 high-order bits.
 */
int64_t cut(int64_t in)
{
	bool negative;

	negative = (in < 0) ? true : false;
	if (negative)
		in = -in;
	in &= 0xFFFFFFFF;
	if (negative)
		in = -in;

	return in;
}

/*
 * Configure the given level.
 * level argument must be LEVEL1, LEVEL2 or RECODE1.
 * Possible types of weights argument:
 *   - LEVEL1: int16_t[NEU1][FSIZE]
 *   - LEVEL2: int16_t[NEU2][NEU1]
 *   - RECODE1: int16_t[NEU1]
 */
void nn_config_level(void *weights, enum level level)
{
	int i, j;
	int32_t *config_buffer_alloc = NULL, *config_buffer = NULL;
	size_t bufsize;

	// On rajoute 16 * 4 pour pouvoir réaligner ensuite sans perdre de
	// données.
	switch (level) {
	case LEVEL1:
		bufsize = NEU1 * FSIZE * sizeof(int32_t) + 16 * 4;
		break;
	case LEVEL2:
		bufsize = NEU2 * NEU1 * sizeof(int32_t) + 16 * 4;
		break;
	case RECODE1:
		bufsize = NEU1 * sizeof(int32_t) + 16 * 4;
		break;
	default:
		abort_printf("configuring an undefined level\n");
		break;
	}
	nn_process_clear();
	config_buffer_alloc = malloc_check(bufsize);
	/*
	 * On réaligne la zone mémoire allouée sur une frontière de 16 * 4
	 * octets avant de la remplir pour la préparer aux bursts.
	 */
	config_buffer = (void *)uint_roundup((long)config_buffer_alloc, 16 * 4);

	switch (level) {
	case LEVEL1:
		for (j = 0; j < NEU1; j++) {
			for (i = 0; i < FSIZE; i++) {
				config_buffer[j * FSIZE + i] =
					((int16_t *)weights)[j * FSIZE + i];
			}
		}
		break;
	case LEVEL2:
		for (j = 0; j < NEU2; j++) {
			for (i = 0; i < NEU1; i++) {
				config_buffer[j * NEU1 + i] =
					((int16_t *)weights)[j * FSIZE + i];
			}
		}
		break;
	case RECODE1:
		for (i = 0; i < NEU1; i++) {
			config_buffer[i] = ((int16_t *)weights)[i];
		}
		break;
	default:
		abort_printf("configuring an undefined level\n");
		break;
	}
	Xil_DCacheFlush();
	nn_process_clear();
	accreg_wr(10, (uint32_t)config_buffer);
	/*
	 * L'écriture dans le registre 12 déclenche le burst.
	 */
	switch (level) {
	case LEVEL1:
		accreg_set_wmode_lvl1();
		accreg_wr(12, MINIMUM_BURSTS(NEU1 * FSIZE));
		break;
	case LEVEL2:
		accreg_set_wmode_lvl2();
		accreg_wr(12, MINIMUM_BURSTS(NEU1 * NEU2));
		break;
	case RECODE1:
		accreg_set_wmode_rec12();
		accreg_wr(12, MINIMUM_BURSTS(NEU1));
		break;
	default:
		abort_printf("configuring an undefined level\n");
		break;
	}
	while(accreg_check_busyr());
	free(config_buffer_alloc);
	nn_process_clear();
}

/*
 * Classify the given results ie,or each frame computes the selected neuron.
 * WARNING: arrays sizes must match the following:
 *	- results: uint32_t[FRAMES_NB][NEU2]
 *	- classification: uint32_t[FRAMES_NB]
 */
void classify(uint32_t **results, uint32_t *classification)
{
	int i, j;
	uint32_t max[FRAMES_NB] = {-1};

	for (i = 0; i < FRAMES_NB; i++) {
		for (j = 0; j < NEU2; j++) {
			if (results[i][j] > max[i]) {
				max[i] = results[i][j];
				classification[i] = j;
			}
		}
	}
}
