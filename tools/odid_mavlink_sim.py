#!/usr/bin/env python3
"""
OpenDroneID MAVLink Simulator

Sends MAVLink v2 OpenDroneID messages over UART to the PIC32CX-BW1 board
for testing the full pipeline: UART RX -> MAVLink parse -> ODID encode -> BLE advertise.

Usage:
    python3 odid_mavlink_sim.py --port /dev/ttyACM0 --baud 115200

Requirements:
    pip install pymavlink pyserial
"""

import argparse
import math
import time
import sys

from pymavlink import mavutil
from pymavlink.dialects.v20 import common as mavlink2


def create_connection(port, baud):
    """Create MAVLink serial connection."""
    connection = mavutil.mavlink_connection(
        port,
        baud=baud,
        source_system=1,
        source_component=1,
        dialect='common'
    )
    connection.port.dtr = False
    connection.port.write_timeout = 1.0
    return connection


def drain_board_output(connection):
    """Read firmware diagnostics so the USB receive endpoint cannot back up."""
    waiting = connection.port.in_waiting
    if waiting <= 0:
        return ""
    return connection.port.read(waiting).decode("ascii", errors="replace").strip()


def send_basic_id(mav, serial_number="MCHP00TEST1234AB"):
    """Send OPEN_DRONE_ID_BASIC_ID message."""
    uas_id = list(serial_number.encode('ascii')[:20].ljust(20, b'\x00'))

    msg = mavlink2.MAVLink_open_drone_id_basic_id_message(
        target_system=0,
        target_component=0,
        id_or_mac=[0] * 20,
        id_type=1,          # ODID_ID_TYPE_SERIAL_NUMBER
        ua_type=2,          # ODID_UA_TYPE_HELICOPTER_OR_MULTIROTOR
        uas_id=uas_id
    )
    mav.mav.send(msg)


def send_location(mav, lat, lon, alt, heading, speed):
    """Send OPEN_DRONE_ID_LOCATION message."""
    msg = mavlink2.MAVLink_open_drone_id_location_message(
        target_system=0,
        target_component=0,
        id_or_mac=[0] * 20,
        status=2,               # ODID_STATUS_AIRBORNE
        direction=int(heading * 100),
        speed_horizontal=int(speed * 100),
        speed_vertical=0,
        latitude=int(lat * 1e7),
        longitude=int(lon * 1e7),
        altitude_barometric=alt,
        altitude_geodetic=alt,
        height_reference=0,     # ODID_HEIGHT_REF_OVER_TAKEOFF
        height=alt - 100.0,
        horizontal_accuracy=10, # ODID_HOR_ACC_10M
        vertical_accuracy=4,    # ODID_VER_ACC_25M
        barometer_accuracy=4,
        speed_accuracy=3,       # ODID_SPEED_ACC_3MS
        timestamp_accuracy=2,
        timestamp=time.time() % 3600
    )
    mav.mav.send(msg)


def send_system(mav, operator_lat, operator_lon):
    """Send OPEN_DRONE_ID_SYSTEM message."""
    msg = mavlink2.MAVLink_open_drone_id_system_message(
        target_system=0,
        target_component=0,
        id_or_mac=[0] * 20,
        operator_location_type=0,   # ODID_OPERATOR_LOCATION_TYPE_TAKEOFF
        classification_type=1,      # ODID_CLASSIFICATION_TYPE_EU
        operator_latitude=int(operator_lat * 1e7),
        operator_longitude=int(operator_lon * 1e7),
        area_count=1,
        area_radius=0,
        area_ceiling=150.0,
        area_floor=0.0,
        category_eu=1,              # ODID_CATEGORY_EU_OPEN
        class_eu=2,                 # ODID_CLASS_EU_CLASS_1
        operator_altitude_geo=100.0,
        timestamp=int(time.time())
    )
    mav.mav.send(msg)


def send_self_id(mav, description="PIC32CX-BW1 ODID Sim"):
    """Send OPEN_DRONE_ID_SELF_ID message."""
    desc = description.encode('ascii')[:23].ljust(23, b'\x00')

    msg = mavlink2.MAVLink_open_drone_id_self_id_message(
        target_system=0,
        target_component=0,
        id_or_mac=[0] * 20,
        description_type=0,     # ODID_DESC_TYPE_TEXT
        description=desc
    )
    mav.mav.send(msg)


