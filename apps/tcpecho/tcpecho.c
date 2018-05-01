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
#include "FreeRTOS.h"
#include "event_groups.h"

#include "tcpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

#include "Audio.h"
#define DAC_STATE (1<<0)
#define ON_UDP_MENU (1<<1)

extern QueueHandle_t port_selection;
extern EventGroupHandle_t WirelessSpeakers_events;
extern QueueHandle_t UDP_status_values;

typedef struct{
	uint16_t losses;
	uint16_t received;
	uint16_t relation;
}UDP_val_t;

static err_t tcp_printMenu(struct netconn * connection){
	xEventGroupClearBits(WirelessSpeakers_events, ON_UDP_MENU);	//clears the 'on menu' event in order to tell the queue sender function that it shouldn't send somenthing
	uint8_t * spacing5 = "\n\n\n\n\n";
	spacing5 = (void*) spacing5;
	uint8_t spacing5_length = 5;
	uint8_t menus_length = 93;
	uint8_t * menus = "WIRELESS K64 SPEAKERS\n\n1)Mute/Unmute audio\n2)Select music source device\n3)Deploy comms stats\n";
	menus = (void*) menus;
	err_t err;
    err = netconn_write(connection, menus, menus_length, NETCONN_COPY);
	err = netconn_write(connection, spacing5, spacing5_length, NETCONN_COPY);
	return err;
}
static err_t tcp_printSelector(struct netconn * connection){
	uint8_t * spacing9 = "\n\n\n\n\n\n\n\n\n";
	spacing9 = (void*) spacing9;
	uint8_t spacing9_length = 9;
	uint8_t * menu2 = "Type in the port\nyou want to listen to:";
	uint8_t menu2_length = 39;
	menu2 = (void*) menu2;
	err_t err;
	err = netconn_write(connection, menu2, menu2_length, NETCONN_COPY);
	err = netconn_write(connection, spacing9, spacing9_length, NETCONN_COPY);
	return err;
}
static err_t tcp_printUDPstatus(struct netconn * connection){
	xEventGroupSetBits(WirelessSpeakers_events, ON_UDP_MENU);	//sets the 'on menu' event in order to tell the queue sender function that it should send somenthing
	UDP_val_t  receiver_values ;//= pvPortMalloc(sizeof(UDP_val_t)); //TODO: quitar el malloc, es para pruebas
	receiver_values.losses = 12345;
	receiver_values.received = 23456;
	receiver_values.relation = 34567;
	//xQueueReceive(UDP_status_values,&receiver_values,portMAX_DELAY);
	err_t err;
	uint8_t * spacing = "\n\n\n\n\n\n\n";
	spacing = (void*) spacing;
	uint8_t spacing_length = 7;
	uint8_t * menu3_1 = "comms stats\nUDP packs received per second: ";
	uint8_t menu3_1_length = 43;
	menu3_1 = (void*) menu3_1;
	err = netconn_write(connection, menu3_1, menu3_1_length, NETCONN_COPY);
	uint8_t value3_1[5];
	value3_1[0] = (uint8_t)((receiver_values.received/10000)+'0');
	value3_1[1] = (uint8_t)( ( (receiver_values.received - ((value3_1[0]-'0')*10000) ) /1000)+'0' );
	value3_1[2] = ( (receiver_values.received - ((value3_1[0]-'0')*10000)-((value3_1[1]-'0')*1000)) /100 )+'0';
	value3_1[3] = ( (receiver_values.received - ((value3_1[0]-'0')*10000)-((value3_1[1]-'0')*1000)- ((value3_1[2]-'0')*100) ) /10 )+'0';
	value3_1[4] = ( (receiver_values.received - ((value3_1[0]-'0')*10000)-((value3_1[1]-'0')*1000)- ((value3_1[2]-'0')*100) - ((value3_1[3]-'0')*10) ) )+'0';
	//uint8_t value3_1_length = 5;
	uint8_t value3_1_length = 5;
	uint8_t *value3_1_casted = (void*) value3_1;
	err = netconn_write(connection, value3_1_casted, value3_1_length, NETCONN_COPY);
	uint8_t * menu3_2 = "\nUDP packs lost per second: ";
	uint8_t menu3_2_length = 28;
	menu3_2 = (void*) menu3_2;
	err = netconn_write(connection, menu3_2, menu3_2_length, NETCONN_COPY);
	uint8_t value3_2[5];
	value3_2[0] = (receiver_values.losses/10000)+'0';
	value3_2[1] = ( (receiver_values.losses - ((value3_2[0]-'0')*10000) ) /1000)+'0';
	value3_2[2] = ( (receiver_values.losses - ((value3_2[0]-'0')*10000)-((value3_2[1]-'0')*1000)) /100 )+'0';
	value3_2[3] = ( (receiver_values.losses - ((value3_2[0]-'0')*10000)-((value3_2[1]-'0')*1000)- ((value3_2[2]-'0')*100) ) /10 )+'0';
	value3_2[4] = ( (receiver_values.losses - ((value3_2[0]-'0')*10000)-((value3_2[1]-'0')*1000)- ((value3_2[2]-'0')*100) - ((value3_2[3]-'0')*10) ) )+'0';
	uint8_t value3_2_length = 5;
	void *value3_2_casted = (void*) value3_2;
	err = netconn_write(connection, value3_2_casted, value3_2_length, NETCONN_COPY);
	uint8_t * menu3_3 = "\nUDP comm quality: ";
	uint8_t menu3_3_length = 19;
	menu3_3 = (void*) menu3_3;
	err = netconn_write(connection, menu3_3, menu3_3_length, NETCONN_COPY);
	uint8_t value3_3[5];
	value3_3[0] = (receiver_values.relation/10000)+'0';
	value3_3[1] = ( (receiver_values.relation - ((value3_3[0]-'0')*10000) ) /1000)+'0';
	value3_3[2] = ( (receiver_values.relation - ((value3_3[0]-'0')*10000)-((value3_3[1]-'0')*1000)) /100 )+'0';
	value3_3[3] = ( (receiver_values.relation - ((value3_3[0]-'0')*10000)-((value3_3[1]-'0')*1000)- ((value3_3[2]-'0')*100) ) /10 )+'0';
	value3_3[4] = ( (receiver_values.relation - ((value3_3[0]-'0')*10000)-((value3_3[1]-'0')*1000)- ((value3_3[2]-'0')*100) - ((value3_3[3]-'0')*10) ) )+'0';
	uint8_t value3_3_length = 5;
	void *value3_3_casted = (void*) value3_3;
	err = netconn_write(connection, value3_3_casted, value3_3_length, NETCONN_COPY);
	err = netconn_write(connection, spacing, spacing_length, NETCONN_COPY);
	//vPortFree(receiver_values);
	return err;
}

