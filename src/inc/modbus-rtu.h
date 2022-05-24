/*
 * modbus-rtu.h
 *
 *  Created on: Jul 6, 2020
 *      Author: user
 */

#ifndef MODBUS_RTU_H_
#define MODBUS_RTU_H_

#include <modbus.h>
#include <modbus-private.h>


 /* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
  * RS232 / RS485 ADU = 253 bytes + slave (1 byte) + CRC (2 bytes) = 256 bytes
  */
 #define MODBUS_RTU_MAX_ADU_LENGTH  256
  modbus_t* modbus_new_rtu(const char *device, int baud, char parity,
                                 int data_bit, int stop_bit);

 #define MODBUS_RTU_RS232 0
 #define MODBUS_RTU_RS485 1

  int modbus_rtu_set_serial_mode(modbus_t *ctx, int mode);
  int modbus_rtu_get_serial_mode(modbus_t *ctx);

 #define MODBUS_RTU_RTS_NONE   0
 #define MODBUS_RTU_RTS_UP     1
 #define MODBUS_RTU_RTS_DOWN   2

  int modbus_rtu_set_rts(modbus_t *ctx, int mode);
  int modbus_rtu_get_rts(modbus_t *ctx);



#endif /* MODBUS_RTU_H_ */
