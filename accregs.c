
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

// Utility function to print the state of relevant IP registers
void accreg_print_regs() {
	unsigned i = 0;
	for(i = 0; i < 16; i++) {
		if(i > 0) printf(" ");
		printf("%08x", (unsigned)accreg_rd(i));
	}
	printf("\n");
}

// Use the 14th register to print the count of different FIFOs according to
// given bit mapping.
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

int32_t accregs_pop_r2()
{
	return (int32_t)accreg_rd(8);
}
int32_t accregs_pop_1r()
{
	return (int32_t)accreg_rd(8);
}
int32_t accregs_pop_rd()
{
	return (int32_t)accreg_rd(8);
}

void accregs_push_rd(uint32_t val)
{
	accreg_wr(9, val);
}

unsigned accreg_lvl1_max_fsize = 0;
unsigned accreg_lvl1_max_nbneu = 0;
unsigned accreg_lvl2_max_nbneu = 0;

unsigned accreg_lvl1_fsize = 0;
unsigned accreg_lvl1_nbneu = 0;
unsigned accreg_lvl2_nbneu = 0;

// Get IP configuration
void accreg_config_get() {
	unsigned r = 0;

	r = accreg_rd(0);
	accreg_lvl1_fsize     = r & 0x0FFFF;
	accreg_lvl1_max_fsize = r >> 16;

	r = accreg_rd(1);
	accreg_lvl1_nbneu     = r & 0x0FFFF;
	accreg_lvl1_max_nbneu = r >> 16;

	r = accreg_rd(2);
	accreg_lvl2_nbneu     = r & 0x0FFFF;
	accreg_lvl2_max_nbneu = r >> 16;

}

// Print IP configuration
void accreg_config_print() {
	printf("Summary of IP configuration:\n");
	printf("  Level 1, frame size .. %u, max %u\n", accreg_lvl1_fsize, accreg_lvl1_max_fsize);
	printf("  Level 1, neurons ..... %u, max %u\n", accreg_lvl1_nbneu, accreg_lvl1_max_nbneu);
	printf("  Level 2, neurons ..... %u, max %u\n", accreg_lvl2_nbneu, accreg_lvl2_max_nbneu);
}