typedef enum{
	main_menu,
	port_selector,
	comm_status
}menus_t;

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
    	  err = tcp_printMenu(newconn);
      }

      static uint8_t current_menu = main_menu;
      static uint8_t MusicState = 1;
      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {	//while the client asks for data,
        /*printf("Recved\n");*/

    	  do {
        	netbuf_data(buf, &data, &len);

        	uint8_t * data_casted = (uint8_t*)data;

        	switch(*data_casted){
        	case 'x':
        		current_menu = main_menu;
        		err = tcp_printMenu(newconn);
        		break;
        	case '1':
        		if(1 == MusicState){ //if the state was previously on, turns it off
        		    AudioOutput(1);
        			MusicState = 0;
        		}else if(0 == MusicState){	//if the state was previously off, turns it on
        			AudioOutput(0);
        			MusicState = 1;
        		}
        		break;
        	case '2':
        		if(main_menu == current_menu || port_selector == current_menu){
        			current_menu = port_selector;
        			err = tcp_printSelector(newconn);
        			netbuf_delete(buf);
        			err = netconn_recv(newconn, &buf);
        			uint8_t read_port[10];
        			netbuf_copy(buf,read_port,10);
        			if('x' != read_port[0]){
        				uint16_t new_port = ((read_port[0]-'0')*10000) +
        						((read_port[1]-'0')*1000) +
								((read_port[2]-'0')*100) +
								((read_port[3]-'0')*10) +
								(read_port[4]-'0');
        				uint16_t * port_transporter;
        				port_transporter = pvPortMalloc(sizeof(uint16_t));
        				*port_transporter = new_port;
        				xQueueSend(port_selection,&port_transporter,100);	//sends the port selection to the UDP task
        			}
      				current_menu = main_menu;
      			    err = tcp_printMenu(newconn);
        		}
        		break;
        	case '3':
        		if(main_menu == current_menu){
        			current_menu = comm_status;
        			err = tcp_printUDPstatus(newconn);
        		}
        		break;
        	}

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
