/*
 * modbus-tcp.h
 *
 *  Created on: Jul 6, 2020
 *      Author: user
 */

#ifndef MODBUS_TCP_H_
#define MODBUS_TCP_H_

#include <modbus.h>
#include <modbus-private.h>



#define MODBUS_TCP_DEFAULT_PORT   502
#define MODBUS_TCP_SLAVE         0xFF

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes
 */
#define MODBUS_TCP_MAX_ADU_LENGTH  260

modbus_t* modbus_new_tcp(const char *ip_address, int port);
int modbus_tcp_listen(modbus_t *ctx, int nb_connection);
int modbus_tcp_accept(modbus_t *ctx, int *s);

modbus_t* modbus_new_tcp_pi(const char *node, const char *service);
int modbus_tcp_pi_listen(modbus_t *ctx, int nb_connection);
int modbus_tcp_pi_accept(modbus_t *ctx, int *s);


//==========private==============================================================

#define _MODBUS_TCP_HEADER_LENGTH      7
#define _MODBUS_TCP_PRESET_REQ_LENGTH 12
#define _MODBUS_TCP_PRESET_RSP_LENGTH  8

#define _MODBUS_TCP_CHECKSUM_LENGTH    0

/* In both structures, the transaction ID must be placed on first position
   to have a quick access not dependant of the TCP backend */
typedef struct _modbus_tcp {
    /* Extract from MODBUS Messaging on TCP/IP Implementation Guide V1.0b
       (page 23/46):
       The transaction identifier is used to associate the future response
       with the request. This identifier is unique on each TCP connection. */
    uint16_t t_id;
    /* TCP port */
    int port;
    /* IP address */
    char ip[16];
} modbus_tcp_t;

#define _MODBUS_TCP_PI_NODE_LENGTH    1025
#define _MODBUS_TCP_PI_SERVICE_LENGTH   32

typedef struct _modbus_tcp_pi {
    /* Transaction ID */
    uint16_t t_id;
    /* TCP port */
    int port;
    /* Node */
    char node[_MODBUS_TCP_PI_NODE_LENGTH];
    /* Service */
    char service[_MODBUS_TCP_PI_SERVICE_LENGTH];
} modbus_tcp_pi_t;

//=============================================================================


#endif /* MODBUS_TCP_H_ */
