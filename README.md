<div align="center" markdown="1">

<img src=".github/meshtastic_logo.png" alt="Meshtastic Logo" width="80"/>
<h1>Meshtastic Firmware</h1>

![GitHub release downloads](https://img.shields.io/github/downloads/meshtastic/firmware/total)
[![CI](https://img.shields.io/github/actions/workflow/status/meshtastic/firmware/main_matrix.yml?branch=master&label=actions&logo=github&color=yellow)](https://github.com/meshtastic/firmware/actions/workflows/ci.yml)
[![CLA assistant](https://cla-assistant.io/readme/badge/meshtastic/firmware)](https://cla-assistant.io/meshtastic/firmware)
[![Fiscal Contributors](https://opencollective.com/meshtastic/tiers/badge.svg?label=Fiscal%20Contributors&color=deeppink)](https://opencollective.com/meshtastic/)
[![Vercel](https://img.shields.io/static/v1?label=Powered%20by&message=Vercel&style=flat&logo=vercel&color=000000)](https://vercel.com?utm_source=meshtastic&utm_campaign=oss)

<a href="https://trendshift.io/repositories/5524" target="_blank"><img src="https://trendshift.io/api/badge/repositories/5524" alt="meshtastic%2Ffirmware | Trendshift" style="width: 250px; height: 55px;" width="250" height="55"/></a>

</div>

</div>

<div align="center">
	<a href="https://meshtastic.org">Website</a>
	-
	<a href="https://meshtastic.org/docs/">Documentation</a>
</div>

---

> ## 🔐 This is a WireGuard-enabled fork of Meshtastic
>
> This fork adds **experimental WireGuard VPN support** to the Meshtastic firmware — an optional, streamlined module that lets an IP-capable node (ESP32 Wi‑Fi/Ethernet) bring up an encrypted WireGuard tunnel automatically once networking and NTP time are available.
>
> **Highlights**
>
> - Optional, opt-in module gated behind the `HAS_WIREGUARD_VPN` compile-time flag (disabled by default — production defaults stay blank).
> - Single-peer tunnel configured via `ModuleConfig.wireguard` over the admin API (address, server endpoint, keys, optional preshared key).
> - Tunnel is started/stopped automatically by the Wi‑Fi and Ethernet client code when the link and clock are ready.
> - Standard WireGuard `.conf` files map cleanly onto the device config.
>
> **Status & roadmap**
> The implementation has been streamlined with the goal of getting WireGuard into mainline Meshtastic. The current proposal is a recognized **"community support" track** (unofficial, clearly-labeled builds distributed via the Web Flasher) while the integration is rebased onto the actively-maintained `droscy/esp_wireguard` library. Full context and discussion:
>
> - 💬 [WireGuard: status, a viable library path, and a "community support" proposal](https://github.com/meshtastic/firmware/discussions/10716)
>
> **Where to look**
>
> - 📦 This fork: [TheWISPRer/Meshtastic (`master`)](https://github.com/TheWISPRer/Meshtastic/tree/master)
> - 📖 Developer guide: [`src/mesh/wireguard/WireGuard_ReadMe.md`](https://github.com/TheWISPRer/Meshtastic/blob/codex/wireguard-develop/src/mesh/wireguard/WireGuard_ReadMe.md)
> - 🧩 Implementation: [`src/mesh/wireguard/`](https://github.com/TheWISPRer/Meshtastic/tree/codex/wireguard-develop/src/mesh/wireguard)
> - 🖥️ Standalone configurator (until native clients expose the fields): [Meshtastic-Wireguard-Configurator](https://github.com/TheWISPRer/Meshtastic-Wireguard-Configurator)
>
> Active development happens on the **`Wireguard`** (tracks upstream `master`) and **`codex/wireguard-develop`** (tracks upstream `develop`) branches. Everything below is the upstream Meshtastic README, unchanged.

---

## Overview

This repository contains the official device firmware for Meshtastic, an open-source LoRa mesh networking project designed for long-range, low-power communication without relying on internet or cellular infrastructure. The firmware supports various hardware platforms, including ESP32, nRF52, RP2040/RP2350, and Linux-based devices.

Meshtastic enables text messaging, location sharing, and telemetry over a decentralized mesh network, making it ideal for outdoor adventures, emergency preparedness, and remote operations.

### Get Started

- 🔧 **[Building Instructions](https://meshtastic.org/docs/development/firmware/build)** – Learn how to compile the firmware from source.
- ⚡ **[Flashing Instructions](https://meshtastic.org/docs/getting-started/flashing-firmware/)** – Install or update the firmware on your device.

Join our community and help improve Meshtastic! 🚀

## Stats

![Alt](https://repobeats.axiom.co/api/embed/8025e56c482ec63541593cc5bd322c19d5c0bdcf.svg "Repobeats analytics image")
