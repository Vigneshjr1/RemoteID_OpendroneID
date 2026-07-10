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
    return mavutil.mavlink_connection(
        port,
        baud=baud,
        source_system=1,
        source_component=1,
        dialect='common'
    )


def send_basic_id(mav, serial_number="SIM-PIC32-TEST01"):
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
        height_reference=1,     # ODID_HEIGHT_REF_OVER_GROUND
        height=alt - 10.0,
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
        classification_type=0,      # ODID_CLASSIFICATION_TYPE_UNDECLARED
        operator_latitude=int(operator_lat * 1e7),
        operator_longitude=int(operator_lon * 1e7),
        area_count=1,
        area_radius=0,
        area_ceiling=-1000.0,
        area_floor=-1000.0,
        category_eu=0,
        class_eu=0,
        operator_altitude_geo=15.0,
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


def send_operator_id(mav, operator_id="OP-SIM-001"):
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
    """Simulate a circular flight path with large visible changes."""
    radius = 0.005  # ~550m radius - very visible on map
    speed_factor = 0.3  # radians per second - fast orbit
    angle = elapsed * speed_factor

    lat = base_lat + radius * math.cos(angle)
    lon = base_lon + radius * math.sin(angle)
    heading = (math.degrees(angle) + 90) % 360
    alt = 50.0 + 100.0 * math.sin(elapsed * 0.2)  # 50-150m swings
    speed = 5.0 + 15.0 * abs(math.sin(elapsed * 0.3))  # 5-20 m/s

    return lat, lon, alt, heading, speed


def main():
    parser = argparse.ArgumentParser(description='OpenDroneID MAVLink UART Simulator')
    parser.add_argument('--port', default='/dev/ttyACM0', help='Serial port (default: /dev/ttyACM0)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--rate', type=float, default=1.0, help='Message rate in Hz (default: 1.0)')
    parser.add_argument('--lat', type=float, default=33.3061, help='Base latitude (default: Chandler AZ)')
    parser.add_argument('--lon', type=float, default=-111.8413, help='Base longitude (default: Chandler AZ)')
    args = parser.parse_args()

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

    start_time = time.time()
    msg_count = 0

    try:
        while True:
            elapsed = time.time() - start_time
            lat, lon, alt, heading, speed = simulate_flight(args.lat, args.lon, elapsed)

            send_basic_id(mav)
            time.sleep(0.05)
            send_location(mav, lat, lon, alt, heading, speed)
            time.sleep(0.05)
            send_system(mav, args.lat, args.lon)
            time.sleep(0.05)
            send_self_id(mav)
            time.sleep(0.05)
            send_operator_id(mav)

            msg_count += 5
            print(f"\r  [{elapsed:6.1f}s] Sent {msg_count} msgs | "
                  f"Pos: {lat:.6f}, {lon:.6f} | Alt: {alt:.1f}m | "
                  f"Hdg: {heading:.0f} deg | Spd: {speed:.1f} m/s",
                  end='', flush=True)

            time.sleep(1.0 / args.rate)

    except KeyboardInterrupt:
        print(f"\n\nStopped. Sent {msg_count} messages in {time.time() - start_time:.1f}s")


if __name__ == '__main__':
    main()
