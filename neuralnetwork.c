
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
	int i = 0;

	// Reset the accelerator
	nn_process_clear();


	int32_t *config_buffer_1_alloc = NULL, *config_buffer_1 = NULL;
	int32_t *config_buffer_2_alloc = NULL, *config_buffer_2 = NULL;
	int32_t *config_buffer_rec_alloc = NULL, *config_buffer_rec = NULL;

	// Attention : bufsize doit être un multiple de 16 * 4 !
	const unsigned bufsize_1 = accreg_lvl1_nbneu * fsize * sizeof(*config_buffer_1) + 16 * 4;
	const unsigned bufsize_2 = accreg_lvl2_nbneu * accreg_lvl1_nbneu * sizeof(*config_buffer_2) + 16 * 4;
	const unsigned bufsize_rec = accreg_lvl1_nbneu * sizeof(*config_buffer_rec) + 16 * 4;
	// On alloue bufsize sur une adresse multiple de 16 * 4 octets.
	// On rajoute 16 * 4 pour pouvoir réaligner ensuite sans perdre de données.
	config_buffer_1_alloc = malloc_check(bufsize_1);
	config_buffer_2_alloc = malloc_check(bufsize_2);
	config_buffer_rec_alloc = malloc_check(bufsize_rec);
	// Aller à la prochaine frontière des multiples de 16 * 4 octets.
	config_buffer_1 = (void *)uint_roundup((long)config_buffer_1_alloc, 16 * 4);
	config_buffer_2 = (void *)uint_roundup((long)config_buffer_2_alloc, 16 * 4);
	config_buffer_rec = (void *)uint_roundup((long)config_buffer_rec_alloc, 16 * 4);

	printf("config_buffer_1 = 0x%p\n", config_buffer_1);
	printf("config_buffer_2 = 0x%p\n", config_buffer_2);
	printf("config_buffer_rec = 0x%p\n", config_buffer_rec);

	// w1 contains given weights for level 1
	for (i = 0; i < accreg_lvl1_nbneu * fsize; i++) {
		//config_buffer_1[i] = w1[i];
		config_buffer_1[i] = config_neu1[i / fsize][i % accreg_lvl1_nbneu];
	}
	// w2 contains given weights for level 2
	for (i = 0; i < accreg_lvl2_nbneu * accreg_lvl1_nbneu; i++) {
		//config_buffer_2[i] = w2[i];
		config_buffer_2[i] = config_neu2[i / accreg_lvl1_nbneu][i % accreg_lvl2_nbneu];
	}
	// b1 contains given weights for recode level
	for (i = 0; i < accreg_lvl1_nbneu; i++) {
		//config_buffer_rec[i] = b1[i];
		config_buffer_rec[i] = config_recode[i];
	}

	// Flush cached data to DDR memory
	Xil_DCacheFlush();

	//accreg_print_regs();
	accreg_clear();
	while (accreg_check_clear());
	accregs_print_fifo_counts();

	print("Send config for level 1\n");
	// Write config for level 1
	accreg_set_wmode_lvl1();
	accreg_wr(10, (uint32_t)config_buffer_1);
	// 1 burst is 16 * 4 bytes.
	accreg_wr(12, (bufsize_1 / 16 / 4) - 1);
	while(accreg_check_busyr()) {
		//accregs_print_fifo_counts();
	}

	//accreg_print_regs();
	accregs_print_fifo_counts();
	print("Send config for recode level\n");
	// Write config for recode level
	accreg_set_wmode_rec12();
	accreg_wr(10, (uint32_t)config_buffer_rec);
	// 1 burst is 16 * 4 bytes.
	accreg_wr(12, (bufsize_rec / 16 / 4));
	while(accreg_check_busyr());

	accregs_print_fifo_counts();
	print("Send config for level 2\n");
	// Write config for level 2
	accreg_set_wmode_lvl2();
	accreg_wr(10, (uint32_t)config_buffer_2);
	// 1 burst is 16 * 4 bytes.
	accreg_wr(12, (bufsize_2 / 16 / 4));
	while(accreg_check_busyr());
	accregs_print_fifo_counts();

	accreg_print_regs();

	// Reset the accelerator
	nn_process_clear();
}

