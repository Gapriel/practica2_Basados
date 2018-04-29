/*
 * pit.h
 *
 *  Created on: Apr 28, 2018
 *      Author: francisco
 */

#ifndef PIT_H_
#define PIT_H_

#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_pit.h"
#include "pit.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "FreeRTOS.h"


void PIT_ConfigAndStart(uint32_t usecs);

#endif
