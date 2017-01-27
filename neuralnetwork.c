
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

//#define PRINT_DEBUG
//#define POP_RD
//#define POP_1R
//#define POP_R2
//#define MNIST

void nn_process_clear() {
	// Reset the accelerator
	accreg_clear();
	while(accreg_check_clear());
	// Get config
	//accreg_config_get();
	// Set dimensions
	accreg_lvl1_fsize = FSIZE;
	accreg_set_lvl1_fsize(FSIZE);
	accreg_lvl1_nbneu = NEU1;
	accreg_set_lvl1_nbneu(NEU1);
	accreg_lvl2_nbneu = NEU2;
	accreg_set_lvl2_nbneu(NEU2);
}

void nn_process_config() {
	// Reset the accelerator
	nn_process_clear();

	printf("DEBUT de config\n");

	int i = 0;
	int j = 0;
	int32_t *config_buffer_1_alloc = NULL, *config_buffer_1 = NULL;
	int32_t *config_buffer_2_alloc = NULL, *config_buffer_2 = NULL;
	int32_t *config_buffer_rec_alloc = NULL, *config_buffer_rec = NULL;
	const unsigned bufsize_1 = accreg_lvl1_nbneu * FSIZE * sizeof(*config_buffer_1) + 16 * 4;
	const unsigned bufsize_2 = accreg_lvl2_nbneu * accreg_lvl1_nbneu * sizeof(*config_buffer_2) + 16 * 4;
	const unsigned bufsize_rec = accreg_lvl1_nbneu * sizeof(*config_buffer_rec) + 16 * 4;

	// On alloue bufsize sur une adresse multiple de 16 * 4 octets.
	// On rajoute 16 * 4 pour pouvoir réaligner ensuite sans perdre de données.
	printf("bufsize_1 = %u\n", bufsize_1);
	printf("bufsize_2 = %u\n", bufsize_2);
	printf("bufsize_rec = %u\n", bufsize_rec);

	// Données MNIST de taille phénoménale, gigantesque, ces malloc vont-ils passer ?
	config_buffer_1_alloc = malloc_check(bufsize_1);
	config_buffer_2_alloc = malloc_check(bufsize_2);
	config_buffer_rec_alloc = malloc_check(bufsize_rec);

	printf("config_buffer_1_alloc = 0x%p\n", config_buffer_1_alloc);
	printf("config_buffer_2_alloc = 0x%p\n", config_buffer_2_alloc);
	printf("config_buffer_rec_alloc = 0x%p\n", config_buffer_rec_alloc);

	// Aller à la prochaine frontière des multiples de 16 * 4 octets.
	config_buffer_1 = (void *)uint_roundup((long)config_buffer_1_alloc, 16 * 4);
	config_buffer_2 = (void *)uint_roundup((long)config_buffer_2_alloc, 16 * 4);
	config_buffer_rec = (void *)uint_roundup((long)config_buffer_rec_alloc, 16 * 4);

#ifdef PRINT_DEBUG
	printf("config_buffer_1 = 0x%p\n", config_buffer_1);
	printf("config_buffer_2 = 0x%p\n", config_buffer_2);
	printf("config_buffer_rec = 0x%p\n", config_buffer_rec);

	// w1 contains given weights for level 1
	printf("config_buffer_1:\n");
#endif

	for (j = 0; j < accreg_lvl1_nbneu; j++) {
		for (i = 0; i < FSIZE; i++) {
#ifdef MNIST
			config_buffer_1[j * FSIZE + i] = w1[j][i / ROWS][i % COLUMNS];
#else
			config_buffer_1[j * FSIZE + i] = config_neu1[j][i];
#endif
#ifdef PRINT_DEBUG
			printf("poids L1: %d: %ld\n", j * FSIZE + i, (long)config_buffer_1[ j * FSIZE + i]);
#endif
		}
	}

	// w2 contains given weights for level 2
#ifdef PRINT_DEBUG
	printf("config_buffer_2:\n");
#endif

	for (j = 0; j < accreg_lvl2_nbneu; j++) {
		for (i = 0; i < accreg_lvl1_nbneu; i++) {
#ifdef MNIST
			config_buffer_2[j * accreg_lvl1_nbneu + i] = w2[j][i];
#else
			config_buffer_2[j * accreg_lvl1_nbneu + i] = config_neu2[j][i];
#endif
			#ifdef PRINT_DEBUG
			printf("poids L2: %d: %ld\n", j * accreg_lvl1_nbneu + i, (long)config_buffer_2[ j * accreg_lvl1_nbneu + i]);
			#endif
		}
	}
	// b1 contains given weights for recode level
#ifdef PRINT_DEBUG
	printf("config_buffer_rec:\n");
#endif
	for (i = 0; i < accreg_lvl1_nbneu; i++) {
		config_buffer_rec[i] = config_recode[i];
		/*
		 * MNIST application
		 */
		config_buffer_rec[i] = b1[i];
#ifdef PRINT_DEBUG
		printf("poids recode: %d: %ld\n", i, (long)config_buffer_rec[i]);
#endif
	}

	// Flush cached data to DDR memory
	Xil_DCacheFlush();

	//sleep(5);
	
	nn_process_clear();

	accregs_print_fifo_counts();

	print("Send config for recode level\n");
	// Write config for recode level
	accreg_set_wmode_rec12();
	accreg_wr(10, (uint32_t)config_buffer_rec);
	// 1 burst is 16 ;
	accreg_wr(12, MINIMUM_BURSTS(accreg_lvl1_nbneu));

#ifdef POP_RD
	for (i = 0; i < NEU1; i++) {
		// to get the fifo rdbuf out rdy
		unsigned register15 = (unsigned)accreg_rd(15);
		while (!(( 1 << 14) & register15)) {
			accregs_print_fifo_counts();
			usleep(10);
			register15 = (unsigned)accreg_rd(15);
		}
		printf("fifo_rdbuf poids rec %d: %ld\n", i, (long int)accregs_pop_rd());
	}
	printf("FINI\n");
#endif

	while(accreg_check_busyr());
	free(config_buffer_rec_alloc);
	accregs_print_fifo_counts();

	//sleep(1);

	print("Send config for level 1\n");
	nn_process_clear();
	// Write config for level 1
	accreg_set_wmode_lvl1();
	accreg_wr(10, (uint32_t)config_buffer_1);
	// 1 burst is 16 * 4 bytes.
	accreg_wr(12, MINIMUM_BURSTS(accreg_lvl1_nbneu * FSIZE));
	// recupération des poids dans la fifo rdbuf
#ifdef POP_RD
	for (i = 0; i < NEU1 * FSIZE; i++) {
		if (i%FSIZE == 0) {
			printf("NEU N°%d: \n", i/FSIZE);
			usleep(100);
		}
		// to get the fifo rdbuf out rdy
		unsigned register15 = (unsigned)accreg_rd(15);
		while (!(( 1 << 14) & register15)) {
			accregs_print_fifo_counts();
			usleep(10);
			register15 = (unsigned)accreg_rd(15);
		}
		printf("fifo_rdbuf poids L1 %d: %ld\n", i%FSIZE, (long int)accregs_pop_rd());
	}
	printf("FINI\n");
#endif

	accregs_print_fifo_counts();
	while(accreg_check_busyr());
	free(config_buffer_1_alloc);

	//sleep(5);

	print("Send config for level 2\n");
	nn_process_clear();
	// Write config for level 2
	accreg_set_wmode_lvl2();
	accreg_wr(10, (uint32_t)config_buffer_2);
	// 1 burst is 16 * 4 bytes.
	accreg_wr(12, MINIMUM_BURSTS(accreg_lvl1_nbneu * accreg_lvl2_nbneu));
#ifdef POP_RD
	for (i = 0; i < NEU1 * NEU2; i++) {
		if (i%NEU1 == 0) {
			printf("NEU N°%d: \n", i/NEU1);
			usleep(100);
		}
		// to get the fifo rdbuf out rdy
		unsigned register15 = (unsigned)accreg_rd(15);
		while (!(( 1 << 14) & register15)) {
			accregs_print_fifo_counts();
			usleep(10);
			register15 = (unsigned)accreg_rd(15);
		}
		printf("fifo_rdbuf poids L2 %d: %ld\n", i%NEU1, (long int)accregs_pop_rd());
	}
	printf("FINI\n");
#endif
	
	while(accreg_check_busyr());
	free(config_buffer_2_alloc);
	accregs_print_fifo_counts();

	//sleep(5);

	printf("FIN de config\n");

	// Reset the accelerator
	nn_process_clear();
}

