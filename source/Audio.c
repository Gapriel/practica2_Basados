/*
 * Audio.c
 *
 *  Created on: Apr 28, 2018
 *      Author: francisco
 */
#include "Audio.h"
#include "FreeRTOS.h"



extern uint16_t buffer1[audio_buffer_sizes] ;
extern uint16_t buffer2[audio_buffer_sizes] ;

uint16_t i = 0;
extern bool buffer_flag ;


void PIT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    if(pdFALSE == buffer_flag){
        DAC_SetBufferValue(DAC0, 0, buffer1[i]);
    }else{

        DAC_SetBufferValue(DAC0, 0, buffer2[i]);
    }
    i = (i < audio_buffer_sizes -1) ? i+1 : 0;
}



void AudioConfig(uint32_t Tx){
    DAC_Config();
    PIT_ConfigAndStart(Tx);
}


void AudioOutput(uint8_t ON_OFF){
    if(1 == ON_OFF){
        PIT_StartTimer(PIT, kPIT_Chnl_0);
        //PIT_ON
    }else{
        //PIT OFF
        PIT_StopTimer(PIT, kPIT_Chnl_0);
        DAC_SetBufferValue(DAC0, 0, 0);
    }
}
