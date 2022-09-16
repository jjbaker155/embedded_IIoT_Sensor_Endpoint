/********************************************************
 * util.c
 *
 * SER486 Final Project
 * Fall 2021
 * Author: Jesse Baker (student jjbaker4)
 * Date: 2021-12-1
 *
 * This file implements utility functions for Final Project,
 * that are used to update the temperature warning/critical thresholds
 *
 *
 * functions:
 *
 * update_tcrit_hi()
 *  Update the configuration tcrit_hi limit with the specified value.
 *  This function is called by the packet command parser.
 *
 * update_twarn_hi()
 *  Update the configuration twarn_hi limit with the specified value.
 *  This function is called by the packet command parser.
 *
 * update_tcrit_lo()
 *  Update the configuration tcrit_lo limit with the specified value.
 *  This function is called by the packet command parser.
 *
 * update_twarn_lo()
 *  Update the configuration twarn_lo limit with the specified value
 *  This function is called by the packet command parser.
 */

 #include "config.h"


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
 int update_tcrit_hi(int value){
     if(value > config.hi_warn && value <= 0x3FF && value != config.hi_alarm){
        config.hi_alarm = value;
        return 1;
     }
    return 0;
 }

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
 int update_twarn_hi(int value){
    if(value > config.lo_warn && value < config.hi_alarm && value != config.hi_warn){
        config.hi_warn = value;
        return 1;
     }
    return 0;
 }

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
 int update_tcrit_lo(int value){
     if(value < config.lo_warn && value != config.lo_alarm){
        config.lo_alarm = value;
        return 1;
     }
    return 0;
 }

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
 int update_twarn_lo(int value){
    if(value < config.hi_warn && value > config.lo_alarm && value != config.lo_warn){
        config.lo_warn = value;
        config_set_modified();
        return 1;
     }
    return 0;
 }
