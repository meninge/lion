/* Neural network application
 * Hardware accelerator that does pipelined NN
 *
 * Note: The DDR mem is needed to hold the config of NN layers and dataset
 * => Correctly configure the ldscript to use full DDR as heap etc
 */

#include <stdio.h>
#include <stdlib.h>
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
#include "demo.h"


double XTime_ToDouble(XTime *t) {
	return ((double)(*t)) / COUNTS_PER_SECOND;
}
double XTime_GetDiff_Double(XTime *oldtime, XTime *newtime) {
	return ((double)(*newtime - *oldtime)) / COUNTS_PER_SECOND;
}

double XTime_DiffCurrReal_Double(XTime *oldtime) {
	XTime newtime;
	XTime_GetTime(&newtime);
	return XTime_GetDiff_Double(oldtime, &newtime);
}



void* malloc_with_loc(unsigned size, char* file, unsigned line) {
	void* ptr = malloc(size);
	if(ptr==NULL) abort_printf("From %s:%u : malloc() returned NULL for size %u\n", file, line, size);
	return ptr;
}



int main() {
	int image, neuron;

	init_platform();

	print("Hello World\n");
	print("Waiting for 3s to let user connect UART...");
	sleep(3);
	print(" done\n");

	print("Explicitely enabling UART Rx and Tx, no parity, no interrupts...\n");
	// Explicitely enable Rx ad Tx
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET, XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN);
	// No parity
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_MR_OFFSET, XUARTPS_MR_CHMODE_NORM | XUARTPS_MR_PARITY_NONE);
	// Disable interrupts
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_IDR_OFFSET, XUARTPS_IXR_MASK);

#ifdef DEMO
	start_demo();
#else
	// Launch main computing
	nn_process_config();
	nn_process_frames();

#endif
	// Launch computing on software only
	nn_soft();
	for (image = 0; image < FRAMES_NB; image++) {
		for (neuron = 0; neuron < NEU2; neuron++) {
			if (result_soft[image][neuron] != result_hard[image][neuron]) {
				printf("WARNING: difference soft/hard\n");
				printf("soft: image°%d, neurone n°%d : %d\n", image, neuron,
						result_soft[image][neuron]);
				printf("hard: image°%d, neurone n°%d : %d\n", image, neuron,
						result_hard[image][neuron]);
			}
		}
	}
	printf("%u frames\n:", FRAMES_NB);
	printf("hardware: %g seconds => %g frames/s\n", hard_time, FRAMES_NB/hard_time);
	printf("software: %g seconds => %g frames/s\n", soft_time, FRAMES_NB/soft_time);
	printf("speedup is %g\n", soft_time / hard_time);

	print("Finished - Entering infinite loop\n");
	while(1);

	cleanup_platform();
	return 0;
}

