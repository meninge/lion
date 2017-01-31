/*
 * demo.h
 *
 *  Created on: Jan 31, 2017
 *      Author: xph3sle502
 */

#ifndef DEMO_H_
#define DEMO_H_


#include <stdint.h>
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

void start_demo();
void nn_process_frames_for_demo(int32_t* digit);

#endif /* DEMO_H_ */
