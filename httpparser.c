/********************************************************
 * httpparser.c
 *
 * SER486 Final Project
 * Fall 2021
 * Author: Jesse Baker (student jjbaker4)
 * Date: 2021-12-1
 *
 * This file implements a finite state machine for parsing http
 * requests to the Atmega hardware
 *
 * Functions:
 *
 * parse_http()
 *  Parses the received http formatted text and takes appropriate actions
 *  This function is the finite state machine and will loop until all actions
 *  are completed. This includes returning an error message and exiting.
 *
 * send_json_device_info()
 *  Prepares and sends a json string which represents a
 *  status summary of the device
 *
 * create_error_response()
 *  Prepares and sends a 400 error response containing
 *  containing a description provided in the parameter
 *
 * get_state()
 *  Retreives temp info and uses it to calculate and return alarm state
 *
 * apply_config_changes()
 *  Applies config changes received via PUT req
 *
 * clear_junk()
 *  removes the specified number of characters (up to 254) from the receive buffer
 *  or stop early if the abort character is encountered
 *
 *
 */

 #include "httpparser.h"
 #include "socket.h"
 #include "vpd.h"
 #include "config.h"
 #include "temp.h"
 #include "log.h"
 #include "util.h"
 #include "uart.h"
 #include "rtc.h"
 #include "wdt.h"

 #define MAX_TEMP 0x3FF

 enum put_config_operation {TCRIT_HI, TCRIT_LO, TWARN_HI, TWARN_LO, ERROR_OP};
 enum put_config_operation config_op;


 /**********************************
 * create_error_response()
 *
 * Prepares and sends a 400 error response containing
 * containing a description provided in the parameter
 *
 * arguments:
 *  socket - unsigned char that represents the socket
 *  msg - string with a description of the error
 *
 * returns:
 *  none
 *
 * changes:
 *  none
 */
 static void create_error_response(unsigned char socket, char* msg){
    socket_writestr(socket, "HTTP/1.1 400 Bad Request\r\n"); //TODO: can sub in my message for "bad request"
    //TODO: How to get web browser to display error
    //socket_writestr(socket, "Content-Type: application/vnd.api+json\r\n");
    socket_writestr(socket, "Connection: close\r\n");
    socket_writestr(socket, "\r\n");
 }


 /**********************************
 * get_state()
 *
 * Retreives temp info and uses it to calculate alarm state
 *
 * arguments:
 *  none
 *
 * returns:
 *  char array representing the state
 *
 * changes:
 *  none
 */
 static char* get_state(){
    if(temp_get() >= config.hi_alarm){
        return "CRIT_HI";
    }
    else if(temp_get() >= config.hi_warn){
        return "WARN_HI";
    }
    else if(temp_get() <= config.lo_alarm){
        return "CRIT_LO";
    }
    else if(temp_get() <= config.lo_warn){
        return "WARN_LO";
    }
    else{
        return "NORMAL";
    }
 }

