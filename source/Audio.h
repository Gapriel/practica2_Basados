/*
 * Audio.h
 *
 *  Created on: Apr 28, 2018
 *      Author: francisco
 */

#ifndef AUDIO_H_
#define AUDIO_H_
#include "dac_adc.h"
#include "pit.h"




void AudioConfig(uint32_t Tx);

void AudioOutput(uint8_t ON_OFF);
#endif /* AUDIO_H_ */
