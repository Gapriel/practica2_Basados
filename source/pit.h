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

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
    \brief  Configures and start a PIT timer
     \param[in] usecs time in microseconds for the period of PIT0
    \return void
 */

void PIT_ConfigAndStart(uint8_t channel, uint32_t usecs);

#endif
