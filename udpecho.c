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

#include "udpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/api.h"
#include "lwip/sys.h"
#include "Audio.h"
#define MAX_FS 44100
#define DOWNSAMPLE 4
uint16_t buffer_cagazon[2045] = {750};
uint16_t buffer2_cagazon[2045] ={750};

bool buffer_flag = pdFALSE;
#define sec_prueba ((1/(MAX_FS/DOWNSAMPLE))* 1000000)

uint32_t usec = (uint32_t)(sec_prueba);

/*-----------------------------------------------------------------------------------*/
static void udpecho_thread(void *arg)
{
  struct netconn *conn;
  struct netbuf *buf;
  err_t err;
  LWIP_UNUSED_ARG(arg);

#if LWIP_IPV6
  conn = netconn_new(NETCONN_UDP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_UDP);
  netconn_bind(conn, IP_ADDR_ANY, 50007);
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
  AudioConfig(45);
  while (1) {
    err = netconn_recv(conn, &buf);
    if (err == ERR_OK) {
      /*  no need netconn_connect here, since the netbuf contains the address */
      if (pdFALSE == buffer_flag)
      {
          buffer_flag = pdTRUE;
          if(netbuf_copy(buf, buffer_cagazon, sizeof(buffer_cagazon)) != buf->p->tot_len)
          {
           LWIP_DEBUGF(LWIP_DBG_ON, ("netbuf_copy failed\n"));
          }
          else
          {
              err = netconn_send(conn, buf);
              if(err != ERR_OK)
              {
                 LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
              }
              else
              {

              //Llamada a función de transferencia de buffer a la posición de lectura
             //del DAC
              }
            }
      }
      else
      {
         buffer_flag = pdFALSE;
         if(netbuf_copy(buf, buffer2_cagazon, sizeof(buffer2_cagazon)) != buf->p->tot_len)
         {
          LWIP_DEBUGF(LWIP_DBG_ON, ("netbuf_copy failed\n"));
         }
         else
         {
             err = netconn_send(conn, buf);
         if(err != ERR_OK)
         {
            LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
         }
         else
         {
            //Llamada a función de transferencia de buffer a la posición de lectura
            //del DAC
         }
         }
      }
      netbuf_delete(buf);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
udpecho_init(void)
{
  sys_thread_new("udpecho_thread", udpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}

#endif /* LWIP_NETCONN */