def send_operator_id(mav, operator_id="OP-MCHP-TEST-01"):
    """Send OPEN_DRONE_ID_OPERATOR_ID message."""
    op_id = operator_id.encode('ascii')[:20].ljust(20, b'\x00')

    msg = mavlink2.MAVLink_open_drone_id_operator_id_message(
        target_system=0,
        target_component=0,
        id_or_mac=[0] * 20,
        operator_id_type=0,     # ODID_OPERATOR_ID_TYPE_CAA
        operator_id=op_id
    )
    mav.mav.send(msg)


def simulate_flight(base_lat, base_lon, elapsed):
    """Simulate a plausible circular flight around the known-good fixture."""
    radius = 0.0005  # Approximately 55m radius.
    speed_factor = 0.1
    angle = elapsed * speed_factor

    lat = base_lat + radius * math.cos(angle)
    lon = base_lon + radius * math.sin(angle)
    heading = (math.degrees(angle) + 90) % 360
    alt = 120.0 + 20.0 * math.sin(elapsed * 0.1)
    speed = 5.0 + 3.0 * abs(math.sin(elapsed * 0.2))

    return lat, lon, alt, heading, speed


def main():
    parser = argparse.ArgumentParser(description='OpenDroneID MAVLink UART Simulator')
    parser.add_argument('--port', default='/dev/ttyACM0', help='Serial port (default: /dev/ttyACM0)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--rate', type=float, default=1.0, help='Message rate in Hz (default: 1.0)')
    parser.add_argument('--lat', type=float, default=33.3025, help='Base latitude (default: test fixture)')
    parser.add_argument('--lon', type=float, default=-111.8514, help='Base longitude (default: test fixture)')
    parser.add_argument('--serial-number', default='MCHP00TEST1234AB',
                        help='20-character maximum UAS serial number')
    args = parser.parse_args()

    if args.rate <= 0:
        parser.error('--rate must be greater than zero')

    print(f"OpenDroneID MAVLink Simulator")
    print(f"  Port: {args.port} @ {args.baud} baud")
    print(f"  Rate: {args.rate} Hz")
    print(f"  Base position: {args.lat}, {args.lon}")
    print(f"  Press Ctrl+C to stop\n")

    try:
        mav = create_connection(args.port, args.baud)
    except Exception as e:
        print(f"ERROR: Could not open {args.port}: {e}")
        sys.exit(1)

    start_time = time.monotonic()
    next_cycle = start_time
    msg_count = 0
    next_static_update = start_time

    try:
        while True:
            elapsed = time.monotonic() - start_time
            lat, lon, alt, heading, speed = simulate_flight(args.lat, args.lon, elapsed)
            board_output = drain_board_output(mav)

            if board_output:
                print(f"\n  Board: {board_output}")

            # Location is the only RID message that requires a 1 Hz update.
            # Refresh the static/slow-changing messages every three seconds to
            # avoid unnecessarily flooding the board UART and application task.
            send_location(mav, lat, lon, alt, heading, speed)
            msg_count += 1

            if time.monotonic() >= next_static_update:
                time.sleep(0.02)
                send_basic_id(mav, args.serial_number)
                time.sleep(0.02)
                send_system(mav, args.lat, args.lon)
                time.sleep(0.02)
                send_self_id(mav)
                time.sleep(0.02)
                send_operator_id(mav)
                msg_count += 4
                next_static_update += 3.0
                if next_static_update <= time.monotonic():
                    next_static_update = time.monotonic() + 3.0

            mav.port.flush()

            print(f"\r  [{elapsed:6.1f}s] Sent {msg_count} msgs | "
                  f"Pos: {lat:.6f}, {lon:.6f} | Alt: {alt:.1f}m | "
                  f"Hdg: {heading:.0f} deg | Spd: {speed:.1f} m/s",
                  end='', flush=True)

            # Account for the inter-message sleeps so --rate is the batch rate.
            next_cycle += 1.0 / args.rate
            remaining = next_cycle - time.monotonic()
            if remaining > 0:
                time.sleep(remaining)
            else:
                next_cycle = time.monotonic()

    except KeyboardInterrupt:
        print(f"\n\nStopped. Sent {msg_count} messages in {time.monotonic() - start_time:.1f}s")


if __name__ == '__main__':
    main()
