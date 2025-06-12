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

#include "FreeRTOS.h"

typedef TickType_t z_clock_t;
typedef struct timeval z_time_t;

// TODO(giafranchini): these are the 2 wrapper types for tcp/ip driver of W5100
typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1 || Z_FEATURE_LINK_UDP_UNICAST == 1
        Socket_t _socket;
#endif
    };
} _z_sys_net_socket_t;

typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1 || Z_FEATURE_LINK_UDP_UNICAST == 1
        struct freertos_addrinfo *_iptcp;
#endif
    };
} _z_sys_net_endpoint_t;

#endif
