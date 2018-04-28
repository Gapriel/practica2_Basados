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
#include "tcpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

static void
tcpecho_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);		     /**creates an empty socket*/
  netconn_bind(conn, IP_ADDR_ANY, 50008);	/**assign a local IP address and port number to the previously created socket*/
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  static uint8_t first_menu_print = 0;
  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);	//socket connection accepted
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {	//if there was a data request by a client,
      struct netbuf *buf;
      void *data;
      u16_t len;

      if(0 == first_menu_print){
    	  first_menu_print = 1;
    	  uint8_t menus_length = 91;
    	  uint8_t * menus = "WIRELESS K64 SPEAKERS\n\n1)Stop/play audio\n2)Select music source device\n3)Deploy comms stats\n";
    	  menus = (void*)menus;
    	  err = netconn_write(newconn, menus, menus_length, NETCONN_COPY);
      }


      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {	//while the client asks for data,
        /*printf("Recved\n");*/
    	  uint8_t * spacing = "\n\n\n\n\n\n\n";
    	  spacing = (void*) spacing;
   		  uint8_t spacing_length = 7;
    	  do {
        	netbuf_data(buf, &data, &len);
        	uint8_t * data_casted = (uint8_t*)data;
        	if(*data_casted == 'x'){
        		uint8_t * spacing5 = "\n\n\n\n\n";
        		spacing5 = (void*) spacing5;
        		uint8_t spacing5_length = 5;
        		uint8_t menus_length = 91;
        	    uint8_t * menus = "WIRELESS K64 SPEAKERS\n\n1)Stop/play audio\n2)Select music source device\n3)Deploy comms stats\n";
        		menus = (void*)menus;
        		err = netconn_write(newconn, menus, menus_length, NETCONN_COPY);
        		err = netconn_write(newconn, spacing5, spacing5_length, NETCONN_COPY);
        	}
        	if(*data_casted == '2'){
        		uint8_t * spacing9 = "\n\n\n\n\n\n\n\n\n";
        		spacing9 = (void*) spacing9;
        		uint8_t spacing9_length = 9;
        		uint8_t * menu2 = "Type in the port\nyou want to listen to:";
        		uint8_t menu2_length = 39;
        		menu2 = (void*)menu2;
        		err = netconn_write(newconn, menu2,menu2_length, NETCONN_COPY);
        		err = netconn_write(newconn, spacing9, spacing9_length, NETCONN_COPY);
        	}

        	if(*data_casted == '3'){	//if the comm quality was selected,
        		uint8_t * menu3_1 = "comms stats\nUDP packs received per second: ";
        		uint8_t menu3_1_length = 43;
        		menu3_1 = (void*)menu3_1;
        		err = netconn_write(newconn, menu3_1,menu3_1_length, NETCONN_COPY);
        		uint8_t value3_1 = " 3";
        		uint8_t value3_1_length = 3;
        		value3_1 = (void*)value3_1;
        		err = netconn_write(newconn, value3_1,value3_1_length, NETCONN_COPY);
        		uint8_t * menu3_2 = "\nUDP packs received per second: ";
        	    uint8_t menu3_2_length = 32;
        	    menu3_2 = (void*)menu3_2;
        		err = netconn_write(newconn, menu3_2,menu3_2_length, NETCONN_COPY);
        		uint8_t value3_2 = " 8";
        		uint8_t value3_2_length = 3;
        		value3_2 = (void*)value3_2;
        		err = netconn_write(newconn, value3_2,value3_2_length, NETCONN_COPY);
        		uint8_t * menu3_3 = "\nUDP comm quality: ";
        		uint8_t menu3_3_length = 19;
        		menu3_3 = (void*)menu3_3;
        		err = netconn_write(newconn, menu3_3,menu3_3_length, NETCONN_COPY);
        		uint8_t value3_3 = " 2";
        		uint8_t value3_3_length = 3;
        		value3_3 = (void*)value3_3;
        		err = netconn_write(newconn, value3_3,value3_3_length, NETCONN_COPY);

        		err = netconn_write(newconn, spacing, spacing_length, NETCONN_COPY);
        	}
             //netbuf_data(buf, &data, &len);
             //err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}

/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
  sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
