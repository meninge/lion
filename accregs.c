#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sleep.h>
#include <time.h>

#include "platform.h"
#include "xparameters.h"
#include "xuartps_hw.h"
#include "xtime_l.h"
#include "main.h"
#include "accregs.h"

volatile unsigned* accregs_ptr = (void*)0x43C00000;

/*
 * DEBUG
 * Utility function to print the state of relevant IP registers
 */
void accreg_print_regs()
{
	unsigned i = 0;

	for(i = 0; i < 16; i++) {
		if(i > 0) printf(" ");
		printf("%08x", (unsigned)accreg_rd(i));
	}
	printf("\n");
}

/*
 * DEBUG
 * Use the 14th register to print the count of different FIFOs according to
 * given bit mapping.
 */
void accregs_print_fifo_counts()
{
	unsigned register14 = (unsigned)accreg_rd(14);
	unsigned register15 = (unsigned)accreg_rd(15);

	printf("FIFO rd count : %u\n", (register14 & 0xFF000000) >> 24);
	printf("FIFO 1r count : %u\n", (register14 & 0x000000FF) >> 0);
	printf("FIFO r2 count : %u\n", (register14 & 0x0000FF00) >> 8);
	printf("FIFO 2o count : %u\n", (register14 & 0x00FF0000) >> 16);
	printf("FIFO wr count : %u\n", (register15 & 0x000000FF) >> 0);
}

/*
 * DEBUG
 * Pop the value from one of the FIFO. The according unused register, 8 here,
 * has to be granted this functionality in the hardware for the corresponding
 * FIFO to be poped after a read in this register.
 */
int32_t accregs_pop()
{
	return (int32_t)accreg_rd(8);
}
