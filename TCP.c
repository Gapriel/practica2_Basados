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
#include <TCP/TCP.h>
#include <UDP/UDP.h>
#include "FreeRTOS.h"
#include "event_groups.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

#include "Audio.h"

#define ON_UDP_MENU (1<<1)  //event group bit used to know if UDP status must be deployed

extern QueueHandle_t port_selection;  //queue used for storing the new port to listen through UDP
extern EventGroupHandle_t WirelessSpeakers_events;  //event group used with the ON_UDP_MENU to know
													//if the user is asking for the UDP status menu
extern QueueHandle_t UDP_status_values;  //queue used for storing the UDP status values
extern pit1flag;

static err_t tcp_printMenu(struct netconn * connection){  //this function sends by tcp the main menu
	xEventGroupClearBits(WirelessSpeakers_events, ON_UDP_MENU);	//clears the 'on menu' event in order to tell the queue sender function that it shouldn't send somenthing
	uint8_t * spacing5 = "\n\n\n\n\n";  //string with enters in order to simulate a screen cleaning
	spacing5 = (void*) spacing5; //casts the spacing5 string from pointer to uint8_t to void
	uint8_t spacing5_length = 5; //defines a variable which holds the spacing5 character quantity
	uint8_t menus_length = 93;  //defines a variable which holds the menus character quantity
	uint8_t * menus = "WIRELESS K64 SPEAKERS\n\n1)Mute/Unmute audio\n2)Select music source device\n3)Deploy comms stats\n";  //the menu options to be printed for the user
	menus = (void*) menus;  //casts the menus string from pointer to uint8_t to void
	err_t err;  //declares an error variable
    err = netconn_write(connection, menus, menus_length, NETCONN_COPY);  //sends via tcp the menus
	err = netconn_write(connection, spacing5, spacing5_length, NETCONN_COPY);  //sends via tcp the spacing5 string to clear the string
	return err;  	//returns the error variable to know if there was a mistake
}
static err_t tcp_printSelector(struct netconn * connection){  	//returns the error variable to know if there was a mistake
	uint8_t * spacing9 = "\n\n\n\n\n\n\n\n\n";  //string with enters in order to simulate a screen cleaning
	spacing9 = (void*) spacing9;  //casts the spacing9 string from pointer to uint8_t to void
	uint8_t spacing9_length = 9;  //defines a variable which holds the spacing9 character quantity
	uint8_t * menu2 = "Type in the port\nyou want to listen to:";  //the menu options to be printed for the user
	uint8_t menu2_length = 39;  //defines a variable which holds the menus character quantity
	menu2 = (void*) menu2;  //casts the menus string from pointer to uint8_t to void
	err_t err;  //declares an error variable
	err = netconn_write(connection, menu2, menu2_length, NETCONN_COPY);  //sends via tcp the menu2
	err = netconn_write(connection, spacing9, spacing9_length, NETCONN_COPY);  //sends via tcp the spacing5 string to clear the string
	return err;  //returns the error variable to know if there was a mistake
}
err_t tcp_printUDPstatus(struct netconn * connection){  //this function prints the UDP status menu
		xEventGroupSetBits(WirelessSpeakers_events, ON_UDP_MENU);	//sets the 'on menu' event in order to tell the queue sender function that it should send somenthing
			UDP_val_t * receiver_values ;//= pvPortMalloc(sizeof(UDP_val_t)); //TODO: quitar el malloc, es para pruebas
			//receiver_values.losses = 12345;
			//receiver_values.received = 23456;
			//receiver_values.relation = 34567;
			xQueueReceive(UDP_status_values,&receiver_values,portMAX_DELAY);
			err_t err;  //declares an error variable
			uint8_t * spacing = "\n\n\n\n\n\n\n";  //string with enters in order to simulate a screen cleaning
			spacing = (void*) spacing;  //casts the spacing string from pointer to uint8_t to void
			uint8_t spacing_length = 7;  //defines a variable which holds the spacing character quantity
			uint8_t * menu3_1 = "comms stats\nUDP packs received per second: ";  //the menu first line to be printed for the user
			uint8_t menu3_1_length = 43;  //defines a variable which holds the menus3.1 character quantity
			menu3_1 = (void*) menu3_1;   //casts the menu 3.1 string from pointer to uint8_t to void
			err = netconn_write(connection, menu3_1, menu3_1_length, NETCONN_COPY);  //sends via tcp the menu 3 first line
			uint8_t value3_1[5];  //declares an array where the UDP received packages value will be hold
			value3_1[0] = (uint8_t)((receiver_values->received/10000)+'0');  //stores the tens of thousand of the UDP received value
			value3_1[1] = (uint8_t)( ( (receiver_values->received - ((value3_1[0]-'0')*10000) ) /1000)+'0' );  //stores the thousands of the UDP received value
			value3_1[2] = ( (receiver_values->received - ((value3_1[0]-'0')*10000)-((value3_1[1]-'0')*1000)) /100 )+'0'; //stores the hundreds of the UDP received value
			value3_1[3] = ( (receiver_values->received - ((value3_1[0]-'0')*10000)-((value3_1[1]-'0')*1000)- ((value3_1[2]-'0')*100) ) /10 )+'0';  //stores the tens of the UDP received value
			value3_1[4] = ( (receiver_values->received - ((value3_1[0]-'0')*10000)-((value3_1[1]-'0')*1000)- ((value3_1[2]-'0')*100) - ((value3_1[3]-'0')*10) ) )+'0';  //stores the units of thousand of the UDP received value
			uint8_t value3_1_length = 5;  //defines a variable which holds the value3.1 character quantity
			uint8_t *value3_1_casted = (void*) value3_1;	//casts the value 3.1 string from pointer to uint8_t to void
			err = netconn_write(connection, value3_1_casted, value3_1_length, NETCONN_COPY);  //writes the value 3.1 through tcp
			uint8_t * menu3_2 = "\nUDP packs lost per second: ";  //the menu second line to be printed for the user
			uint8_t menu3_2_length = 28;  //defines a variable which holds the menus3.2 character quantity
			menu3_2 = (void*) menu3_2;  //casts the menu 3.2 string from pointer to uint8_t to void
			err = netconn_write(connection, menu3_2, menu3_2_length, NETCONN_COPY);  //sends via tcp the menu 3 second line
			uint8_t value3_2[5];  //declares an array where the UDP lost packages value will be hold
			value3_2[0] = (receiver_values->losses/10000)+'0';  //stores the tens of thousand of the UDP losses value
			value3_2[1] = ( (receiver_values->losses - ((value3_2[0]-'0')*10000) ) /1000)+'0';  //stores the thousands of the UDP losses value
			value3_2[2] = ( (receiver_values->losses - ((value3_2[0]-'0')*10000)-((value3_2[1]-'0')*1000)) /100 )+'0';  //stores the thousands of the UDP losses value
			value3_2[3] = ( (receiver_values->losses - ((value3_2[0]-'0')*10000)-((value3_2[1]-'0')*1000)- ((value3_2[2]-'0')*100) ) /10 )+'0';  //stores the tens of the UDP losses value
			value3_2[4] = ( (receiver_values->losses - ((value3_2[0]-'0')*10000)-((value3_2[1]-'0')*1000)- ((value3_2[2]-'0')*100) - ((value3_2[3]-'0')*10) ) )+'0';  //stores the units of thousand of the UDP losses value
			uint8_t value3_2_length = 5; //defines a variable which holds the value3.2 character quantity
			void *value3_2_casted = (void*) value3_2;  //casts the value 3.2 string from pointer to uint8_t to void
			err = netconn_write(connection, value3_2_casted, value3_2_length, NETCONN_COPY);  //writes the value 3.2 through tcp
			uint8_t * menu3_3 = "\nUDP comm quality: ";  //the menu third line to be printed for the user
			uint8_t menu3_3_length = 19; //defines a variable which holds the menus3.3 character quantity
			menu3_3 = (void*) menu3_3;  //casts the menu 3.3 string from pointer to uint8_t to void
			err = netconn_write(connection, menu3_3, menu3_3_length, NETCONN_COPY);  //sends via tcp the menu 3 third line
			uint8_t value3_3[5];  //declares an array where the UDP relation packages value will be hold
			value3_3[0] = (receiver_values->relation/10000)+'0'; //stores the tens of thousand of the UDP relation value
			value3_3[1] = ( (receiver_values->relation - ((value3_3[0]-'0')*10000) ) /1000)+'0';  //stores the thousands of the UDP relation value
			value3_3[2] = ( (receiver_values->relation - ((value3_3[0]-'0')*10000)-((value3_3[1]-'0')*1000)) /100 )+'0'; //stores the thousands of the UDP relation value
			value3_3[3] = ( (receiver_values->relation - ((value3_3[0]-'0')*10000)-((value3_3[1]-'0')*1000)- ((value3_3[2]-'0')*100) ) /10 )+'0';  //stores the tens of the UDP relation value
			value3_3[4] = ( (receiver_values->relation - ((value3_3[0]-'0')*10000)-((value3_3[1]-'0')*1000)- ((value3_3[2]-'0')*100) - ((value3_3[3]-'0')*10) ) )+'0'; //stores the units of thousand of the UDP relation value
			uint8_t value3_3_length = 5;  //defines a variable which holds the value3.3 character quantity
			void *value3_3_casted = (void*) value3_3;  //casts the value 3.3 string from pointer to uint8_t to void
			err = netconn_write(connection, value3_3_casted, value3_3_length, NETCONN_COPY);   //writes the value 3.3 through tcp
			err = netconn_write(connection, spacing, spacing_length, NETCONN_COPY);  //sends via tcp the spacing string to clear the string
			//vPortFree(receiver_values);
			return err;  //returns the error variable to know if there was a mistake
}


