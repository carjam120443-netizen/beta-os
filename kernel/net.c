#include "net.h"

static int ready;

void net_init(void) {
    ready = 1;
}

int net_is_ready(void) {
    return ready;
}

int net_configure_loopback(void) {
    return ready ? 0 : -1;
}

uint32_t net_ipv4(const uint8_t octets[4]) {
    return ((uint32_t)octets[0] << 24) | ((uint32_t)octets[1] << 16) |
           ((uint32_t)octets[2] << 8) | (uint32_t)octets[3];
}
