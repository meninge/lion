
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

#include "dataset.h"



void nn_process_clear() {
	// Reset the accelerator
	accreg_clear();
	while(accreg_check_clear());
	// Get config
	accreg_config_get();
	// Set dimensions
	accreg_lvl1_fsize = fsize;
	accreg_set_lvl1_fsize(fsize);
	accreg_lvl1_nbneu = neu1;
	accreg_set_lvl1_nbneu(neu1);
	accreg_lvl2_nbneu = neu2;
	accreg_set_lvl2_nbneu(neu2);
}

void nn_process_config() {

	// Reset the accelerator
	nn_process_clear();

	print("Send config for level 1\n");

	uint32_t* config_buffer_alloc = NULL;
	uint32_t* config_buffer = NULL;

	const unsigned bufsize = neu1 * fsize * sizeof(*config_buffer) + 32 * 4;
	config_buffer_alloc = malloc_check(bufsize);
	config_buffer = (void*)uint_roundup((long)config_buffer_alloc, 16*4);

	// FIXME Fill that buffer

	// Flush cached data to DDR memory
	Xil_DCacheFlush();

	// Write config for level 1
	accreg_set_wmode_lvl1();
	accreg_wr(10, (long)config_neu1);
	accreg_wr(12, bufsize / 16 / 4);
	while(accreg_check_busyr());

	// Reset the accelerator
	nn_process_clear();
}

void nn_process_frames() {
	XTime oldtime;

	// Reset the accelerator
	nn_process_clear();

	print("Send frames\n");

	uint32_t* frames_buffer_alloc = NULL;
	uint32_t* frames_buffer = NULL;

	uint32_t* out_buffer_alloc = NULL;
	uint32_t* out_buffer = NULL;

	const unsigned frames_bufsize = frames_nb * fsize * sizeof(*frames_buffer) + 32 * 4;
	frames_buffer_alloc = malloc_check(frames_bufsize);
	frames_buffer = (void*)uint_roundup((long)frames_buffer_alloc, 16*4);

	const unsigned out_bufsize = frames_nb * neu2 * sizeof(*out_buffer) + 32 * 4;
	out_buffer_alloc = malloc_check(out_bufsize);
	out_buffer = (void*)uint_roundup((long)out_buffer_alloc, 16*4);

	// FIXME Fill the buffer of frames

	// Flush cached data to DDR memory
	Xil_DCacheFlush();

	XTime_GetTime(&oldtime);

	// Set the number of results to get
	accreg_set_nboutputs(frames_nb * neu2);

	// Send the frames, receive results
	accreg_set_wmode_frame();
	accreg_wr(10, (long)frames_buffer);
	accreg_wr(12, frames_bufsize / 16 / 4);

	accreg_wr(11, (long)out_buffer);
	accreg_wr(13, out_bufsize / 16 / 4);

	while(accreg_check_busyr());
	while(accreg_check_busyw());

	double d = XTime_DiffCurrReal_Double(&oldtime);

	accreg_print_regs();

	// Reset the accelerator
	nn_process_clear();

	printf("Time: %u frames done in %g seconds => %g frames/s\n", frames_nb, d, frames_nb/d);

	print("Results for the first 10 frames:\n");

	// Force the cache to get data from DDR
	Xil_DCacheInvalidateRange((unsigned)out_buffer, frames_bufsize);

	unsigned outidx = 0;
	for(unsigned f=0; f<neu2; f++) {
		printf("Frame %u:", f);
		for(unsigned n=0; n<neu2; n++) printf(" %i", (int)out_buffer[outidx++]);
		printf("\n");
	}
}

