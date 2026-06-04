#pragma once

#include_next "lwip/netif.h"

#if defined(__cplusplus)
// Arduino 3.x ESP-NETIF callbacks inspect netif->state during netif_add(),
// before WireGuard-ESP32 has replaced its init-data pointer with device state.
typedef struct meshtastic_wireguard_netif_init_context {
    void *state;
    netif_init_fn init;
} meshtastic_wireguard_netif_init_context;

static meshtastic_wireguard_netif_init_context meshtasticWireGuardNetifInitContext;

static inline err_t meshtastic_wireguard_netif_init_adapter(struct netif *netif)
{
    if (!meshtasticWireGuardNetifInitContext.init) {
        return ERR_ARG;
    }
    netif->state = meshtasticWireGuardNetifInitContext.state;
    return meshtasticWireGuardNetifInitContext.init(netif);
}

static inline struct netif *meshtastic_wireguard_netif_add(struct netif *netif,
                                                          const ip4_addr_t *ipaddr,
                                                          const ip4_addr_t *netmask,
                                                          const ip4_addr_t *gw,
                                                          void *state,
                                                          netif_init_fn init,
                                                          netif_input_fn input)
{
    meshtasticWireGuardNetifInitContext.state = state;
    meshtasticWireGuardNetifInitContext.init = init;
    struct netif *result = netif_add(netif, ipaddr, netmask, gw, NULL, meshtastic_wireguard_netif_init_adapter, input);
    meshtasticWireGuardNetifInitContext.state = NULL;
    meshtasticWireGuardNetifInitContext.init = NULL;
    return result;
}

#define netif_add(netif, ipaddr, netmask, gw, state, init, input) \
    meshtastic_wireguard_netif_add((netif), (ipaddr), (netmask), (gw), (state), (init), (input))
#endif