void nn_process_frames() {
	XTime oldtime;
	int i;

	// Reset the accelerator
	nn_process_clear();

	print("Send frames\n");
	accregs_print_fifo_counts();

	uint32_t* frames_buffer_alloc = NULL;
	uint32_t* frames_buffer = NULL;

	uint32_t* out_buffer_alloc = NULL;
	uint32_t* out_buffer = NULL;

	const unsigned frames_bufsize = frames_nb * fsize * sizeof(*frames_buffer) + 16 * 4;
	frames_buffer_alloc = malloc_check(frames_bufsize);
	frames_buffer = (void*)uint_roundup((long)frames_buffer_alloc, 16*4);

	const unsigned out_bufsize = frames_nb * neu2 * sizeof(*out_buffer) + 16 * 4;
	out_buffer_alloc = malloc_check(out_bufsize);
	out_buffer = (void*)uint_roundup((long)out_buffer_alloc, 16*4);


	for (i = 0; i < frames_nb * fsize; i++) {
		frames_buffer[i] = data_frames[i / fsize][i % fsize];
		//frames_buffer[i] = frames[i];
	}
	for (int i = 0; i < frames_nb * neu2; i++) {
		out_buffer[i] = 120 - i;
	}

	// Flush cached data to DDR memory
	Xil_DCacheFlush();

	XTime_GetTime(&oldtime);

	// Set the number of results to get
	accreg_set_nboutputs(frames_nb * neu2);

	// Send the frames, receive results
	accreg_set_wmode_frame();
	accreg_wr(10, (long)frames_buffer);
	accreg_wr(12, frames_bufsize / 16 / 4);

	printf("out_buffer before burst:\n");
	for (int i = 0; i < frames_nb * neu2; i++) {
		printf("%d: %u\n", i, out_buffer[i]);
	}

	accreg_wr(11, (long)out_buffer);
	accreg_wr(13, out_bufsize / 16 / 4);

	printf("frames_buffer = 0x%p\n", frames_buffer);
	printf("out_buffer = 0x%p\n", out_buffer);
	printf("read bursts : %d\n", frames_bufsize / 16 / 4);
	printf("write bursts : %d\n", out_bufsize / 16 / 4);

	for (int i = 5; i >= 0; i--) {
		accregs_print_fifo_counts();
		accreg_print_regs();
	}
	printf("out_buffer after bursts:\n");
	for (int i = 0; i < frames_nb * neu2; i++) {
		printf("%d: %u\n", i, out_buffer[i]);
	}
	while(accreg_check_busyr());
	accregs_print_fifo_counts();
	while(accreg_check_busyw());
	accregs_print_fifo_counts();

	double d = XTime_DiffCurrReal_Double(&oldtime);

	accreg_print_regs();

	// Reset the accelerator
	nn_process_clear();

	printf("Time: %u frames done in %g seconds => %g frames/s\n", frames_nb, d, frames_nb/d);

	print("Results for the first 10 frames:\n");

	// Force the cache to get data from DDR
	Xil_DCacheInvalidateRange((unsigned)out_buffer, out_bufsize);

	/*
	unsigned outidx = 0;
	for(unsigned f=0; f<frames_nb; f++) {
		printf("Frame %u:", f);
		for(unsigned n=0; n<neu2; n++) printf(" %i", (int)out_buffer[outidx++]);
		//printf("Result: %d\n", labels[f]);
		printf("\n");
	}
	*/
	printf("out_buffer after all:\n");
	//for (int i = 0; i < frames_nb * neu2; i++) {
	for (int i = 0; i < 16; i++) {
		printf("%d: %u\n", i, out_buffer[i]);
	}
}

