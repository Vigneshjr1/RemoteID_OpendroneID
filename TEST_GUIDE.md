# OpenDroneID Remote ID - Test Guide

Test guide for verifying ASTM F3411 Remote ID broadcasting over BLE and WiFi on the PIC32CX-BW1 Curiosity Board.

---

## Table of Contents

1. [Hardware Requirements](#1-hardware-requirements)
2. [Software Prerequisites](#2-software-prerequisites)
3. [Building and Flashing](#3-building-and-flashing)
4. [Quick Test (Hardcoded Data)](#4-quick-test-hardcoded-data)
5. [Full Test (MAVLink Simulator)](#5-full-test-mavlink-simulator)
6. [Verifying BLE Beacons](#6-verifying-ble-beacons)
7. [Verifying WiFi Beacons](#7-verifying-wifi-beacons)
8. [Key Parameters Reference](#8-key-parameters-reference)
9. [Troubleshooting](#9-troubleshooting)

---

## 1. Hardware Requirements

| Item | Details |
|------|---------|
| **Board** | PIC32CX-BW1 Curiosity Board (EV60G68A) |
| **USB Cable** | Micro-USB for board power + UART console |
| **Receiver Device** | Android phone (BLE 5.0+ recommended) or laptop with BLE + WiFi |
| **Programmer** (optional) | On-board debugger via USB, or external MPLAB SNAP / ICD4 |

### Board Connections

- Connect the micro-USB cable to the **DEBUG USB** port on the Curiosity Board.
- This provides both power and UART console access (via onboard MCP2200 USB-UART bridge).
- The board enumerates as a serial device (Linux: typically `/dev/ttyACM0` or `/dev/ttyACM1`).

---

## 2. Software Prerequisites

### For Building Firmware (optional - prebuilt hex available)

- [MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide) v6.x or later
- [XC32 Compiler](https://www.microchip.com/xc32) v4.x or later
- PIC32CX-BW DFP (Device Family Pack) - install via MPLAB X Pack Manager

### For Flashing Prebuilt Hex

- [MPLAB IPE](https://www.microchip.com/mplab/mplab-integrated-programming-environment) (included with MPLAB X)

### For MAVLink Simulation

```bash
pip install pymavlink pyserial
```

### For Verifying Beacons

| Tool | Platform | Purpose |
|------|----------|---------|
| **OpenDroneID Android App** | Android | Decodes both BLE and WiFi Remote ID beacons |
| **nRF Connect** | Android / iOS | Raw BLE advertisement scanner |
| **Wireshark** | PC | WiFi VSIE packet capture and inspection |

OpenDroneID Android app: search "OpenDroneID" on Google Play, or build from [opendroneid-core-c](https://github.com/opendroneid/receiver-android).

---

## 3. Building and Flashing

### Option A: Flash the Prebuilt Hex (Fastest)

A production hex is included at:

```
hex/ext_adv.X.production.hex
```

1. Open **MPLAB IPE**
2. Select device: **PIC32CX1012BZ25048**
3. Connect the board via USB
4. Load `hex/ext_adv.X.production.hex`
5. Click **Program**

### Option B: Build from Source

1. Open MPLAB X IDE
2. File > Open Project > navigate to `firmware/ext_adv.X/`
3. Select the **default** configuration
4. Click **Clean and Build** (hammer icon)
5. Click **Make and Program Device** (green arrow icon)

### Compile-Time Flags

Edit `firmware/src/config/default/configuration.h` to toggle features:

| Flag | Default | Effect |
|------|---------|--------|
| `ODID_USE_TEST_DATA` | Defined | Populates hardcoded test beacon data at boot |
| `ODID_WIFI_DISABLE` | Not defined | Define to disable WiFi, keep BLE only |
| `ODID_WIFI_ONLY_TEST` | Not defined | Define to disable BLE, keep WiFi only |

---

## 4. Quick Test (Hardcoded Data)

With `ODID_USE_TEST_DATA` defined (the default), the board broadcasts Remote ID beacons immediately on power-up with no external input required.

### Hardcoded Test Data

| Field | Value |
|-------|-------|
| Serial Number | `MCHP00TEST1234AB` |
| UA Type | Helicopter / Multirotor |
| Status | Airborne |
| Latitude | 33.3025 (Chandler, AZ) |
| Longitude | -111.8514 |
| Altitude | 120.0 m barometric |
| Height | 20.0 m above takeoff |
| Speed | 5.5 m/s horizontal |
| Heading | 45.0 deg |
| Self ID | "PIC32 ODID Test Flight" |
| Operator ID | "OP-MCHP-TEST-01" |
| EU Category | Open, Class 1 |

### Steps

1. Flash the firmware (Section 3)
2. Power the board via USB
3. Wait ~3 seconds for BLE + WiFi initialization
4. Verify beacons using the tools in Sections 6 and 7

---

## 5. Full Test (MAVLink Simulator)

The included Python simulator sends MAVLink v2 OpenDroneID messages over UART, simulating a circular flight pattern.

### Identify the Serial Port

```bash
# Linux
ls /dev/ttyACM*

# Windows (Device Manager > Ports)
# Look for "MCP2200 USB Serial Port" (e.g., COM3)

# macOS
ls /dev/tty.usbmodem*
```

### Run the Simulator

```bash
cd tools/

# Default settings (Linux)
python3 odid_mavlink_sim.py --port /dev/ttyACM0 --baud 115200

# Windows
python3 odid_mavlink_sim.py --port COM3 --baud 115200

# Custom parameters
python3 odid_mavlink_sim.py \
    --port /dev/ttyACM0 \
    --baud 115200 \
    --rate 1.0 \
    --lat 12.9716 \
    --lon 77.5946 \
    --serial-number "MY_DRONE_SN_001"
```

### Simulator Options

| Option | Default | Description |
|--------|---------|-------------|
| `--port` | `/dev/ttyACM0` | Serial port |
| `--baud` | `115200` | Baud rate |
| `--rate` | `1.0` | Message rate in Hz |
| `--lat` | `33.3025` | Base latitude |
| `--lon` | `-111.8514` | Base longitude |
| `--serial-number` | `MCHP00TEST1234AB` | UAS serial number (max 20 chars) |

### Expected Output

```
OpenDroneID MAVLink Simulator
  Port: /dev/ttyACM0 @ 115200 baud
  Rate: 1.0 Hz
  Base position: 33.3025, -111.8514
  Press Ctrl+C to stop

  [  5.0s] Sent 13 msgs | Pos: 33.302998, -111.851351 | Alt: 129.8m | Hdg: 117 deg | Spd: 6.9 m/s
```

The simulator sends:
- **Location** messages at the configured rate (1 Hz default)
- **BasicID, System, SelfID, OperatorID** every 3 seconds

---

## 6. Verifying BLE Beacons

The firmware broadcasts on two BLE advertising sets simultaneously:

| Mode | PHY | Interval | TX Power | Range |
|------|-----|----------|----------|-------|
| **Legacy (BLE 4)** | LE 1M | 120-167 ms | 0 dBm | ~50 m |
| **Long Range (BLE 5)** | LE Coded (S8) | 750-1000 ms | +12 dBm | ~300+ m |

### Using OpenDroneID Android App

1. Install the [OpenDroneID receiver app](https://play.google.com/store/apps/details?id=org.opendroneid.android)
2. Grant Bluetooth and Location permissions
3. Open the app and tap **Bluetooth** scan
4. The board should appear with serial number `MCHP00TEST1234AB`
5. Tap the entry to see decoded fields: location, altitude, speed, operator info

**What to verify:**
- All 5 message types are present (BasicID, Location, SelfID, System, OperatorID)
- Location updates when using the MAVLink simulator (coordinates change over time)
- Both BLE4 and BLE5 entries may appear (depending on phone capabilities)

### Using nRF Connect

1. Open nRF Connect and start scanning
2. Filter by service UUID: **0xFFFA** (ASTM Remote ID)
3. Look for advertisement data with:
   - AD Type: `0x16` (Service Data - 16-bit UUID)
   - Service UUID: `0xFFFA` (shown as `FA FF` in little-endian)
   - App Code byte: `0x0D`
4. The Legacy (BLE4) advertisements cycle through 5 message types every 200 ms each:
   - Location > BasicID > SelfID > System > OperatorID
5. The Long Range (BLE5) advertisement contains a packed message with all types

**Raw payload structure (Legacy BLE4, 31 bytes):**
```
1E 16 FA FF 0D [counter] [25 bytes encoded ODID message]
```

**Raw payload structure (Long Range BLE5, up to 250 bytes):**
```
[len] 16 FA FF 0D [counter] [packed ODID message payload]
```

---

## 7. Verifying WiFi Beacons

The firmware starts a WiFi Access Point and injects ASTM F3411 Vendor-Specific Information Elements (VSIE) into beacon and probe response frames.

| Parameter | Value |
|-----------|-------|
| SSID | `UAS_ID_OPEN` |
| Channel | 6 |
| Security | Open (no password) |
| IP Address | 192.168.4.1 |
| Beacon Update Rate | 1 second |

### Using OpenDroneID Android App

1. Open the app
2. Enable **WiFi** scan mode (in addition to Bluetooth)
3. The board should appear as a WiFi-detected Remote ID device
4. Decoded fields should match the BLE data

### Using WiFi Scanner

1. On any device, scan for WiFi networks
2. Look for SSID: **`UAS_ID_OPEN`** on channel 6
3. Presence of this SSID confirms the WiFi AP is running

### Using Wireshark (Advanced)

1. Set your WiFi adapter to monitor mode on channel 6:
   ```bash
   sudo airmon-ng start wlan0
   sudo iwconfig wlan0mon channel 6
   ```
2. Capture with Wireshark, filter: `wlan.ssid == "UAS_ID_OPEN"`
3. In beacon frames, look for the Vendor-Specific IE:
   - **Tag Number:** `0xDD` (Vendor Specific)
   - **OUI:** `FA:0B:BC` (ASTM OpenDroneID)
   - **OUI Type:** `0x0D`
   - **Payload:** Counter byte + packed ODID message data

**VSIE structure:**
```
DD [length] FA 0B BC 0D [counter] [packed ODID payload...]
```

---

## 8. Key Parameters Reference

### UART

| Parameter | Value |
|-----------|-------|
| Peripheral | SERCOM1 (PPS mode) |
| Baud Rate | 115200 |
| Config | 8N1 (8 data, no parity, 1 stop) |
| TX Pin | PA5 (SERCOM1 PAD0) |
| RX Pin | PA6 (SERCOM1 PAD1) |
| RX Buffer | 128 bytes |

### BLE

| Parameter | Legacy (BLE4) | Long Range (BLE5) |
|-----------|---------------|-------------------|
| ADV Handle | 0 | 1 |
| PHY | LE 1M | LE Coded (S8) |
| Interval | 120-167 ms | 750-1000 ms |
| TX Power | 0 dBm | +12 dBm |
| Update Period | 200 ms (cycles 5 types) | 1000 ms (all packed) |
| Max Payload | 31 bytes | 250 bytes |
| Service UUID | 0xFFFA | 0xFFFA |

### WiFi

| Parameter | Value |
|-----------|-------|
| SSID | `UAS_ID_OPEN` |
| Channel | 6 |
| Auth | Open |
| VSIE OUI | `FA:0B:BC` |
| Update Period | 1000 ms |
| Reg Domain | GEN |

---

## 9. Troubleshooting

### Serial port not found

- Verify the USB cable is connected to the **DEBUG USB** port (not the target USB).
- Check `dmesg | tail` (Linux) for USB enumeration.
- The MCP2200 bridge may enumerate as `/dev/ttyACM0` or `/dev/ttyACM1`. Try both.
- On Windows, check Device Manager > Ports for the correct COM port.

### Simulator connects but no beacons update

- Ensure baud rate matches: **115200**.
- The board UART is on SERCOM1 with PPS pin muxing. If using a custom board, verify PA5 (TX) and PA6 (RX) are connected.
- Check that the simulator is sending (message counter increments on screen).

### BLE beacons not visible

- Ensure your phone supports BLE 5.0 for Long Range advertisements.
- Legacy BLE4 advertisements should be visible on any BLE 4.0+ device.
- Move closer — Legacy TX power is 0 dBm (~50 m range).
- In nRF Connect, disable filters or filter specifically for UUID `0xFFFA`.
- Allow ~3 seconds after power-up for BLE stack initialization.

### WiFi AP not visible

- The WINCS02 module takes a few seconds to initialize after boot.
- Ensure `ODID_WIFI_DISABLE` is **not** defined in `configuration.h`.
- Scan for `UAS_ID_OPEN` on channel 6 specifically if your scanner allows channel selection.
- If WiFi was recently added, ensure the WINCS02 module firmware is up to date.

### Both BLE and WiFi not working

- Power-cycle the board (unplug and replug USB).
- Re-flash the firmware using MPLAB IPE.
- Check that the correct hex is flashed: `hex/ext_adv.X.production.hex`.

### MAVLink simulator errors

- `ModuleNotFoundError: pymavlink` — Run `pip install pymavlink pyserial`.
- `Permission denied on /dev/ttyACM0` — Add your user to the dialout group:
  ```bash
  sudo usermod -a -G dialout $USER
  ```
  Then log out and back in.
- On Windows, ensure no other application (like a terminal emulator) has the COM port open.

---

## Quick Verification Checklist

- [ ] Board powered via DEBUG USB
- [ ] **BLE Legacy**: Visible in nRF Connect / OpenDroneID app (UUID 0xFFFA)
- [ ] **BLE Long Range**: Visible on BLE 5.0 device (Coded PHY, +12 dBm)
- [ ] **WiFi AP**: SSID `UAS_ID_OPEN` visible in WiFi scan
- [ ] **WiFi VSIE**: Beacon contains OUI `FA:0B:BC` (Wireshark)
- [ ] **MAVLink input**: Simulator running, location updates reflected in beacons
- [ ] **All 5 message types decoded**: BasicID, Location, SelfID, System, OperatorID
