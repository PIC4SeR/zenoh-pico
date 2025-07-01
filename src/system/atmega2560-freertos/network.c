#include "zenoh-pico/system/platform.h"
#include "zenoh-pico/transport/transport.h"
#include "zenoh-pico/utils/pointers.h"
#include "zenoh-pico/utils/result.h"

// FreeRTOS includes
#include "FreeRTOS.h"

// ioLibrary_Driver includes for W5100
#include "socket.h"
#include "wizchip_conf.h"

/**
 * @brief Converts an IPv4 address string (e.g., "192.168.1.100") to a uint8_t array.
 *
 * @param ip_str A pointer to the null-terminated string containing the IPv4 address.
 * @param ip_array A pointer to a uint8_t array of size 4 where the converted IP will be stored.
 * @return true if the conversion was successful and the IP address was valid.
 * @return false if the input string was NULL, ip_array was NULL, or the IP address format was invalid.
 */
bool convert_ip_string_to_uint8_array(const char *ip_str, uint8_t *ip_array) {
    if (ip_str == NULL) {
        return false;
    }

    // A valid IPv4 string "XXX.XXX.XXX.XXX" has a minimum length (e.g., "0.0.0.0" is 7 chars)
    // and a maximum length (e.g., "255.255.255.255" is 15 chars).
    // Plus the null terminator.
    size_t len = strlen(ip_str);
    if (len < 7 || len > 15) {
        return false;
    }

    unsigned int octet[4]; // Use unsigned int to read, then cast to uint8_t

    // Use sscanf to parse the string into four unsigned integers
    // The " %u.%u.%u.%u" format string expects four decimal numbers separated by dots.
    // The space before %u is important to consume any leading whitespace if present (though typically not for IP).
    // The %n specifier stores the number of characters successfully parsed so far.
    // This allows us to check if the entire string was consumed, ensuring no extra characters are present.
    int chars_parsed;
    int result = sscanf(ip_str, "%u.%u.%u.%u%n", &octet[0], &octet[1], &octet[2], &octet[3], &chars_parsed);

    if (result != 4) { return false; }

    if (chars_parsed != len) { return false; }

    for (int i = 0; i < 4; i++) {
        if (octet[i] > 255) {
            return false;
        }
        ip_array[i] = (uint8_t)octet[i];
    }

    return true;
}

void _z_socket_close(_z_sys_net_socket_t *sock) {close(sock->_number);}

/*------------------ TCP sockets ------------------*/
z_result_t _z_create_endpoint_tcp(_z_sys_net_endpoint_t *ep, const char *s_address, const char *s_port) {
    z_result_t ret = _Z_RES_OK;

    // Parse, check and add IP address
    if (!convert_ip_string_to_uint8_array(s_address, ep->_ip)) {
        ret = _Z_ERR_GENERIC;
        return ret;
    }

    // Parse, check and add the port
    uint32_t port = strtoul(s_port, NULL, 10);
    if ((port > (uint32_t)0) && (port <= (uint32_t)65355)) {  // Port numbers should range from 1 to 65355
        ep->_port = (uint16_t)port;
    } else {
        ret = _Z_ERR_GENERIC;
    }

    return ret;
}

void _z_free_endpoint_tcp(_z_sys_net_endpoint_t *ep) { 
    if (ep->_ip != NULL) {
        free(ep->_ip);
        ep->_ip = NULL;
    }
    ep->_port = 0;
}

z_result_t _z_open_tcp(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep, uint32_t tout) {
    z_result_t ret = _Z_RES_OK;

    // Create a new socket
    sock->_number = 0;
    sock->_port = 5000;

    uint8_t dest_ip[4] = {192, 168, 0, 100};  // Example IP address, replace with actual
    uint16_t dest_port = 7447;  // Example port, replace with actual

    if (socket(sock->_number, Sn_MR_TCP, sock->_port, 0) != sock->_number) {
        ret = _Z_ERR_GENERIC;
    }

    if (connect(0, dest_ip, dest_port) != SOCK_OK) {
        ret = _Z_ERR_GENERIC;
    }
    
    return ret;
}

z_result_t _z_listen_tcp(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep) {
    z_result_t ret = _Z_RES_OK;

    // Create a new socket
    sock->_number = 0;
    sock->_port = 5000;

    if (socket(sock->_number, Sn_MR_TCP, sock->_port, 0) != sock->_number) {
        ret = _Z_ERR_GENERIC;
    }

    if (connect(sock->_number, rep._ip, rep._port) != SOCK_OK) {
        ret = _Z_ERR_GENERIC;
    }

    ret = listen(sock->_number);
    if (ret != SOCK_OK) {
        ret = _Z_ERR_GENERIC;
    }

    return ret;
}

void _z_close_tcp(_z_sys_net_socket_t *sock) {close(sock->_number);}

size_t _z_read_tcp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
    
    int32_t rb = recv(0, ptr, len);
    if(rb != len) {
        // As in FreeRTOS+TCP SIZE_MAX
        rb =  SIZE_MAX;
    }

    return rb;
}

size_t _z_send_tcp(const _z_sys_net_socket_t sock, const uint8_t *ptr, size_t len) {
    int32_t ret = send(0, ptr, len);
    if (ret != len) {
        return SIZE_MAX;
    }
    return ret;
}

size_t _z_read_exact_tcp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
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
