/*
 *\brief API for communication over CAN bus 
 *\detailed The API for communicating with the various motor controllers
 *          over the CAN bus interface on the robot hand
 *
 *$Author: Sangyup Yi $
 *$Date: 2012/5/11 23:34:00 $
 *$Revision: 1.0 $
 */ 

#ifndef _CANDAPI_H
#define _CANDAPI_H

#include "canDef.h"

CANAPI_BEGIN

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif

/*=====================*/
/*       Defines       */
/*=====================*/
//constants
#define TX_QUEUE_SIZE       (32)
#define RX_QUEUE_SIZE       (32)
#define TX_TIMEOUT          (5)
#define RX_TIMEOUT          (5)
#define mbxID               (0)
#define BASE_ID             (0)
#define MAX_BUS             (256)

/******************/
/* CAN device API */
/******************/
// CAN channel:
int can_open(int ch);
int can_open_ex(int ch, int type, int index);
int can_reset(int ch);
int can_close(int ch);

// utilities:
int can_query_object(int ch, unsigned char node_id, unsigned short obj_index, unsigned char sub_index, unsigned char* buf, unsigned short& buf_len);  // send single SDO query
int can_dump_slave(int ch, unsigned char node_id);
int can_get_message(int ch, unsigned char& fn_code, unsigned char& node_id, unsigned char& len, unsigned char* data, bool blocking);

// NMT:
int can_nmt_change_state(int ch, unsigned char node_id);
int can_nmt_query_state(int ch, unsigned char node_id);

// object dictionary:
int can_sys_init(int ch, unsigned char node_id, int period_msec);
int can_pdo_map(int ch, unsigned char node_id);
int can_set_mode_of_operation(int ch, unsigned char node_id, unsigned short opmode);
int can_servo_on(int ch, unsigned char node_id);
int can_servo_off(int ch, unsigned char node_id);
int can_write_PT(int ch, unsigned char node_id, unsigned short position);
int can_store_params(int ch, unsigned char node_id);
int can_restore_params(int ch, unsigned char node_id);

// OD(object dictionary):
int can_query_device_type(int ch, unsigned char node_id);
int can_query_device_name(int ch, unsigned char node_id);
int can_query_sw_version(int ch, unsigned char node_id);
int can_query_hw_version(int ch, unsigned char node_id);
int can_query_node_id(int ch, unsigned char node_id);
int can_query_RxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id);
int can_query_TxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id);
int can_query_lss_address(int ch, unsigned char node_id);
int can_query_position(int ch, unsigned char node_id);

// LSS:
int can_lss_switch_mode(int ch, unsigned char node_id, unsigned char mode);

// NMT:
int can_nmt_node_start(int ch, unsigned char node_id); // go to operational state
int can_nmt_node_stop(int ch, unsigned char node_id); // go to prepared(stopped) state
int can_nmt_node_ready(int ch, unsigned char node_id); // enter pre-operational state
int can_nmt_soft_reset(); // perform full software reset
int can_nmt_hard_reset(); // It is recommended that you turn off the motor and kill any user program before executing.

CANAPI_END

#endif