/**********************************
 * apply_config_changes()
 *
 * Applies config changes received via PUT req
 *
 * arguments:
 *  operation - enum put_config_operation indicating what will be changed
 *  socket - unsigned char that represents the socket we will receive the new setting from
 *
 * returns:
 *  success - int 1 for success, 0 for fail
 *
 * changes:
 *  none
 */
 static int apply_config_changes(enum put_config_operation setting, unsigned char socket){
    int input;
    if(!socket_recv_int(socket, &input)){
        create_error_response(socket, "Invalid config parameter type for PUT request (int required)");
        parser_state = FLUSH;
        return 0;
    }
    else{
        switch(setting){
        case TCRIT_HI:
            return update_tcrit_hi(input);
            break;
        case TCRIT_LO:
            return update_tcrit_lo(input);
            break;
        case TWARN_HI:
            return update_twarn_hi(input);
            break;
        case TWARN_LO:
            return update_twarn_lo(input);
            break;
        default:
            create_error_response(socket, "Error updating temperature settings");
            return 0;
            break;
        }
    }
 }

 /**********************************
 * send_json_device_info()
 *
 * Prepares and sends a json string which represents a
 * status summary of the device
 *
 * arguments:
 *  socket - unsigned char represents an instance of a server socket
 *
 * returns:
 *  none
 *
 * changes:
 *  none
 */
 static void send_json_device_info(unsigned char socket){
     //TODO: FIX TIMESTAMP ISSUE

    //write <CRLF> line as separator
    socket_writestr(socket, "\r\n");

    socket_writechar(socket, '{'); //open outer object

    //VPD Object
    socket_writequotedstring(socket, "vpd");
    socket_writechar(socket, ':');
    socket_writechar(socket, '{'); //open vpd object
    socket_writequotedstring(socket, "model");
    socket_writechar(socket, ':');
    socket_writequotedstring(socket, vpd.model);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "manufacturer");
    socket_writechar(socket, ':');
    socket_writequotedstring(socket, vpd.manufacturer);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "serial_number");
    socket_writechar(socket, ':');
    socket_writequotedstring(socket, vpd.serial_number);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "manufacture_date");
    socket_writechar(socket, ':');
    socket_writedate(socket, vpd.manufacture_date);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "mac_address");
    socket_writechar(socket, ':');
    socket_write_macaddress(socket, vpd.mac_address);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "country_code");
    socket_writechar(socket, ':');
    socket_writequotedstring(socket, vpd.country_of_origin);
    socket_writechar(socket, '}'); //close vpd object

    socket_writechar(socket, ',');

    //general info
    socket_writequotedstring(socket, "tcrit_hi");
    socket_writechar(socket, ':');
    socket_writedec32(socket, config.hi_alarm);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "twarn_hi");
    socket_writechar(socket, ':');
    socket_writedec32(socket, config.hi_warn);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "tcrit_lo");
    socket_writechar(socket, ':');
    socket_writedec32(socket, config.lo_alarm);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "twarn_lo");
    socket_writechar(socket, ':');
    socket_writedec32(socket, config.lo_warn);
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "temperature");
    socket_writechar(socket, ':');
    socket_writedec32(socket, temp_get());
    socket_writechar(socket, ',');
    socket_writequotedstring(socket, "state");
    socket_writechar(socket, ':');
    socket_writequotedstring(socket, get_state());

    socket_writechar(socket, ',');


    //Log array
    socket_writequotedstring(socket, "log");
    socket_writechar(socket, ':');
    socket_writechar(socket, '['); //start log array

    //create log entry JSON objects and write them
    unsigned char i;

    for (i=0; i < log_get_num_entries(); i++){
        unsigned long time;
        unsigned char event_num;
        log_get_record(i, &time, &event_num);

        socket_writechar(socket, '{'); //open log object

        socket_writequotedstring(socket, "timestamp");
        socket_writechar(socket, ':');
        socket_writequotedstring(socket, rtc_num2datestr(time));
        socket_writechar(socket, ',');
        socket_writequotedstring(socket, "event");
        socket_writechar(socket, ':');
        socket_writedec32(socket, (int)event_num);

        socket_writechar(socket, '}'); //close log object

        //HEADS UP! - This should be fine, but keep an eye on it
        if(i < log_get_num_entries()-1){
            socket_writechar(socket, ',');
        }
    }
    socket_writechar(socket, ']'); //end log array

    socket_writechar(socket, '}'); //close outer object
 }

 /**********************************
 * clear_junk()
 *
 * removes the specified number of characters (up to 254) from the receive buffer
 * or stop early if the abort character is encountered
 *
 * arguments:
 *  socket - unsigned char represents an instance of a server socket
 *  how_much_junk - unsigned char represents how many characters should be removed?
 *  abort_char - char represents the character that should cause junk collection to stop early
 *
 * returns:
 *  none
 *
 * changes:
 *  none
 */
 static void clear_junk(unsigned char socket, unsigned char how_much_junk, char abort_character){
    if (how_much_junk > 254){
        how_much_junk = 254;
    }
    unsigned char junk;
    unsigned char i = 0;
    while (i<how_much_junk){
            socket_peek(socket, &junk);
            if(junk == abort_character){
                i = 255;
            }
            else{
                socket_recv(socket, &junk, 1);
                i++;
            }
    }
 }

 static void send_ok(unsigned char socket){
    socket_writestr(socket, "HTTP/1.1 200 OK\r\n");
    socket_writestr(socket, "Connection: close\r\n");
 }
