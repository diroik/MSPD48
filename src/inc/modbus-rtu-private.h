/*
 * modbus-rtu-private.h
 *
 *  Created on: Jul 6, 2020
 *      Author: user
 */

#ifndef MODBUS_RTU_PRIVATE_H_
#define MODBUS_RTU_PRIVATE_H_

#include <stdint.h>
#include <termios.h>

#define _MODBUS_RTU_HEADER_LENGTH      1
#define _MODBUS_RTU_PRESET_REQ_LENGTH  6
#define _MODBUS_RTU_PRESET_RSP_LENGTH  2

#define _MODBUS_RTU_CHECKSUM_LENGTH    2

/* Time waited beetween the RTS switch before transmit data or after transmit
   data before to read */
#define _MODBUS_RTU_TIME_BETWEEN_RTS_SWITCH 10000

typedef struct _modbus_rtu {
    /* Device: "/dev/ttyS0", "/dev/ttyUSB0" or "/dev/tty.USA19*" on Mac OS X. */
    char *device;
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;
#if defined(_WIN32)
    struct win32_ser w_ser;
    DCB old_dcb;
#else
    /* Save old termios settings */
    struct termios old_tios;
#endif
#if HAVE_DECL_TIOCSRS485
    int serial_mode;
#endif
#if HAVE_DECL_TIOCM_RTS
    int rts;
    int onebyte_time;
#endif
    /* To handle many slaves on the same link */
    int confirmation_to_ignore;
} modbus_rtu_t;



#endif /* MODBUS_RTU_PRIVATE_H_ */
