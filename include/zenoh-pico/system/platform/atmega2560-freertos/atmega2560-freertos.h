//
// Copyright (c) 2023 Fictionlab sp. z o.o.
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   Błażej Sowa, <blazej@fictionlab.pl>

#ifndef ZENOH_PICO_SYSTEM_ATMEGA_2560_FREERTOS_TYPES_H
#define ZENOH_PICO_SYSTEM_ATMEGA_2560_FREERTOS_TYPES_H

#include <time.h>
#include <stdio.h>   // For sscanf
#include <string.h>  // For strlen
#include <stdint.h>  // For uint8_t
#include <stdbool.h> // For bool

#include "FreeRTOS.h"

typedef TickType_t z_clock_t;
typedef struct timeval z_time_t;

typedef int8_t (*Socket_t)(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag);
typedef int8_t (*SocketConnect_t)(uint8_t sn, uint8_t * addr, uint16_t port);
typedef int8_t (*SocketListen_t)(uint8_t sn);
typedef int8_t (*SocketClose_t)(uint8_t sn);
typedef int32_t (*SocketReceive_t)(uint8_t sn, uint8_t * buf, uint16_t len);
typedef int32_t (*SocketSend_t)(uint8_t sn, uint8_t * buf, uint16_t len);

typedef struct {
    union {
        Socket_t _socket;
        SocketConnect_t _connect;
        SocketListen_t _listen;
        SocketClose_t _close;
        SocketReceive_t _receive;
        SocketSend_t _send;

        uint8_t _number;
        uint16_t _port;
    };
} _z_sys_net_socket_t;

typedef struct {
    union {
        uint8_t * _ip;
        uint16_t _port;
    };
} _z_sys_net_endpoint_t;

#endif // ZENOH_PICO_SYSTEM_ATMEGA_2560_FREERTOS_TYPES_H