typedef enum{
	main_menu,   //0
	port_selector, //1
	comm_status  //2
}menus_t; //enum used to identify the menu to be printed

struct netconn * newconn;
TaskHandle_t UDPStatus;
static void
tcp_thread(void *arg)
{  //this function handles the TCP communication
  struct netconn *conn;  //socket-type variable are declared
  err_t err;  //declares an error variable
  LWIP_UNUSED_ARG(arg);  //consumes the input argument (just for portability)
  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);		     /**creates an empty socket*/
  netconn_bind(conn, IP_ADDR_ANY, 50008);	/**assign a local IP address and port number to the previously created socket*/
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);  //prints and error mesage and ends the function, as socket couldn't be created

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);  //once created, the host goes to listening mode

  static uint8_t first_menu_print = 0;  //variable used to know if the menu has been printed
  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);	//socket connection accepted
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {	//if there was a data request by a client,
      struct netbuf *buf;  //creates a buffer to store the messages sent by the client
      void *data;  //creates a data buffer where the extracted messages will be saved
      u16_t len;  //creates a length variable to know how many values were extracted

      if(0 == first_menu_print){  //if it's the initial menu execution, i.e at connection start,
    	  err = tcp_printMenu(newconn);  //prints the main menu
    	 // xTaskCreate(UDPprinter_caller_task, "UDP status menu printer", configMINIMAL_STACK_SIZE, (void*)newconn, 3, &UDPStatus);
    	  //vTaskSuspend(UDPStatus);
      }

      static uint8_t current_menu = main_menu;  //defines a variable used to know the present menu,
      static uint8_t MusicState = 1;  //variable used to know the DAC output status
      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {	//while the client asks for data,
        /*printf("Recved\n");*/

    	  do {
        	netbuf_data(buf, &data, &len);  //extracts the data from the buffer

        	uint8_t * data_casted = (uint8_t*)data;  //cast of the data extracted from pointer to void to pointer to uint8_T

        	switch(*data_casted){  //switch statement to know which menu to print
        	case 'x':  //in the case where the user wants to return to the main menu,
        		current_menu = main_menu;  //sets the current_menu variable to main menu
        		err = tcp_printMenu(newconn);  //prints the main menu
        		//vTaskSuspend(UDPStatus);
        		break;
        	case '1':  //in the case the user selects the first menu,
        		if(1 == MusicState){ //if the state was previously on, turns it off
        		    AudioOutput(1);
        			MusicState = 0;
        		}else if(0 == MusicState){	//if the state was previously off, turns it on
        			AudioOutput(0);
        			MusicState = 1;
        		}
        		break;
        	case '2':  //in the case the user selects the second menu,
        		if(main_menu == current_menu || port_selector == current_menu){  //if the user is on the main menu or currently in this same menu,
        			current_menu = port_selector;  //sets the current_menu variable to port selector menu
        			err = tcp_printSelector(newconn);  //prints the port selector menu
        			netbuf_delete(buf);  //erases the current tcp input buffer
        			err = netconn_recv(newconn, &buf);  //waits and receives for a new client response
        			uint8_t read_port[10];  //declares an array to receive the new port to be set,
        			netbuf_copy(buf,read_port,10);  //extracts the complete buffer data to an array
        			if('x' != read_port[0]){  //if the client response wasn't a return to the menu
        				uint16_t new_port = ((read_port[0]-'0')*10000) +  //the port tens of thousand are stored,
        						((read_port[1]-'0')*1000) +  //the port thousands are stored,
								((read_port[2]-'0')*100) +  //the port hundreds are stored,
								((read_port[3]-'0')*10) +	//the port tens are stored,
								(read_port[4]-'0');  //the port units are stored,
        				uint16_t * port_transporter;  //a uint16_t pointer variable is declared,
        				port_transporter = pvPortMalloc(sizeof(uint16_t));  //memory is reserved for the pointer
        				*port_transporter = new_port;  //the port_transporter variable is loaded with the new port calculated,
        				xQueueSend(port_selection,&port_transporter,100);	//sends the port selection to the UDP task
        			}
      				current_menu = main_menu; //after the new port is sent, the current menu is stablish as main menu
      			    err = tcp_printMenu(newconn);  //prints the main menu
        		}
        		break;
        	case '3':  //in the case the user selects the third menu,
        		if(main_menu == current_menu || port_selector == current_menu){  //if the user is calling to enter this menu from the main menu,
        			//vTaskResume(UDPStatus);
        			current_menu = comm_status;  //the current menu is set to comm status
        			err = tcp_printUDPstatus(newconn); //prints the UDP status menu
        		}
        		break;
        	}

#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0); //while there is something more in the buffer, loop
        netbuf_delete(buf);  	//the buffer is deleted
      }
      /*printf("Got EOF, looping\n");*/
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);  //the socket is closed
      netconn_delete(newconn); //the socket is deleted
    }
  }
}

/*-----------------------------------------------------------------------------------*/
void
TCP_init(void)
{
  sys_thread_new("tcp_thread", tcp_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
