#include "zenoh-pico/system/platform.h"
#include "zenoh-pico/transport/transport.h"
#include "zenoh-pico/utils/pointers.h"
#include "zenoh-pico/utils/result.h"

// FreeRTOS includes
#include "FreeRTOS.h"

// ioLibrary_Driver includes for W5100
#include "socket.h"
#include "wizchip_conf.h"

z_result_t _z_open_tcp(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep, uint32_t tout) {
    z_result_t ret = _Z_RES_OK;

    // Initialize the W5100
    // wizchip_setup();

    if (sock->_socket(sock->_number, Sn_MR_TCP, sock->_port, 0) != sock->_number) {
        // TODO(giafranchini): error handling
        ret = _Z_ERR_GENERIC;
    }

    if (sock->_connect(sock->_number, rep._ip, rep._port) != SOCK_OK) {
        // TODO(giafranchini): error handling
        ret = _Z_ERR_GENERIC;
    }
    
    return ret;
}

z_result_t _z_listen_tcp(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep) {

    // Initialize the W5100
    // wizchip_setup();

    z_result_t ret = _Z_RES_OK;

    if (sock->_socket(sock->_number, Sn_MR_TCP, sock->_port, 0) != sock->_number) {
        // TODO(giafranchini): error handling
        ret = _Z_ERR_GENERIC;
    }

    if (sock->_connect(sock->_number, rep._ip, rep._port) != SOCK_OK) {
        // TODO(giafranchini): error handling
        ret = _Z_ERR_GENERIC;
    }

    ret = sock->_listen(sock->_number);
    if (ret != SOCK_OK) {
        // TODO(giafranchini): error handling
        ret = _Z_ERR_GENERIC;
    }

    return ret;
}

void _z_close_tcp(_z_sys_net_socket_t *sock) {sock->_close(sock->_number);}

size_t _z_read_tcp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
    
    // Initialize the W5100
    // wizchip_setup();

    BaseType_t rb = sock._receive(sock._number, ptr, len);
    if(rb != len) {
        // TODO(giafranchini): error handling
        // As in FreeRTOS+TCP SIZE_MAX
        rb =  SIZE_MAX;
    }

    return rb;
}

size_t _z_send_tcp(const _z_sys_net_socket_t sock, const uint8_t *ptr, size_t len) {sock._send(sock._number, ptr, len);}

size_t _z_read_exact_tcp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
    // Initialize the W5100
    // wizchip_setup();

    // Copied from FreeRTOS+TCP
    size_t n = 0;
    uint8_t *pos = &ptr[0];

    do {
        size_t rb = _z_read_tcp(sock, pos, len - n);
        if ((rb == SIZE_MAX) || (rb == 0)) {
            n = rb;
            break;
        }

        n = n + rb;
        pos = _z_ptr_u8_offset(pos, n);
    } while (n != len);

    return n;
}
