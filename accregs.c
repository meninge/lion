
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
	for(unsigned i=0; i<16; i++) {
		if(i > 0) printf(" ");
		printf("%08x", (unsigned)accreg_rd(i));
	}
	printf("\n");
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

