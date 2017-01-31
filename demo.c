#include "demo.h"
#include "neuralnetwork.h"

#define FRAMES_NB_DEMO 10

void start_demo()
{
	int frame = 0;
	int32_t digit[FRAMES_NB_DEMO] = {-1};

	nn_process_config();
	nn_process_frames_for_demo(digit);

	for (frame = 0; frame < FRAMES_NB_DEMO; frame++) {
		if (digit[frame] == labels[frame]) {

		}
	}

}

void display(int digit) {

	switch(digit) {

	case 0:

		break;
	case 1:

			break;
	case 2:

			break;
	case 3:

			break;
	case 4:

			break;
	case 5:

			break;
	case 6:

			break;
	case 7:

			break;
	case 8:

			break;
	case 9:

			break;
	}

}

void nn_process_frames_for_demo(int32_t* digit) {
	int i, j;

	// Reset the accelerator
	nn_process_clear();

	accregs_print_fifo_counts();

	uint32_t* frames_buffer_alloc = NULL;
	uint32_t* frames_buffer = NULL;

	int32_t* out_buffer_alloc = NULL;
	int32_t* out_buffer = NULL;


	int32_t max[FRAMES_NB_DEMO] = {-100000};

	const unsigned frames_bufsize = FRAMES_NB_DEMO * FSIZE * sizeof(*frames_buffer) + 16 * 4;
	frames_buffer_alloc = malloc_check(frames_bufsize);
	frames_buffer = (void*)uint_roundup((long)frames_buffer_alloc, 16 * 4);

	const unsigned out_bufsize = FRAMES_NB_DEMO * NEU2 * sizeof(*out_buffer) + 16 * 4;
	out_buffer_alloc = malloc_check(out_bufsize);
	out_buffer = (void*)uint_roundup((long)out_buffer_alloc, 16 * 4);

	for (j = 0; j < FRAMES_NB_DEMO; j++) {
		for (i = 0; i < FSIZE; i++) {
			frames_buffer[j * FSIZE + i] = frames[j][i];
		}
	}

	for (i = 0; i < FRAMES_NB_DEMO * NEU2; i++) {
		out_buffer[i] = 42;
	}

	// Flush cached data to DDR memory
	Xil_DCacheFlush();

	// Set the number of results to get
	accreg_set_nboutputs(FRAMES_NB_DEMO * NEU2);

	// Send the frames, receive results
	accreg_set_wmode_frame();

	accreg_wr(10, (long)frames_buffer);
	accreg_wr(12, MINIMUM_BURSTS(FRAMES_NB_DEMO * FSIZE));

	accreg_wr(11, (long)out_buffer);
	accreg_wr(13, MINIMUM_BURSTS(FRAMES_NB_DEMO * NEU2));

	while(accreg_check_busyr());
	while(accreg_check_busyw());

	// Reset the accelerator
	nn_process_clear();

	// Force the cache to get data from DDR
	Xil_DCacheInvalidateRange((unsigned)out_buffer, out_bufsize);


	for (i = 0; i < FRAMES_NB_DEMO * NEU2; i++) {
		result_hard[i / NEU2][i % NEU2] = out_buffer[i];

		out_buffer[i] += b2[i % NEU2];
		if (out_buffer[i] > max[i / NEU2]) {
			max[i / NEU2] = out_buffer[i];
			digit[i / NEU2] = i % NEU2;
		}
	}

	free(frames_buffer_alloc);
	free(out_buffer_alloc);
}
