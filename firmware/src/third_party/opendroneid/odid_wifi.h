/*
Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: Apache-2.0

Open Drone ID C Library - WiFi frame structures
*/

#ifndef _ODID_WIFI_H_
#define _ODID_WIFI_H_

#include "opendroneid.h"

struct __attribute__((__packed__)) ieee80211_mgmt {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    uint16_t seq_ctrl;
};

struct __attribute__((__packed__)) ieee80211_beacon {
    uint64_t timestamp;
    uint16_t beacon_interval;
    uint16_t capability;
};

struct __attribute__((__packed__)) ieee80211_ssid {
    uint8_t element_id;
    uint8_t length;
    uint8_t ssid[];
};

struct __attribute__((__packed__)) ieee80211_supported_rates {
    uint8_t element_id;
    uint8_t length;
    uint8_t supported_rates;
};

struct __attribute__((__packed__)) ieee80211_vendor_specific {
    uint8_t element_id;
    uint8_t length;
    uint8_t oui[3];
    uint8_t oui_type;
};

struct __attribute__((__packed__)) ODID_service_info {
    uint8_t message_counter;
    ODID_MessagePack_encoded odid_message_pack[];
};

#endif // _ODID_WIFI_H_
