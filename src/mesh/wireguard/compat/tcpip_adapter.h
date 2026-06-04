#pragma once

#include <stddef.h>

#include "esp_err.h"
#include "esp_netif.h"
#include "lwip/netif.h"

typedef enum {
    TCPIP_ADAPTER_IF_STA = 0,
    TCPIP_ADAPTER_IF_AP,
    TCPIP_ADAPTER_IF_ETH,
    TCPIP_ADAPTER_IF_PPP,
    TCPIP_ADAPTER_IF_MAX
} tcpip_adapter_if_t;

void handshake_destroy(struct wireguard_handshake *handshake);

static inline esp_err_t tcpip_adapter_get_netif(tcpip_adapter_if_t tcpip_if, struct netif **netif)
{
    static const char *netifIfKeys[TCPIP_ADAPTER_IF_MAX] = {"WIFI_STA_DEF", "WIFI_AP_DEF", "ETH_DEF", "PPP_DEF"};

    *netif = NULL;
    if (tcpip_if < TCPIP_ADAPTER_IF_MAX) {
        esp_netif_t *espNetif = esp_netif_get_handle_from_ifkey(netifIfKeys[tcpip_if]);
        if (espNetif == NULL) {
            return ESP_FAIL;
        }

        const int netifIndex = esp_netif_get_netif_impl_index(espNetif);
        if (netifIndex < 0) {
            return ESP_FAIL;
        }

        *netif = netif_get_by_index(netifIndex);
        if (*netif == NULL && tcpip_if == TCPIP_ADAPTER_IF_STA) {
            *netif = netif_default;
        }
    } else {
        *netif = netif_default;
    }

    return (*netif != NULL) ? ESP_OK : ESP_FAIL;
}
