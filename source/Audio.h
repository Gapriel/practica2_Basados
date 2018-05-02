/*
 * Audio.h
 *
 *  Created on: Apr 28, 2018
 *      Author: francisco
 */

#ifndef AUDIO_H_
#define AUDIO_H_
#include <DAC.h>
#include "pit.h"


#define audio_buffer_sizes 2000

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
    \brief  Configures the PIT and DAC for audio with the output period
     \param[in] Tx is the output period for the DAC
    \return void
 */

void AudioConfig(uint32_t Tx);



/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
    \brief  Changes the output of the DAC (Mute)
     \param[in] ON_OFF DAC
    \return void
 */
void AudioOutput(uint8_t ON_OFF);

#endif /* AUDIO_H_ */
