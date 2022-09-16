/********************************************************
 * httpparser.h
 *
 * SER486 Final Project
 * Fall 2021
 * Author: Jesse Baker (student jjbaker4)
 * Date: 2021-12-1
 *
 * This file provides declarations for the httpparser.c file
 */

#ifndef HTTPPARSER_H_INCLUDED
#define HTTPPARSER_H_INCLUDED

enum http_parser_state {WAIT, ID_TYPE, GET, PUT, DELETE, APPLY_CHANGES, FLUSH, DONE, RESET};
enum http_parser_state parser_state;

/**********************************
 * parse_http()
 *
 * Parses the received http formatted text and takes appropriate actions
 * This function is the finite state machine and will loop until all actions
 * are completed. This includes returning an error message and exiting.
 */
void parse_http(unsigned char s);

/**********************************
 * httpparser_init()
 *
 * Inits parser to default DONE state
 */
void httpparser_init();

#endif // HTTPPARSER_H_INCLUDED
