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
 * CoAuthor: Gabriel Santamaría
 * CoAuthor: Francisco Avelar
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

/*
 * Buffers for the incoming data
 */
uint16_t buffer1[audio_buffer_sizes] = { 2045 };
uint16_t buffer2[audio_buffer_sizes] = { 2045 };
extern uint8_t i;


extern QueueHandle_t port_selection;
extern EventGroupHandle_t WirlessSpeakers_events;
extern QueueHandle_t UDP_status_values;

bool buffer_flag = pdFALSE;


/*-----------------------------------------------------------------------------------*/
static void UDP_thread(void *arg) {
    LWIP_UNUSED_ARG(arg);
/*
 * socket for the new connection
 */
    struct netconn *conn;
      struct netbuf *buf;
      err_t err;
      /*
       * default port
       */
      uint16_t* received_port = 0 ;
      uint16_t port = 50007;

      conn = netconn_new(NETCONN_UDP);
    while (1)
    {
        /*
         * if the received port is valid
         */
        if ( (*received_port) != 0){
            port = *received_port;
            vPortFree(received_port);
        }

#if LWIP_IPV6
            conn = netconn_new(NETCONN_UDP_IPV6);
            netconn_bind(conn, IP6_ADDR_ANY, port);
#else /* LWIP_IPV6 */
            /*
             * it can bind with any address but just for the specified port
             */
            netconn_bind(conn, IP_ADDR_ANY, port);
#endif /* LWIP_IPV6 */
            LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);

#if LWIP_IGMP /* Only for testing of multicast join*/
            {
#include "lwip\netif.h"

                ip4_addr_t multiaddr;
                IP4_ADDR(&multiaddr, 224, 5, 6, 7);

                err = netconn_join_leave_group(conn, &multiaddr, &netif_default->ip_addr, NETCONN_JOIN);
                LWIP_ERROR("udpecho: join group is failed", (err == ERR_OK), return;);
            }
#endif
            /*
             * configs the DAC and pit for the audio with a Ts of 45 microseconds
             */
            AudioConfig(45);
            do
            {

                err = netconn_recv(conn, &buf);
                if (err == ERR_OK)
                {
                    /*  no need netconn_connect here, since the netbuf contains the address */
                    if (pdFALSE == buffer_flag)
                    {
                        if (netbuf_copy(buf, buffer1,
                                        sizeof(buffer1))
                                != buf->p->tot_len)
                        {
                            LWIP_DEBUGF(LWIP_DBG_ON, ("netbuf_copy failed\n"));
                            /*
                             * resets the index of the buffer, so no errors with the buffer are done
                             */
                            i = 0;
                        } else
                        {
                            err = netconn_send(conn, buf);
                            if (err != ERR_OK)
                            {
                            /*
                             * if there was a mistake on the received info
                             */
                                LWIP_DEBUGF(
                                        LWIP_DBG_ON,
                                        ("netconn_send failed: %d\n", (int)err));
                            }
                        }
                        /*
                         * changes the buffer flag so the next received data is storaged on the other buffer
                         */
                        buffer_flag = pdTRUE;
                        i = 0;
                    } else
                    {
                        /*
                         * the same has the fisrt buffer, but with the second one
                         */
                        if (netbuf_copy(buf, buffer2, sizeof(buffer2)) != buf->p->tot_len)
                        {
                            LWIP_DEBUGF(LWIP_DBG_ON, ("netbuf_copy failed\n"));
                            i = 0;
                        } else
                        {
                            err = netconn_send(conn, buf);
                            if (err != ERR_OK)
                            {
                                LWIP_DEBUGF(
                                        LWIP_DBG_ON,
                                        ("netconn_send failed: %d\n", (int)err));
                            }
                        }

                        buffer_flag = pdFALSE;
                        i = 0;
                    }
                    netbuf_delete(buf);
                }
            }while(pdPASS != xQueueReceive(port_selection,&received_port,0));
            /*
             * if something was received by the queue, then it breaks the loop so it can cheack if it has to bind with
             * another port
             */
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
