/*
 * Audio.c
 *
 *  Created on: Apr 28, 2018
 *      Author: francisco
 */
#include "Audio.h"
#include "FreeRTOS.h"

extern uint16_t buffer_cagazon[750] ;
extern uint16_t buffer2_cagazon[750] ;

uint16_t i = 0;
extern bool buffer_flag ;
void PIT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    if(pdFALSE == buffer_flag){
        DAC_SetBufferValue(DAC0, 0, buffer_cagazon[i]);
    }else{
        DAC_SetBufferValue(DAC0, 0, buffer2_cagazon[i]);
    }
    i = (i < 748) ? i+1 : 0;
}



void AudioConfig(uint32_t Tx){
    DAC_Config();
    PIT_ConfigAndStart(Tx);
}

