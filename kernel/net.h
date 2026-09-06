#ifndef BETA_NET_H
#define BETA_NET_H

#include <stdint.h>

void net_init(void);
int net_is_ready(void);
int net_configure_loopback(void);
uint32_t net_ipv4(const uint8_t octets[4]);

#endif
