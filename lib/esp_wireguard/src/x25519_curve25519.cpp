// Meshtastic local addition (not in upstream droscy/esp_wireguard).
//
// Provides the X25519 primitive that crypto.h declares, backed by the rweather
// Curve25519 implementation that Meshtastic already bundles via meshtastic/Crypto
// (the same primitive CryptoEngine uses for PKC). This replaces upstream's
// libsodium dependency, which would otherwise pull in the esphome/libsodium
// ESP-IDF component and break the Arduino-framework build.
//
// droscy clamps all private keys before use, so Curve25519::eval (the raw,
// non-clamping scalar multiplication) is the correct match. eval() returns false
// for small-order/zero results; we map that to a non-zero return so droscy's
// `wireguard_x25519(...) == 0` success checks behave as they did with libsodium.

#include <Curve25519.h>
#include <stdint.h>

extern "C" int meshtastic_wireguard_x25519(unsigned char *out, const unsigned char *scalar, const unsigned char *point)
{
    return Curve25519::eval(out, scalar, point) ? 0 : -1;
}
