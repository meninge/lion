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

int main()
{
	int image, neuron;
	uint32_t results_soft[FRAMES_NB][NEU2];
	uint32_t results_hard[FRAMES_NB][NEU2];
	uint32_t classification_soft[FRAMES_NB];
	uint32_t classification_hard[FRAMES_NB];
	uint32_t success_hits_hard = 0;
	uint32_t success_hits_soft = 0;

	init_platform();

	print("Hello World\n");
	print("Waiting for 3s to let user connect UART...");
	sleep(3);
	print(" done\n");

	print("Explicitely enabling UART Rx and Tx, no parity, no interrupts...\n");
	// Explicitely enable Rx ad Tx
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET,
			XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN);
	// No parity
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_MR_OFFSET,
			XUARTPS_MR_CHMODE_NORM | XUARTPS_MR_PARITY_NONE);
	// Disable interrupts
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_IDR_OFFSET,
			XUARTPS_IXR_MASK);

#ifdef DEMO
	start_demo();
#else
	//TODO
	// * compiler et tester pour MNIST et normal
	// * ajouter tableau images 0 à 9 pour démo dans dataset.h

#ifdef MNIST
	nn_hardware((uint8_t **)frames, (uint32_t **)results_hard,
			(int32_t **)w1, (int32_t **)w2,
			(int32_t *)b1, (int32_t *)b2);
	nn_software((uint8_t **)frames, (uint32_t **)results_soft,
			(int32_t **)w1, (int32_t **)w2,
			(int32_t *)b1, (int32_t *)b2);
#else
	nn_hardware((uint8_t **)data_frames, (uint32_t **)results_hard,
			(int32_t **)config_neu1, (int32_t **)config_neu2,
			(int32_t *)config_recode1, (int32_t *)config_recode2);
	nn_software((uint8_t **)data_frames, (uint32_t **)results_soft,
			(int32_t **)config_neu1, (int32_t **)config_neu2,
			(int32_t *)config_recode1, (int32_t *)config_recode2);
#endif /* MNIST */

#endif

	/*
	 * Compare hard and soft results.
	 */
	for (image = 0; image < FRAMES_NB; image++) {
		for (neuron = 0; neuron < NEU2; neuron++) {
			if (results_soft[image][neuron] !=
					results_hard[image][neuron]) {
				printf("WARNING: difference soft/hard\n");
				printf("soft: image n°%3d, neurone n°%3d : %10lu\n",
						image, neuron,
						(unsigned long)
						results_soft[image][neuron]);
				printf("hard: image n°%3d, neurone n°%3d : %10lu\n",
						image, neuron,
						(unsigned long)
						results_hard[image][neuron]);
			}
		}
	}

	/*
	 * Classify the results.
	 */
	classify((uint32_t **)results_hard, (uint32_t *)classification_hard);
	classify((uint32_t **)results_soft, (uint32_t *)classification_soft);

	/*
	 * Compute hardware and software succes rates.
	 */
	for (image = 0; image < FRAMES_NB; image++) {
		if (classification_hard[image] == labels[image]) {
			success_hits_hard++;
		}
		if (classification_soft[image] == labels[image]) {
			success_hits_soft++;
		}
		printf("hard, taux de réussite : %.2f%%\n",
				(success_hits_hard / (float)FRAMES_NB) * 100);
		printf("soft, taux de réussite : %.2f%%\n",
				(success_hits_soft / (float)FRAMES_NB) * 100);
	}

	/*
	 * Compare hard and soft timing performances.
	 */
	printf("%u frames\n:", FRAMES_NB);
	printf("hardware: %3g seconds => %3g frames/s\n",
			hard_time, FRAMES_NB/hard_time);
	printf("software: %3g seconds => %3g frames/s\n",
			soft_time, FRAMES_NB/soft_time);
	printf("speedup is %g\n", soft_time / hard_time);

	print("Finished - Entering infinite loop\n");
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