/**********************************
 * parse_http()
 *
 * Parses the received http formatted text and takes appropriate actions
 * This function is the finite state machine and will loop until all actions
 * are completed. This includes returning an error message and exiting.
 *
 * arguments:
 *  socket - unsigned char represents an instance of a server socket
 *
 * returns:
 *  none
 *
 * changes:
 *  none
 */
 void parse_http(unsigned char socket){
    //uart_writestr("Starting http Parse\r\n");

    while(parser_state != DONE){
        switch(parser_state){
        case ID_TYPE:
            if(socket_recv_compare(socket, "GET ")){
                parser_state = GET;
                clear_junk(socket, 18, '/');
            } else if(socket_recv_compare(socket, "PUT ")){
                parser_state = PUT;
                clear_junk(socket, 18, '/');
            } else if(socket_recv_compare(socket, "DELETE ")){
                parser_state = DELETE;
                clear_junk(socket, 18, '/');
            } else{
                create_error_response(socket, "Invalid Request Type");
                parser_state = FLUSH;
            }
            break;
        case GET:
            //check URI/Endpoint
            if(socket_recv_compare(socket, "/device ")){
                socket_writestr(socket, "HTTP/1.1 200 OK\r\n");
                socket_writestr(socket, "Content-Type: application/vnd.api+json\r\n");
                socket_writestr(socket, "Connection: close\r\n");

                socket_writestr(socket, "\r\n"); //start of message body
                send_json_device_info(socket);
                socket_writestr(socket, "\r\n"); //end of message body
            } else{
                create_error_response(socket, "Invalid endpoint for GET request");
            }
            parser_state = FLUSH;
            break;
        case PUT:
            //check URI/Endpoint
            if(socket_recv_compare(socket, "/device/config")){
                if (socket_recv_compare(socket, "?")){
                    parser_state = APPLY_CHANGES;
                } else{
                    create_error_response(socket, "Invalid PUT request");
                    parser_state = FLUSH;
                    break;
                }
            } else if(socket_recv_compare(socket, "/device")){
                if (socket_recv_compare(socket, "?reset=")){
                    parser_state = RESET;
                } else{
                    create_error_response(socket, "Invalid PUT request");
                    parser_state = FLUSH;
                    break;
                }
            }
            else{
                create_error_response(socket, "Invalid endpoint for PUT request");
                parser_state = FLUSH;
            }
            break;
        case RESET:
            if (socket_recv_compare(socket, "\"true\"")){
                send_ok(socket);
                socket_disconnect(socket);
                //reset machine
                wdt_force_restart();
            } else if(socket_recv_compare(socket, "\"false\"")){
                //do nothing - just close connection with ok...?
                send_ok(socket);
            } else{
                create_error_response(socket, "Not a valid option for reset setting");
            }
            parser_state = FLUSH;
            break;
        case DELETE:
            //check URI/Endpoint
            if(socket_recv_compare(socket, "/device/log")){
                log_clear();
                send_ok(socket);
            } else{
                create_error_response(socket, "Invalid endpoint for DELETE request");
            }
            parser_state = FLUSH;
            break;
        case APPLY_CHANGES:
            if(socket_recv_compare(socket, "twarn_hi=")){
                if(apply_config_changes(TWARN_HI, socket)){
                    config_set_modified();
                    send_ok(socket);
                } else{
                    create_error_response(socket, "Invalid high warning temperature");
                }
            }
            else if(socket_recv_compare(socket, "twarn_lo=")){
                if(apply_config_changes(TWARN_LO, socket)){
                    config_set_modified();
                    send_ok(socket);
                }else{
                    create_error_response(socket, "Invalid low warning temperature");
                }
            }
            else if(socket_recv_compare(socket, "tcrit_hi=")){
                if(apply_config_changes(TCRIT_HI, socket)){
                    config_set_modified();
                    send_ok(socket);
                }else{
                    create_error_response(socket, "Invalid critical high temperature");
                }
            }
            else if(socket_recv_compare(socket, "tcrit_lo=")){
                if(apply_config_changes(TCRIT_LO, socket)){
                    config_set_modified();
                    send_ok(socket);
                } else{
                    create_error_response(socket, "Invalid critical low temperature");
                }
            }
            else{
                create_error_response(socket, "Invalid config parameter name for PUT request");
            }
            parser_state = FLUSH;
            break;
        case FLUSH:
            do{
                socket_flush_line(socket);
            } while(socket_recv_available(socket)>0 || socket_received_line(socket));
            socket_disconnect(socket);
            parser_state = DONE;
            break;
        case DONE:
            //nothing - unreachable
            break;
        default:
            break;
        }
    }
 }

/**********************************
 * httpparser_init()
 *
 * Inits parser to default DONE state
 *
 * arguments:
 *  none
 *
 * returns:
 *  none
 *
 * changes:
 *  none
 */
 void httpparser_init(){
    parser_state = DONE;
 }
