#include <Arduino.h>
#include <Ethernet.h>
#include <HardwareSerial.h>

extern "C" {


#include <Arduino_FreeRTOS.h>

#include "zenoh-pico/protocol/codec/serial.h"
#include "zenoh-pico/system/common/serial.h"
#include "zenoh-pico/system/link/serial.h"
#include "zenoh-pico/system/platform.h"
#include "zenoh-pico/transport/transport.h"
#include "zenoh-pico/utils/logging.h"
#include "zenoh-pico/utils/pointers.h"

/**
 * @brief Converts an IPv4 address string (e.g., "192.168.1.100") to a uint8_t array.
 *
 * @param ip_str A pointer to the null-terminated string containing the IPv4 address.
 * @param ip_array A pointer to a uint8_t array of size 4 where the converted IP will be stored.
 * @return true if the conversion was successful and the IP address was valid.
 * @return false if the input string was NULL, ip_array was NULL, or the IP address format was invalid.
 */
bool convert_ip_string_to_uint8_array(const char *ip_str, uint8_t *ip_array) {
    if (ip_str == NULL || ip_array == NULL) {
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

z_result_t _z_socket_set_non_blocking(const _z_sys_net_socket_t *sock) {
    // There should not be anything blocking in the EthernetClient API
    return _Z_RES_OK;
}

void _z_socket_close(_z_sys_net_socket_t *sock) {
#if Z_FEATURE_LINK_TCP == 1
    sock->_client->stop();
#endif
#if Z_FEATURE_LINK_SERIAL == 1
    sock->_serial->end();
    delete sock->_serial;
#endif
}

#if Z_FEATURE_LINK_TCP == 1
/*------------------ TCP sockets ------------------*/
z_result_t _z_create_endpoint_tcp(_z_sys_net_endpoint_t *ep, const char *s_address, const char *s_port) {
    z_result_t ret = _Z_RES_OK;

    // Allocate memory for IP address
    ep->_ip = (uint8_t*)malloc(4 * sizeof(uint8_t));
    if (ep->_ip == NULL) {
        return _Z_ERR_GENERIC;
    }

    // Parse, check and add IP address
    if (!convert_ip_string_to_uint8_array(s_address, ep->_ip)) {
        free(ep->_ip);
        ep->_ip = NULL;
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
    ep->_port = -1;
}

z_result_t _z_open_tcp(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep, uint32_t tout) {
    z_result_t ret = _Z_RES_OK;

    sock->_client = new EthernetClient();
    sock->_client->setConnectionTimeout(tout);

    if (!sock->_client->connect(rep._ip, rep._port)) {
        ret = _Z_ERR_GENERIC;
    }
    
    return ret;
}

z_result_t _z_listen_tcp(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep) {
    z_result_t ret = _Z_RES_OK;

    sock->_client = new EthernetClient();

    if (!sock->_client->connect(rep._ip, rep._port)) {
        ret = _Z_ERR_GENERIC;
    }

    // TODO(giafranchini): listen is not implemented for EthernetClient
    // ret = listen(sock->_number);
    // if (ret != SOCK_OK) {
    //     ret = _Z_ERR_GENERIC;
    // }

    return ret;
}

void _z_close_tcp(_z_sys_net_socket_t *sock) {sock->_client->stop();}

size_t _z_read_tcp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
    int32_t rb = sock._client->read(ptr, len);
    if(rb != len) {
        // As in FreeRTOS+TCP SIZE_MAX
        rb =  SIZE_MAX;
    }

    return rb;
}

size_t _z_send_tcp(const _z_sys_net_socket_t sock, const uint8_t *ptr, size_t len) {
    int32_t ret = sock._client->write(ptr, len);
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
#endif

#if Z_FEATURE_LINK_SERIAL == 1
/*------------------ Serial sockets ------------------*/
z_result_t _z_open_serial_from_pins(_z_sys_net_socket_t *sock, uint32_t txpin, uint32_t rxpin, uint32_t baudrate) {
    z_result_t ret = _Z_RES_OK;
    (void)(sock);
    (void)(txpin);
    (void)(rxpin);
    (void)(baudrate);

    // Not implemented, Arduino Serial internally initializes the Serial ports based on the pins
    ret = _Z_ERR_GENERIC;

    return ret;
}

z_result_t _z_open_serial_from_dev(_z_sys_net_socket_t *sock, char *dev, uint32_t baudrate) {
    if (strcmp(dev, "UART1") == 0) {
        sock->_serial = &Serial1; // Use Serial1 for UART_1
    } else if (strcmp(dev, "UART2") == 0) {
        sock->_serial = &Serial2; // Use Serial2 for UART_2
    } else if (strcmp(dev, "UART3") == 0) {
        sock->_serial = &Serial3; // Use Serial3 for UART_3
    } else {
        return _Z_ERR_GENERIC;
    }

    if (sock->_serial != NULL) {
        sock->_serial->begin(baudrate);
        sock->_serial->flush();
    } else {
        return _Z_ERR_GENERIC;
    }

    return _z_connect_serial(*sock);
}

z_result_t _z_listen_serial_from_pins(_z_sys_net_socket_t *sock, uint32_t txpin, uint32_t rxpin, uint32_t baudrate) {
    z_result_t ret = _Z_RES_OK;
    (void)(sock);
    (void)(txpin);
    (void)(rxpin);
    (void)(baudrate);

    // Not implemented
    ret = _Z_ERR_GENERIC;

    return ret;
}

z_result_t _z_listen_serial_from_dev(_z_sys_net_socket_t *sock, char *dev, uint32_t baudrate) {
    z_result_t ret = _Z_RES_OK;
    (void)(sock);
    (void)(dev);
    (void)(baudrate);

    // Not implemented
    ret = _Z_ERR_GENERIC;

    return ret;
}

void _z_close_serial(_z_sys_net_socket_t *sock) {
    sock->_serial->end();
    delete sock->_serial;
}

size_t _z_read_serial_internal(const _z_sys_net_socket_t sock, uint8_t *header, uint8_t *ptr, size_t len) {
    uint8_t *raw_buf = (uint8_t *)z_malloc(_Z_SERIAL_MAX_COBS_BUF_SIZE);
    size_t rb = 0;
    for (size_t i = 0; i < _Z_SERIAL_MAX_COBS_BUF_SIZE; i++) {
        while (sock._serial->available() < 1) {
            z_sleep_ms(1);  // FIXME: Yield by sleeping.
        }
        raw_buf[i] = sock._serial->read();
        rb = rb + (size_t)1;
        if (raw_buf[i] == (uint8_t)0x00) {
            break;
        }
    }

    uint8_t *tmp_buf = (uint8_t *)z_malloc(_Z_SERIAL_MFS_SIZE);
    size_t ret = _z_serial_msg_deserialize(raw_buf, rb, ptr, len, header, tmp_buf, _Z_SERIAL_MFS_SIZE);

    z_free(raw_buf);
    z_free(tmp_buf);

    return ret;
}

size_t _z_send_serial_internal(const _z_sys_net_socket_t sock, uint8_t header, const uint8_t *ptr, size_t len) {
    uint8_t *tmp_buf = (uint8_t *)z_malloc(_Z_SERIAL_MFS_SIZE);
    uint8_t *raw_buf = (uint8_t *)z_malloc(_Z_SERIAL_MAX_COBS_BUF_SIZE);
    size_t ret =
        _z_serial_msg_serialize(raw_buf, _Z_SERIAL_MAX_COBS_BUF_SIZE, ptr, len, header, tmp_buf, _Z_SERIAL_MFS_SIZE);

    if (ret == SIZE_MAX) {
        return ret;
    }

    size_t wb = sock._serial->write(raw_buf, ret);
    if (wb != (size_t)ret) {
        ret = SIZE_MAX;
    }

    z_free(raw_buf);
    z_free(tmp_buf);

    return len;
}
#endif

}  // extern "C"