void nn_process_frames() {
	printf("DEBUT d'envoie des frames\n");

	XTime oldtime;
	int i, j;

	// Reset the accelerator
	nn_process_clear();

	accregs_print_fifo_counts();

	uint32_t* frames_buffer_alloc = NULL;
	uint32_t* frames_buffer = NULL;

	int32_t* out_buffer_alloc = NULL;
	int32_t* out_buffer = NULL;

#ifdef MNIST
	int32_t max[FRAMES_NB] = {-100000};
	int32_t digit[FRAMES_NB] = {-1};
	uint32_t success_hits = 0;
#endif

	const unsigned frames_bufsize = FRAMES_NB * FSIZE * sizeof(*frames_buffer) + 16 * 4;
	frames_buffer_alloc = malloc_check(frames_bufsize);
	printf("frames_buffer_alloc = 0x%p\n", frames_buffer_alloc);
	frames_buffer = (void*)uint_roundup((long)frames_buffer_alloc, 16 * 4);
	printf("frames_buffer = 0x%p\n", frames_buffer);

	const unsigned out_bufsize = FRAMES_NB * NEU2 * sizeof(*out_buffer) + 16 * 4;
	out_buffer_alloc = malloc_check(out_bufsize);
	printf("out_buffer_alloc = 0x%p\n", out_buffer_alloc);
	out_buffer = (void*)uint_roundup((long)out_buffer_alloc, 16 * 4);
	printf("out_buffer = 0x%p\n", out_buffer);

#ifdef PRINT_DEBUG
	printf("frames_buffer:\n");
#endif
	for (j = 0; j < FRAMES_NB; j++) {
		for (i = 0; i < FSIZE; i++) {
#ifdef MNIST
			frames_buffer[j * FSIZE + i] = frames[j][i];
#else
			frames_buffer[j * FSIZE + i] = data_frames[j][i];
#endif
			#ifdef PRINT_DEBUG
			printf("frame %d pix %d: %lu\n", j, i, (unsigned long)frames_buffer[j * FSIZE + i]);
			#endif
		}
	}

#ifdef PRINT_DEBUG
	printf("out_buffer before bursts:\n");
#endif
	for (int i = 0; i < FRAMES_NB * NEU2; i++) {
		out_buffer[i] = 42;
#ifdef PRINT_DEBUG
		printf("outbuf before burst: %d: %ld\n", i, (long)out_buffer[i]);
#endif
	}


	// Flush cached data to DDR memory
	Xil_DCacheFlush();

	XTime_GetTime(&oldtime);

	// Set the number of results to get
	accreg_set_nboutputs(FRAMES_NB * NEU2);



	printf("Send frames:\n");
	// Send the frames, receive results
	accreg_set_wmode_frame();

	accreg_wr(10, (long)frames_buffer);
	accreg_wr(12, MINIMUM_BURSTS(FRAMES_NB * FSIZE));

	accreg_wr(11, (long)out_buffer);
	accreg_wr(13, MINIMUM_BURSTS(FRAMES_NB * NEU2));

#ifdef POP_RD
	for (i = 0; i < FRAMES_NB * FSIZE; i++) {
		// to get the fifo rdbuf out rdy
		unsigned register15 = (unsigned)accreg_rd(15);
		while (!(( 1 << 14) & register15)) {
			accregs_print_fifo_counts();
			usleep(10);
			register15 = (unsigned)accreg_rd(15);
		}
		printf("fifo_rdbuf frame %d: %ld\n", i, (long int)accregs_pop_rd());
	}
	printf("FINI\n");
#endif

#ifdef PRINT_DEBUG
	accregs_print_fifo_counts();
	sleep(5);
	accregs_print_fifo_counts();
	#endif

#ifdef POP_1R
	for (i = 0; i < NEU1 * FRAMES_NB; i++) {
		if (i%NEU1 == 0) {
			printf("FRAME N°%d: \n", i/NEU1);
			usleep(100);
		}
		// to get the fifo 1r out rdy
		unsigned register15 = (unsigned)accreg_rd(15);
		while (!(( 1 << 18) & register15)) {
			accregs_print_fifo_counts();
			usleep(10);
			register15 = (unsigned)accreg_rd(15);
		}
		printf("fifo_1r %d: %ld\n", i%NEU1, (long int)accregs_pop_1r());
	}
#endif 
#ifdef POP_R2
	for (i = 0; i < NEU1 * FRAMES_NB; i++) {
		if (i%NEU1 == 0) {
			printf("FRAME N°%d: \n", i/NEU1);
			usleep(100);
		}
		// to get the fifo 1r out rdy
		unsigned register15 = (unsigned)accreg_rd(15);
		while (!(( 1 << 22) & register15)) {
			accregs_print_fifo_counts();
			usleep(10);
			register15 = (unsigned)accreg_rd(15);
		}
		printf("fifo_r2 %d: %ld\n", i%NEU1, (long int)accregs_pop_r2());
	}
#endif 
#ifdef PRINT_DEBUG
	accregs_print_fifo_counts();
	accreg_print_regs();
#endif

	while(accreg_check_busyr());
	while(accreg_check_busyw());

#ifdef PRINT_DEBUG
	accregs_print_fifo_counts();
#endif

	double d = XTime_DiffCurrReal_Double(&oldtime);

	// Reset the accelerator
	nn_process_clear();

	printf("Time: %u frames done in %g seconds => %g frames/s\n", FRAMES_NB, d, FRAMES_NB/d);

	// Force the cache to get data from DDR
	Xil_DCacheInvalidateRange((unsigned)out_buffer, out_bufsize);

	sleep(1);

	printf("out_buffer after all:\n");

	for (int i = 0; i < FRAMES_NB * NEU2; i++) {
		printf("RES FRAME N°%d, res n°%d: %ld\n", i/NEU2, i%NEU2, (long)out_buffer[i]);
#ifdef MNIST
		if (out_buffer[i] > max[i / NEU2]) {
			max[i / NEU2] = out_buffer[i];
			digit[i / NEU2] = i % NEU2;
		}
#endif
	}
#ifdef MNIST
	for (int i = 0; i < FRAMES_NB; i++) {
		if (digit[i] == labels[i]) {
			success_hits++;
		}
	}
	printf("Taux de réussite : %.2f%%\n", (success_hits / (float)FRAMES_NB) * 100);
#endif

	printf("FIN d'envoi des frames\n");

	free(frames_buffer_alloc);
	free(out_buffer_alloc);
}
