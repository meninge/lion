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

/*
 * main.c
 * demo version: use 10 MNIST frames and print the guessed digits.
 */
int main()
{
	int image;
	uint32_t results_hard[FRAMES_NB][NEU2];
	uint32_t classification_hard[FRAMES_NB];
	uint32_t success_hits_hard = 0;

	init_platform();

	print("     --------------------------\n");
	print("     | MENINGE NEURAL NETWORK |\n");
	print("     | DEMO VERSION           |\n");
	print("     | Paul Luperini          |\n");
	print("     | Lucas Mahieu           |\n");
	print("     | Hugues de Valon        |\n");
	print("     --------------------------\n\n");
	sleep(8);

	// Explicitely enable Rx ad Tx
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET,
			XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN);
	// No parity
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_MR_OFFSET,
			XUARTPS_MR_CHMODE_NORM | XUARTPS_MR_PARITY_NONE);
	// Disable interrupts
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_IDR_OFFSET,
			XUARTPS_IXR_MASK);

	nn_hardware(&frames[0][0], &results_hard[0][0],	&w1[0][0], &w2[0][0],
			b1, b2);

	/*
	 * Classify the results.
	 */
	classify(&results_hard[0][0], classification_hard);

	/*
	 * Compute hardware succes rates.
	 */
	for (image = 0; image < FRAMES_NB; image++) {
		display(classification_hard[image]);
		if (classification_hard[image] == labels[image]) {
			success_hits_hard++;
		}
		sleep(2);
	}

	printf("hardware success rate: %.2f %%\n",
			(success_hits_hard / (float)FRAMES_NB) * 100);

	while(1);

	cleanup_platform();
	return 0;
}

double XTime_ToDouble(XTime *t)
{
	return ((double)(*t)) / COUNTS_PER_SECOND;
}

double XTime_GetDiff_Double(XTime *oldtime, XTime *newtime)
{
	return ((double)(*newtime - *oldtime)) / COUNTS_PER_SECOND;
}

double XTime_DiffCurrReal_Double(XTime *oldtime)
{
	XTime newtime;
	XTime_GetTime(&newtime);
	return XTime_GetDiff_Double(oldtime, &newtime);
}

void* malloc_with_loc(unsigned size, char* file, unsigned line)
{
	void* ptr = malloc(size);
	if (ptr == NULL) {
		abort_printf(
				"From %s:%u : malloc() returned NULL for size %u\n",
				file, line, size);
	}
	return ptr;
}
