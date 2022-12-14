/*************************************************************************
* main.c
* 
* SER486 Final Project
* Written By: Jesse Baker (student jjbaker4)
* 
* This file contains the main() method for the SER486 final project
*/
#include "config.h"
#include "delay.h"
#include "dhcp.h"
#include "led.h"
#include "log.h"
#include "rtc.h"
#include "spi.h"
#include "uart.h"
#include "vpd.h"
#include "temp.h"
#include "socket.h"
#include "alarm.h"
#include "wdt.h"
#include "tempfsm.h"
#include "eeprom.h"
#include "ntp.h"
#include "w51.h"
#include "signature.h"
#include "httpparser.h"

#define HTTP_PORT       8080	/* TCP port for HTTP */
#define SERVER_SOCKET   0

int current_temperature = 75;

int main(void)
{
	/* Initialize the hardware devices*/
    config_init();
    uart_init();
    led_init();
    vpd_init();
    log_init();
    rtc_init();
    spi_init();
    temp_init();
    W5x_init();
    tempfsm_init();
    httpparser_init();


    uart_writestr("SER486 Final Project\r\n");
    uart_writestr("Jesse Baker");
    uart_writestr("\r\n");


    //this is intended to ensure that the socket is truely closed at startup
    socket_open(SERVER_SOCKET, HTTP_PORT);
    do{
                socket_flush_line(SERVER_SOCKET);
            } while(socket_recv_available(SERVER_SOCKET)>0 || socket_received_line(SERVER_SOCKET));
    socket_close(SERVER_SOCKET);

    /*Assignment signature*/
    signature_set("Jesse","Baker","jjbaker4");

    /* configure the W51xx ethernet controller prior to DHCP */
    unsigned char blank_addr[] = {0,0,0,0};
    W5x_config(vpd.mac_address, blank_addr, blank_addr, blank_addr);

    /* loop until a dhcp address has been gotten */
    while (!dhcp_start(vpd.mac_address, 60000UL, 4000UL)) {}
    uart_writestr("local ip: ");uart_writeip(dhcp_getLocalIp());

    /* configure the MAC, TCP, subnet and gateway addresses for the Ethernet controller*/
    W5x_config(vpd.mac_address, dhcp_getLocalIp(), dhcp_getGatewayIp(), dhcp_getSubnetMask());

	/* add a log record for EVENT_TIMESET prior to synchronizing with network time */
	log_add_record(EVENT_TIMESET);

    /* synchronize with network time */
    ntp_sync_network_time(5);

    /* add a log record for EVENT_NEWTIME now that time has been synchronized */
    log_add_record(EVENT_NEWTIME);

    /* start the watchdog timer */
    wdt_init();

    /* log the EVENT STARTUP and send and ALARM to the Master Controller */
    log_add_record(EVENT_STARTUP);
    alarm_send(EVENT_STARTUP);

    /* request start of test if 'T' key pressed - You may run up to 3 tests per
     * day.  Results will be e-mailed to you at the address asurite@asu.edu
     */
    check_for_test_start();

    /* start the first temperature reading and wait 5 seconds before reading again
    * this long delay ensures that the temperature spike during startup does not
    * trigger any false alarms.
    */
    temp_start();
    delay_set(1,5000);


    while (1) {
        /* reset  the watchdog timer every loop */
        wdt_reset();

        /* update the LED blink state */
        led_update();

        /* if the temperature sensor delay is done, update the current temperature
        * from the temperature sensor, update the temperature sensor finite state
        * machine (which provides hysteresis) and send any temperature sensor
        * alarms (from FSM update).
        */
        if(delay_isdone(1)){
            /* read the temperature sensor */
            current_temperature = temp_get();
            /* update the temperature fsm and send any alarms associated with it */
            tempfsm_update(current_temperature,config.hi_alarm,config.hi_warn,config.lo_alarm,config.lo_warn);
            /* restart the temperature sensor delay to trigger in 1 second */
            temp_start();
            delay_set(1,1000);
        }
        /*if the server socket is closed */
            /* if socket is closed, open it in passive (listen) mode */
        if(socket_is_closed(SERVER_SOCKET)){
            socket_open(SERVER_SOCKET, HTTP_PORT);
            socket_listen(SERVER_SOCKET);
            uart_writestr("Socket is now open and listening\r\n");
            parser_state = WAIT;
        }
        else if(socket_is_active(SERVER_SOCKET)){
            if(socket_is_established(SERVER_SOCKET)){
                if(socket_received_line(SERVER_SOCKET)){
                    parser_state = ID_TYPE;
                    parse_http(SERVER_SOCKET);
                }
            }
        }
        /* update any pending config write backs */
        if (!eeprom_isbusy()) {
            config_update();
        }
        /* update any pending log write backs */
        if (!eeprom_isbusy()){
            log_update();
        }
    }
	return 0;
}
