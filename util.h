/********************************************************
 * util.h
 *
 * SER486 Final Project
 * Fall 2021
 * Author: Jesse Baker (student jjbaker4)
 * Date: 2021-12-1
 *
 * This file provides function declarations for various
 * utility functions for SER486.
 * 
 */

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#ifdef __cplusplus
   extern "C" {
#endif
/* update the checksum (last byte) of the specifed data so
* that the sum of all the bytes will be zero
*/
void update_checksum(unsigned char *data, unsigned int dsize);

/* returns 1 if the sum of all the bytes in the data structure is 0.
* Otherwise, returns 0.
*/
int is_checksum_valid(unsigned char *data, unsigned int dsize);

/* dump the contents of the eeprom (instructor provided code) */
void dump_eeprom(unsigned int start_address, unsigned int numbytes);

/**********************************
 * update_tcrit_hi()
 *
 * Update the configuration tcrit_hi limit with the specified value.
 * This function is called by the packet command parser. It should
 * perform range checking on the value. Valid inputs for tcrit_ hi
 * must be greater than twarn_hi but less than or equal to 0x3FF.
 *
 * arguments:
 *  value - integer value to set high critical temp to
 *
 * returns:
 *  integer 1 for successful, and 0 for error
 *
 * changes:
 *  none
 */
 int update_tcrit_hi(int value);

 /**********************************
 * update_twarn_hi()
 *
 * Update the configuration twarn_hi limit with the specified value.
 * This function is called by the packet command parser. It should
 * perform range checking on the value. Valid inputs for twarn_hi
 * must be greater than twarn_lo and less than tcrit_hi.
 *
 * arguments:
 *  value - integer value to set high warning temp to
 *
 * returns:
 *  integer 1 for successful, and 0 for error
 *
 * changes:
 *  none
 */
 int update_twarn_hi(int value);

 /**********************************
 * update_tcrit_lo()
 *
 * Update the configuration tcrit_lo limit with the specified value.
 * This function is called by the packet command parser. It should
 * perform range checking on the value. Valid inputs for tcrit_lo
 * must be less than twarn_lo.
 *
 * arguments:
 *  value - integer value to set low warning temp to
 *
 * returns:
 *  integer 1 for successful, and 0 for error
 *
 * changes:
 *  none
 */
 int update_tcrit_lo(int value);

 /**********************************
 * update_twarn_lo()
 *
 * Update the configuration twarn_ lo limit with the specified value.
 * This function is called by the packet command parser. It should
 * perform range checking on the value. Valid inputs for twarn_ lo
 * must be greater than tcrit_lo and less than twarn_hi
 *
 * arguments:
 *  value - integer value to set low warning temp to
 *
 * returns:
 *  integer 1 for successful, and 0 for error
 *
 * changes:
 *  none
 */
 int update_twarn_lo(int value);

#ifdef __cplusplus
   }
#endif

#endif // UTIL_H_INCLUDED
