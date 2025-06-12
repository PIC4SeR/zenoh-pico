#include "zenoh-pico/transport/unicast.h"
#include "zenoh-pico/system/platform.h"
#include "zenoh-pico/system/net.h"
#include <string.h>
#include <stdint.h>

/* You need to implement or link your W5100 SPI driver here */
#include "w5100_driver.h"

typedef struct {
    uint8_t socket; // e.g., socket 0
} w5100_transport_t;

int8_t z_net_open(z_transport_t *zt, const char *locator, void *arg) {
    static w5100_transport_t transport = { .socket = 0 };

    // Extract IP and port from `locator` (e.g., "tcp/192.168.1.100:7447")
    // For simplicity, hardcode for now
    uint8_t ip[4] = {192, 168, 1, 100};
    uint16_t port = 7447;

    if (!w5100_socket_connect(transport.socket, ip, port)) {
        return -1;
    }

    zt->context = (void*)&transport;
    return 0;
}

ssize_t z_net_read(z_transport_t *zt, uint8_t *buf, size_t len) {
    w5100_transport_t *t = (w5100_transport_t*)zt->context;
    return w5100_socket_recv(t->socket, buf, len);
}

ssize_t z_net_write(z_transport_t *zt, const uint8_t *buf, size_t len) {
    w5100_transport_t *t = (w5100_transport_t*)zt->context;
    return w5100_socket_send(t->socket, buf, len);
}

void z_net_close(z_transport_t *zt) {
    w5100_transport_t *t = (w5100_transport_t*)zt->context;
    w5100_socket_close(t->socket);
}