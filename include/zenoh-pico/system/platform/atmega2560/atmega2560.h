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

#ifndef ZENOH_PICO_SYSTEM_ATMEGA_2560_TYPES_H
#define ZENOH_PICO_SYSTEM_ATMEGA_2560_TYPES_H

#include <stdint.h>  // For uint8_t
#include <Arduino_FreeRTOS.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t z_clock_t;
typedef struct {
    uint32_t sec;  // Seconds since epoch
    uint32_t usec; // microseconds part
} z_time_t;

typedef struct EthernetClient EthernetClient;  // Forward declaration to be used in _z_sys_net_socket_t
typedef struct HardwareSerial HardwareSerial;    // Forward declaration to be used in _z_sys_net_socket_t

typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1 
    EthernetClient * _client;
#endif
#if Z_FEATURE_LINK_SERIAL == 1
        HardwareSerial *_serial;  // As pointer to cross the boundary between C and C++
#endif
    };
} _z_sys_net_socket_t;

typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1
    uint8_t * _ip;
    uint16_t _port;
#endif    
    };
} _z_sys_net_endpoint_t;

#ifdef __cplusplus
}
#endif

#endif // ZENOH_PICO_SYSTEM_ATMEGA_2560_TYPES_H
