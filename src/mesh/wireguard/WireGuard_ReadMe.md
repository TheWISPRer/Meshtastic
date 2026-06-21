# WireGuard VPN Developer Guide

This document explains how the experimental WireGuard VPN support in the Meshtastic firmware is implemented and how developers can interact with it.

## Overview

The WireGuard feature is implemented as an optional firmware module for devices with IP networking. It keeps production defaults blank, stores user-provided tunnel settings in `ModuleConfig.wireguard`, and starts the VPN only after networking and NTP time are available.

Until official Meshtastic clients expose `ModuleConfig.wireguard`, a standalone GUI/CLI configurator is available at https://github.com/TheWISPRer/Meshtastic-Wireguard-Configurator.

## Enabling the Feature

WireGuard functionality is optional and controlled by the `HAS_WIREGUARD_VPN` compile-time flag. By default this flag is disabled in `src/configuration.h`.

For ESP32-family variants, use the shared `wireguard_esp32` PlatformIO section so the feature flag and WireGuard build options stay together:

1. Add `${wireguard_esp32.build_flags}` to the variant's `build_flags`.
2. Add `${wireguard_esp32.lib_deps}` to the variant's `lib_deps`.

The WireGuard implementation itself is the in-tree vendored library at `lib/esp_wireguard/` (see [Implementation](#implementation)); it is discovered automatically via the `#include` in `WireGuardVPN.cpp`, so `${wireguard_esp32.lib_deps}` is currently empty and the section mainly carries the `-DHAS_WIREGUARD_VPN=1` and `-DCONFIG_WIREGUARD_MAX_SRC_IPS` build flags.

Example `seeed_xiao_s3` configuration:

```ini
    [env:seeed-xiao-s3]
    extends = esp32s3_base
    board = seeed-xiao-s3
    board_check = true
    board_build.partitions = default_8MB.csv
    upload_protocol = esptool
    upload_speed = 921600
    lib_deps =
        ${esp32s3_base.lib_deps}
        ${wireguard_esp32.lib_deps}
    build_flags =
        ${esp32s3_base.build_flags}
        ${wireguard_esp32.build_flags}
        -D SEEED_XIAO_S3
        -I variants/esp32s3/seeed_xiao_s3
        -DBOARD_HAS_PSRAM
```

## Configuration Structure

The runtime configuration is defined in `src/mesh/wireguard/WireGuardConfig.h`:

```cpp
typedef struct WireGuardConfig {
    bool enabled;           ///< Whether the tunnel should start when networking and NTP are ready
    char address[32];       ///< Client IPv4 address (e.g. 10.0.0.2)
    char serverAddr[64];    ///< WireGuard server host
    uint16_t serverPort;    ///< WireGuard server port
    char privateKey[64];    ///< Client private key
    char publicKey[64];     ///< Server public key
    char presharedKey[64];  ///< Optional preshared key
} WireGuardConfig;
```

Default values for these fields can be set at compile time using the `WIREGUARD_DEFAULT_*` macros in the same header, but production firmware should leave them blank and disabled. A global instance `wireGuardConfig` is allocated in `WireGuardConfig.cpp` and is synchronized from `moduleConfig.wireguard` after loading, restoring, or receiving admin updates.

The protobuf definition `meshtastic_ModuleConfig_WireGuardConfig` (generated in `module_config.pb.h`) mirrors this structure so that the values can be updated over the admin API. It also reports transient runtime `status` and `last_error` fields in get responses.

## Device Configuration

Configure the device through a client that supports `ModuleConfig.wireguard` and the admin message type `AdminMessage_ModuleConfigType_WIREGUARD_CONFIG`. A standard single-peer WireGuard config contains the fields needed by the firmware:

```ini
[Interface]
Address = 10.0.0.2/32
PrivateKey = CLIENT_PRIVATE_KEY

[Peer]
PublicKey = SERVER_PUBLIC_KEY
PresharedKey = OPTIONAL_PRESHARED_KEY
Endpoint = wg.example.net:51820
AllowedIPs = 0.0.0.0/0
```

Client-side importers should map:

- `Interface.Address` -> `address`
- `Interface.PrivateKey` -> `private_key`
- `Peer.PublicKey` -> `public_key`
- `Peer.PresharedKey` -> `preshared_key`
- `Peer.Endpoint` -> `server_addr` and `server_port`

Importers should strip the subnet mask from `Address`, split `Endpoint` into server host and port, and reject multi-peer configs. The preshared key **is** honoured. `DNS` is ignored. `AllowedIPs` and `PersistentKeepalive` are not read from the config — the firmware always runs the tunnel as a **full tunnel** (`AllowedIPs = 0.0.0.0/0`) with a 25-second keepalive, matching the behaviour earlier firmware relied on. (Exposing `AllowedIPs` for split-tunnel setups is a possible future enhancement.)

The private and preshared key sentinel value `sekrit` preserves an existing saved key when updating other WireGuard fields through the admin handler. Clients should redact private and preshared keys from normal readback displays.

For users who need a temporary desktop client before native Meshtastic clients expose these fields, use the standalone configurator: https://github.com/TheWISPRer/Meshtastic-Wireguard-Configurator.

## WireGuard API

The public API is declared in `src/mesh/wireguard/WireGuardVPN.h`:

```cpp
bool startWireGuard();    // Start the VPN tunnel
void stopWireGuard();     // Stop the VPN service
bool isWireGuardRunning();
```

`startWireGuard()` attempts to create a tunnel only when WireGuard is enabled and all required fields are configured. It first checks that the device has valid NTP time and an active network (Wi‑Fi or Ethernet). On success the global VPN instance becomes active and `isWireGuardRunning()` returns `true`.

`stopWireGuard()` tears down the tunnel if it is running.

## Automatic Control

The WiFi and Ethernet client code automatically manages the VPN. When a network connection is established and the real-time clock is synced, `startWireGuard()` is called. When the connection drops, `stopWireGuard()` is invoked. This logic can be seen in `WiFiAPClient.cpp` and `ethClient.cpp`.

## Interacting from Other Modules

Other modules may start or stop the VPN by calling the API functions above. They can also examine or modify `wireGuardConfig` before starting the tunnel. For remote configuration, use the admin protobuf message `AdminMessage_ModuleConfigType_WIREGUARD_CONFIG`.

## Implementation

The tunnel is provided by a vendored copy of [`droscy/esp_wireguard`](https://github.com/droscy/esp_wireguard) (the maintained ESPHome WireGuard stack) at `lib/esp_wireguard/`. `WireGuardVPN.cpp` drives it through `esp_wireguard_init` → `esp_wireguard_connect` → `esp_wireguard_add_allowed_ip` → `esp_wireguard_set_default`.

Upstream targets ESPHome, so the vendored copy carries three small, clearly-commented local patches. When updating to a newer droscy release, drop in their `src/` and re-apply these:

1. **X25519 backend** (`src/crypto.h`, `src/x25519_curve25519.cpp`) — upstream uses libsodium for X25519, which drags in the `esphome/libsodium` ESP-IDF component and breaks the Arduino-framework build. We route X25519 through the rweather `Curve25519` already shipped by `meshtastic/Crypto` (the same primitive `CryptoEngine` uses), so no libsodium is pulled. droscy clamps its own keys, so the raw `Curve25519::eval` is correct.
2. **`netif->state` collision** (`src/esp_wireguard.c`, `esp_wireguard_netif_create`) — ESP-IDF maps a `netif` back to its `esp_netif` via `netif->state`, the same slot `wireguardif` uses for its device. lwIP `netif_add` fires esp_netif's ext-callback before our init runs, so the callback dereferences our data as an `esp_netif` and panics (boot loop). The patch adds the netif with `state=NULL` and restores it via a thin init adapter.
3. **Allowed-IPs / max source IPs** — droscy seeds the peer with only the device's own `/32` and caps `WIREGUARD_MAX_SRC_IPS` at 1. We add `0.0.0.0/0` after connect (full tunnel) and raise the cap via `-DCONFIG_WIREGUARD_MAX_SRC_IPS` in `platformio.ini` so the add succeeds. Without this the handshake completes but **no data flows** (cryptokey routing drops everything).

Preshared keys are fully supported; `set_default()` is safe because `wireguardif` pins its transport UDP to the physical interface, so the encrypted packets do not loop back through the tunnel.

Because this feature is experimental the implementation may evolve, but these entry points are expected to remain stable.
