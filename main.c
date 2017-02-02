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
	int difference_soft_hard = 0;
	uint32_t results_soft[FRAMES_NB][NEU2];
	uint32_t results_hard[FRAMES_NB][NEU2];
	uint32_t classification_soft[FRAMES_NB];
	uint32_t classification_hard[FRAMES_NB];
	uint32_t success_hits_hard = 0;
	uint32_t success_hits_soft = 0;

	init_platform();

	print("     --------------------------\n");
	print("     | MENINGE NEURAL NETWORK |\n");
	print("     | Paul Luperini          |\n");
	print("     | Lucas Mahieu           |\n");
	print("     | Hugues de Valon        |\n");
	print("     --------------------------\n\n");
	sleep(3);

	// Explicitely enable Rx ad Tx
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET,
			XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN);
	// No parity
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_MR_OFFSET,
			XUARTPS_MR_CHMODE_NORM | XUARTPS_MR_PARITY_NONE);
	// Disable interrupts
	XUartPs_WriteReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_IDR_OFFSET,
			XUARTPS_IXR_MASK);

#ifdef MNIST
	/*
	 * On passe à la fonction un tableau à deux dimensions en lui donnant
	 * l'adresse de la première case du tableau.
	 */
	printf("hardware computation... ");
	fflush(stdout);
	nn_hardware(&frames[0][0], &results_hard[0][0],	&w1[0][0], &w2[0][0],
			b1, b2);
	printf("done.\n");
	printf("software computation... ");
	fflush(stdout);
	nn_software(&frames[0][0], &results_soft[0][0],	&w1[0][0], &w2[0][0],
			b1, b2);
	printf("done.\n");
#else
	printf("hardware computation... ");
	fflush(stdout);
	nn_hardware(&data_frames[0][0], &results_hard[0][0],
			&config_neu1[0][0], &config_neu2[0][0],
			config_recode1, config_recode2);
	printf("done.\n");
	printf("software computation... ");
	fflush(stdout);
	nn_software(&data_frames[0][0], &results_soft[0][0],
			&config_neu1[0][0], &config_neu2[0][0],
			config_recode1, config_recode2);
	printf("done.\n");
#endif /* MNIST */

	/*
	 * Compare hard and soft results.
	 */
	for (image = 0; image < FRAMES_NB; image++) {
		for (neuron = 0; neuron < NEU2; neuron++) {
			if (results_soft[image][neuron] !=
					results_hard[image][neuron]) {
				difference_soft_hard = 1;
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
	if (!difference_soft_hard) {
		printf("no difference between software and hardware results.\n");
	}

	/*
	 * Classify the results.
	 */
	classify(&results_hard[0][0], classification_hard);
	classify(&results_soft[0][0], classification_soft);

#ifdef MNIST
	/*
	 * Compute hardware and software succes rates.
	 */
	for (image = 0; image < FRAMES_NB; image++) {
		display(classification_hard[image]);
		if (classification_hard[image] == labels[image]) {
			success_hits_hard++;
		}
		if (classification_soft[image] == labels[image]) {
			success_hits_soft++;
		}

	}
	printf("hardware success rate: %.2f %%\n",
			(success_hits_hard / (float)FRAMES_NB) * 100);
	printf("software success rate: %.2f %%\n",
			(success_hits_soft / (float)FRAMES_NB) * 100);
#endif

	/*
	 * Compare hard and soft timing performances.
	 */
	printf("%u frames\n", FRAMES_NB);
	printf("hardware: %4.2g seconds => %5d frames/s\n",
			hard_time, (int)(FRAMES_NB/hard_time));
	printf("software: %4.2g seconds => %5d frames/s\n",
			soft_time, (int)(FRAMES_NB/soft_time));
	printf("speedup is %d\n", (int)(soft_time / hard_time));

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
