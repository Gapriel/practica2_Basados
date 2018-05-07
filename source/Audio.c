/*
 * Audio.c
 *
 *  Created on: Apr 28, 2018
 *      Author: francisco
 */
#include "Audio.h"
#include "FreeRTOS.h"
#include "semphr.h"



extern uint16_t buffer1[audio_buffer_sizes] ;
extern uint16_t buffer2[audio_buffer_sizes] ;

uint16_t i = 0;
extern bool buffer_flag ;
bool i_flag = pdFALSE;


extern uint16_t* DAC_buffer;
extern uint16_t* Alternative_buffer;
extern uint8_t bussy_buffer;


extern uint8_t buffer_to_use ;

extern SemaphoreHandle_t buffer1_semaphore;
extern SemaphoreHandle_t buffer2_semaphore;

void PIT0_IRQHandler(void)
{

    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    DAC_SetBufferValue(DAC0, 0, DAC_buffer[i]);

    i = (i < (audio_buffer_sizes -1) ) ? i+1 : 0;
    if(i == 0){
        if(buffer_to_use == 1){
            DAC_buffer = buffer1;
            bussy_buffer = 1;
        }else {
            DAC_buffer = buffer2;
            bussy_buffer = 2;
        }

    }

}


uint8_t started = 0;
void AudioConfig(uint32_t Tx){

    if(0 == started){
        DAC_Config();
        PIT_ConfigAndStart(0,Tx);
        PIT_ConfigAndStart(1, 1000);
        started = 1;
    }
}


void AudioOutput(uint8_t ON_OFF){
    if(0 == ON_OFF){
        PIT_StartTimer(PIT, kPIT_Chnl_0);
        //PIT_ON
    }else{
        //PIT OFF
        PIT_StopTimer(PIT, kPIT_Chnl_0);
        DAC_SetBufferValue(DAC0, 0, 0);
    }
}
