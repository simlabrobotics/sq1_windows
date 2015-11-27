/*
 *\brief API for communication over CAN bus 
 *\detailed The API for communicating with the various motor controllers
 *          over the CAN bus interface on the robot hand
 *
 *$Author: Sangyup Yi $
 *$Date: 2012/5/11 23:34:00 $
 *$Revision: 1.0 $
 */ 

#ifndef _CANOPENAPI_H
#define _CANOPENAPI_H

#include "canopenDef.h"
#include "canAPI.h"


CANAPI_BEGIN
CANAPI_EXTERN_C_BEGIN


/******************/
/* CAN device API */
/******************/
// CAN channel:
int can_open(int ch);
int can_open_ex(int ch, int type, int index);
int can_reset(int ch);
int can_close(int ch);

// NMT:
int can_nmt_node_start(int ch, unsigned char node_id); // go to operational state
int can_nmt_node_stop(int ch, unsigned char node_id); // go to prepared(stopped) state
int can_nmt_node_ready(int ch, unsigned char node_id); // enter pre-operational state
int can_nmt_soft_reset(int ch, unsigned char node_id); // perform full software reset
int can_nmt_hard_reset(int ch, unsigned char node_id); // It is recommended that you turn off the motor and kill any user program before executing.

// SYNC and Time Stamp:
int can_sync(int ch);
// Time Stamp:
int can_timestamp(int ch);

// SDO:
int can_sdo_download(int ch, unsigned char node_id, unsigned short obj_index, unsigned char sub_index, unsigned char* buf, unsigned short buf_len); // download data using an SDO
int can_sdo_upload(int ch, unsigned char node_id, unsigned short obj_index, unsigned char sub_index, unsigned char* buf, unsigned short& buf_len); // upload data using an SDO

// SDO queries:
int can_query_device_type(int ch, unsigned char node_id);
int can_query_device_name(int ch, unsigned char node_id);
int can_query_sw_version(int ch, unsigned char node_id);
int can_query_hw_version(int ch, unsigned char node_id);
int can_query_node_id(int ch, unsigned char node_id);
int can_query_status_register(int ch, unsigned char node_id);
int can_query_RxPDO_params(int ch, unsigned char node_id, unsigned char pdo_id);
int can_query_RxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id);
int can_query_TxPDO_params(int ch, unsigned char node_id, unsigned char pdo_id);
int can_query_TxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id);
int can_query_lss_address(int ch, unsigned char node_id);
int can_query_position(int ch, unsigned char node_id);
int can_query_control_word(int ch, unsigned char node_id);
int can_query_status_word(int ch, unsigned char node_id);
int can_query_mode_of_operation_display(int ch, unsigned char node_id);
int can_query_drive_modes(int ch, unsigned char node_id);

// Binary interpreter:
int can_bin_interprete(int ch, unsigned char node_id, unsigned char* buf, unsigned short buf_len);
int can_bin_interprete_get_i(int ch, unsigned char node_id, unsigned char cmd[2], unsigned short index);
int can_bin_interprete_get_f(int ch, unsigned char node_id, unsigned char cmd[2], unsigned short index);
int can_bin_interprete_set_i(int ch, unsigned char node_id, unsigned char cmd[2], unsigned short index, long ival);
int can_bin_interprete_set_f(int ch, unsigned char node_id, unsigned char cmd[2], unsigned short index, float fval);

// Binary interpreter utilities:
int can_bin_query_unit_mode(int ch, unsigned char node_id);
int can_bin_set_unit_mode(int ch, unsigned char node_id, unsigned char um);

// OS interpreter:
int can_os_interprete(int ch, unsigned char node_id, unsigned char* buf, unsigned short buf_len);

// LSS:
int can_lss_switch_mode(int ch, unsigned char node_id, unsigned char mode);

// system initialization & setup:
int can_sys_init(int ch, unsigned char node_id, int period_msec);
int can_map_txpdo1(int ch, unsigned char node_id);
int can_map_txpdo2(int ch, unsigned char node_id);
int can_map_txpdo3(int ch, unsigned char node_id);
int can_map_txpdo4(int ch, unsigned char node_id);
int can_map_rxpdo1(int ch, unsigned char node_id);
int can_map_rxpdo2(int ch, unsigned char node_id);
int can_map_rxpdo3(int ch, unsigned char node_id);
int can_map_rxpdo4(int ch, unsigned char node_id);

// device control:
int can_servo_on(int ch, unsigned char node_id, unsigned short& control_word);
int can_servo_off(int ch, unsigned char node_id, unsigned short& control_word);
int can_set_mode_of_operation(int ch, unsigned char node_id, unsigned char opmode);

// PDO:
int can_pdo_download(int ch, unsigned char node_id, unsigned char pdo_id, unsigned char* data, unsigned char len);

// PDO tranmitions:
int can_pdo_set_target_position(int ch, unsigned char node_id, int target_position, unsigned short& control_word);
int can_pdo_rx1(int ch, unsigned char node_id, int target_position, unsigned int profile_velocity);
int can_pdo_rx3(int ch, unsigned char node_id, unsigned short& control_word, unsigned char mode_of_operation);

//
int can_set_homing_params(int ch, unsigned char node_id, long offset, char method, unsigned long speed1, unsigned long speed2, unsigned long acceleration);
int can_set_target_position(int ch, unsigned char node_id, int target_position, unsigned short& control_word);
int can_write_PT(int ch, unsigned char node_id, unsigned short position);
int can_store_params(int ch, unsigned char node_id);
int can_restore_params(int ch, unsigned char node_id);

// utilities:
int can_dump_slave(int ch, unsigned char node_id);
int can_dump_motion_profile(int ch, unsigned char node_id);
int can_dump_factors(int ch, unsigned char node_id);
int can_dump_homing_params(int ch, unsigned char node_id);
int can_get_message(int ch, unsigned char& fn_code, unsigned char& node_id, unsigned char& len, unsigned char* data, bool blocking);
int can_flush(int ch, unsigned char node_id);

CANAPI_EXTERN_C_END
CANAPI_END

#endif
