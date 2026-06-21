#include "configuration.h"
#if HAS_WIREGUARD_VPN
#include "mesh/wireguard/WireGuardVPN.h"
#include "mesh/wireguard/WireGuardConfig.h"
#include "mesh/NodeDB.h"
#include "gps/RTC.h"
#include <cstring>
#include <WiFi.h>
#include "esp_wireguard.h"
#if HAS_WIFI
#include "mesh/wifi/WiFiAPClient.h"
#endif
#if HAS_ETHERNET
#include "mesh/eth/ethClient.h"
#endif

static bool running = false;
static wireguard_ctx_t wgCtx = ESP_WIREGUARD_CONTEXT_DEFAULT();
static wireguard_config_t wgConf = ESP_WIREGUARD_CONFIG_DEFAULT();
// droscy/esp_wireguard stores the const char* pointers from wgConf for the life
// of the tunnel, so every string it references must outlive startWireGuard().
// The key/address fields point at the static wireGuardConfig global; the resolved
// endpoint IP needs its own static buffer.
static char endpointIp[40];

bool startWireGuard()
{
    if (running) {
        return true;
    }

    const char *configError = nullptr;
    if (!wireGuardConfig.enabled) {
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_DISABLED);
        return false;
    }

    if (!isWireGuardConfigComplete(&configError)) {
        LOG_WARN("WireGuard not configured: %s", configError);
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_NOT_CONFIGURED, configError);
        return false;
    }

    if (getRTCQuality() < RTCQualityNTP) {
        LOG_INFO("WireGuard waiting for NTP");
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_WAITING_FOR_NTP);
        return false;
    }

    bool haveNetwork = false;
#if HAS_WIFI
    if (isWifiAvailable() && WiFi.isConnected()) {
        haveNetwork = true;
    }
#endif
#if HAS_ETHERNET
    if (isEthernetAvailable()) {
        haveNetwork = true;
    }
#endif
    if (!haveNetwork) {
        LOG_WARN("WireGuard requires an active network");
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_WAITING_FOR_NETWORK);
        return false;
    }

    IPAddress serverIp;
    bool resolved = false;
    setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_RESOLVING_SERVER);
#if HAS_ETHERNET
    if (isEthernetAvailable()) {
        resolved = Ethernet.hostByName(wireGuardConfig.serverAddr, serverIp);
    }
#endif
#if HAS_WIFI
    if (!resolved && isWifiAvailable() && WiFi.isConnected()) {
        resolved = WiFi.hostByName(wireGuardConfig.serverAddr, serverIp);
    }
#endif
    if (!resolved) {
        LOG_ERROR("WireGuard server %s unreachable", wireGuardConfig.serverAddr);
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_FAILED, "server unreachable");
        return false;
    }

    IPAddress localIp;
    if (!localIp.fromString(wireGuardConfig.address)) {
        LOG_ERROR("Invalid WireGuard client IP %s", wireGuardConfig.address);
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_FAILED, "invalid client address");
        return false;
    }

    String serverIpStr = serverIp.toString();
    strlcpy(endpointIp, serverIpStr.c_str(), sizeof(endpointIp));

    // Map the saved tunnel settings onto droscy/esp_wireguard's config. Unlike the
    // old ciniml wrapper, the preshared key is honoured here when present.
    wgConf = ESP_WIREGUARD_CONFIG_DEFAULT();
    wgConf.private_key = wireGuardConfig.privateKey;
    wgConf.public_key = wireGuardConfig.publicKey;
    wgConf.preshared_key = (wireGuardConfig.presharedKey[0] != '\0') ? wireGuardConfig.presharedKey : nullptr;
    wgConf.address = wireGuardConfig.address;
    wgConf.netmask = "255.255.255.255";
    wgConf.endpoint = endpointIp;
    wgConf.port = wireGuardConfig.serverPort;
    wgConf.persistent_keepalive = 25;

    wgCtx = ESP_WIREGUARD_CONTEXT_DEFAULT();
    if (esp_wireguard_init(&wgConf, &wgCtx) != ESP_OK) {
        LOG_ERROR("WireGuard init failed");
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_FAILED, "init failed");
        return false;
    }

    if (esp_wireguard_connect(&wgCtx) != ESP_OK) {
        LOG_ERROR("Unable to start WireGuard tunnel");
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_FAILED, "tunnel start failed");
        return false;
    }

    // Route outbound traffic through the tunnel, matching the previous behaviour.
    esp_wireguard_set_default(&wgCtx);

    LOG_INFO("WireGuard tunnel started to %s (%s):%u",
             wireGuardConfig.serverAddr,
             endpointIp,
             wireGuardConfig.serverPort);

    running = true;
    setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_RUNNING);
    return running;
}

void stopWireGuard()
{
    if (!running) {
        return;
    }
    esp_wireguard_disconnect(&wgCtx);
    LOG_INFO("WireGuard VPN stopped");
    running = false;
    if (wireGuardConfig.enabled) {
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_WAITING_FOR_NETWORK);
    } else {
        setWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig_Status_DISABLED);
    }
}

bool isWireGuardRunning()
{
    return running;
}

void populateWireGuardStatus(meshtastic_ModuleConfig_WireGuardConfig &config)
{
    config.status = getWireGuardStatus();
    strncpy(config.last_error, getWireGuardLastError(), sizeof(config.last_error));
    config.last_error[sizeof(config.last_error) - 1] = '\0';
}

#endif // HAS_WIREGUARD_VPN
