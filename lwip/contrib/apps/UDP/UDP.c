/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <UDP/UDP.h>
#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/api.h"
#include "lwip/sys.h"
#include "Audio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"

#include "fsl_edma.h"
#include "fsl_dmamux.h"


uint16_t buffer1[audio_buffer_sizes] ;
uint16_t buffer2[audio_buffer_sizes] ;
uint16_t dummy_buffer[audio_buffer_sizes];
uint16_t* DAC_buffer =buffer2;
uint16_t* Alternative_buffer =buffer1;
uint8_t bussy_buffer = 2;
uint8_t buffer_to_use = 0;



extern uint8_t i;
extern QueueHandle_t port_selection;
extern EventGroupHandle_t WirlessSpeakers_events;
extern QueueHandle_t UDP_status_values;
/********************* DEFINITIONS **************/
#define BUFF_BUSSY (1 << 0)
#define BUFF_EMPTY (1 << 1)
#define BUFF_ONE_ACTIVE (1 << 2)



/***************Variables ***********************/

SemaphoreHandle_t buffer1_semaphore;
SemaphoreHandle_t buffer2_semaphore;


uint32_t Fs = 96000;

void edma_complete_transfer(uint16_t* source,uint16_t* destination);
void init_edma_audio();

edma_handle_t g_EDMA_Handle;
volatile bool g_Transfer_Done = false;







extern struct netconn * newconn;

bool buffer_flag = pdFALSE;
uint16_t packs_count = 0;
uint8_t packs_in_theory_to_be_received = 110;

UDP_val_t * UDP_values_to_transport;
uint8_t pit1flag = 0;
void PIT1_IRQHandler(void){
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
    UDP_values_to_transport->received = packs_count;
    UDP_values_to_transport->losses = (uint16_t)(packs_in_theory_to_be_received - packs_count);
    float relation = ((float)UDP_values_to_transport->received / (float)packs_in_theory_to_be_received);
    UDP_values_to_transport->relation = (uint16_t)((100)*relation);
    //tcp_printUDPstatus(tcp_printUDPstatus);
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(UDP_status_values,&UDP_values_to_transport,&pxHigherPriorityTaskWoken);
    packs_count = 0;
    pit1flag = 1;
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

}






/*-----------------------------------------------------------------------------------*/
static void UDP_thread(void *arg) {
    LWIP_UNUSED_ARG(arg);

    struct netconn *conn;
    struct netbuf *buf;
    uint16_t len;
    uint16_t *msg;
    err_t err;
    uint16_t* received_port = 0;
    uint16_t port = 50012;


    DAC_buffer = buffer1;
    buffer1_semaphore = xSemaphoreCreateBinary();
    buffer2_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(buffer1_semaphore);
    xSemaphoreGive(buffer2_semaphore);

init_edma_audio();
    conn = netconn_new(NETCONN_UDP);
    g_Transfer_Done = true;
    while (1)
    {
        if ((*received_port) != 0)
        {
            port = *received_port;
            vPortFree(received_port);
        }
        netconn_bind(conn, IP_ADDR_ANY, port);

        UDP_values_to_transport = pvPortMalloc(sizeof(UDP_val_t));
        do
        {

            err = netconn_recv(conn, &buf);
            if (err == ERR_OK)
            {
                packs_count++;
                /***************************************************************************/
                    if(2 == bussy_buffer && (g_Transfer_Done == true) ){

                     netbuf_data(buf, (void**)&msg, &len);
                     edma_complete_transfer(msg, buffer1);
                     Alternative_buffer = buffer2;
                     buffer_to_use = 1;
                     AudioConfig(Fs);
                     netbuf_delete(buf);


                    }else if (1 == bussy_buffer && (g_Transfer_Done == true)){

                     /*
                      * Copies to buffer 2
                      */

                     netbuf_data(buf, (void**) &msg, &len);
                     edma_complete_transfer(msg, buffer2);
                     Alternative_buffer = buffer1;
                     buffer_to_use = 2;
                     AudioConfig(Fs);
                     netbuf_delete(buf);

                    }else{
                    }

                /******************************************/

            }
        } while (pdPASS != xQueueReceive(port_selection, &received_port, 0));
        netconn_close(conn);
    }
}

/*-----------------------------------------------------------------------------------*/
void UDP_init(void) {
    sys_thread_new("UDP thread", UDP_thread, NULL,
    DEFAULT_THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO);
}

#endif /* LWIP_NETCONN */


#define EXAMPLE_DMA DMA0
#define EXAMPLE_DMAMUX DMAMUX0

#define BUFF_LENGTH audio_buffer_sizes

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

edma_transfer_config_t transferConfig;
edma_config_t userConfig;
/* User callback function for EDMA transfer. */
void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_Transfer_Done = true;
    }
}

void init_edma_audio(){

    /* Configure DMAMUX */
    DMAMUX_Init(EXAMPLE_DMAMUX);
#if defined(FSL_FEATURE_DMAMUX_HAS_A_ON) && FSL_FEATURE_DMAMUX_HAS_A_ON
    DMAMUX_EnableAlwaysOn(EXAMPLE_DMAMUX, 0, true);
#else
    DMAMUX_SetSource(EXAMPLE_DMAMUX, 0, 63);
#endif /* FSL_FEATURE_DMAMUX_HAS_A_ON */
    DMAMUX_EnableChannel(EXAMPLE_DMAMUX, 0);
    /* Configure EDMA one shot transfer */
    /*
     * userConfig.enableRoundRobinArbitration = false;
     * userConfig.enableHaltOnError = true;
     * userConfig.enableContinuousLinkMode = false;
     * userConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA, &userConfig);
    EDMA_CreateHandle(&g_EDMA_Handle, EXAMPLE_DMA, 0);
    EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
}

void edma_complete_transfer(uint16_t* source,uint16_t* destination){
    g_Transfer_Done =false;
    EDMA_PrepareTransfer(&transferConfig, source, 2U, destination,2U,
                        2U, 2*audio_buffer_sizes, kEDMA_MemoryToMemory);
    EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);
    EDMA_StartTransfer(&g_EDMA_Handle);
    /* Wait for EDMA transfer finish */


}
