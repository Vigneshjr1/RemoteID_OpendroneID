/*
Copyright (C) 2020 Simon Wunderlich, Marek Sobe
Copyright (C) 2020 Doodle Labs

SPDX-License-Identifier: Apache-2.0

Open Drone ID C Library - WiFi Beacon frame builder
Stripped to beacon-only for PIC32CX-BW1 bare-metal (no NAN, no clock_gettime)
*/

#include <string.h>
#include <stddef.h>

#include "opendroneid.h"
#include "odid_wifi.h"

#define ODID_WIFI_EINVAL  (-1)
#define ODID_WIFI_ENOMEM  (-2)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define cpu_to_le16(x)  (x)
#define cpu_to_le64(x)  (x)
#else
#define cpu_to_le16(x)  (bswap_16(x))
#define cpu_to_le64(x)  (bswap_64(x))
#endif

#define IEEE80211_FCTL_FTYPE          0x000c
#define IEEE80211_FCTL_STYPE          0x00f0

#define IEEE80211_FTYPE_MGMT          0x0000
#define IEEE80211_STYPE_BEACON        0x0080

#define IEEE80211_CAPINFO_SHORT_PREAMBLE    0x0020
#define IEEE80211_CAPINFO_SHORT_SLOTTIME    0x0400

#define IEEE80211_ELEMID_SSID         0x00
#define IEEE80211_ELEMID_RATES        0x01
#define IEEE80211_ELEMID_VENDOR       0xDD

static int buf_fill_ieee80211_mgmt(uint8_t *buf, size_t *len, size_t buf_size,
                                   const uint16_t subtype,
                                   const uint8_t *dst_addr,
                                   const uint8_t *src_addr,
                                   const uint8_t *bssid)
{
    if (*len + sizeof(struct ieee80211_mgmt) > buf_size)
        return ODID_WIFI_ENOMEM;

    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)(buf + *len);
    mgmt->frame_control = (uint16_t) cpu_to_le16(IEEE80211_FTYPE_MGMT | subtype);
    mgmt->duration = cpu_to_le16(0x0000);
    memcpy(mgmt->da, dst_addr, sizeof(mgmt->da));
    memcpy(mgmt->sa, src_addr, sizeof(mgmt->sa));
    memcpy(mgmt->bssid, bssid, sizeof(mgmt->bssid));
    mgmt->seq_ctrl = cpu_to_le16(0x0000);
    *len += sizeof(*mgmt);

    return 0;
}

static int buf_fill_ieee80211_beacon(uint8_t *buf, size_t *len, size_t buf_size, uint16_t interval_tu)
{
    if (*len + sizeof(struct ieee80211_beacon) > buf_size)
        return ODID_WIFI_ENOMEM;

    struct ieee80211_beacon *beacon = (struct ieee80211_beacon *)(buf + *len);
    beacon->timestamp = cpu_to_le64(0);
    beacon->beacon_interval = cpu_to_le16(interval_tu);
    beacon->capability = cpu_to_le16(IEEE80211_CAPINFO_SHORT_SLOTTIME | IEEE80211_CAPINFO_SHORT_PREAMBLE);
    *len += sizeof(*beacon);

    return 0;
}

int odid_wifi_build_message_pack_beacon_frame(ODID_UAS_Data *UAS_Data, char *mac,
                                              const char *SSID, size_t SSID_len,
                                              uint16_t interval_tu, uint8_t send_counter,
                                              uint8_t *buf, size_t buf_size)
{
    uint8_t target_addr[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t asd_stan_oui[3] = { 0xFA, 0x0B, 0xBC };

    struct ieee80211_ssid *ssid_s;
    struct ieee80211_supported_rates *rates;
    struct ieee80211_vendor_specific *vendor;
    struct ODID_service_info *si;

    int ret;
    size_t len = 0;

    ret = buf_fill_ieee80211_mgmt(buf, &len, buf_size, IEEE80211_STYPE_BEACON, target_addr, (uint8_t *)mac, (uint8_t *)mac);
    if (ret < 0)
        return ret;

    ret = buf_fill_ieee80211_beacon(buf, &len, buf_size, interval_tu);
    if (ret < 0)
        return ret;

    if (len + sizeof(*ssid_s) > buf_size)
        return ODID_WIFI_ENOMEM;

    ssid_s = (struct ieee80211_ssid *)(buf + len);
    if (!SSID || (SSID_len == 0) || (SSID_len > 32))
        return ODID_WIFI_EINVAL;
    ssid_s->element_id = IEEE80211_ELEMID_SSID;
    ssid_s->length = (uint8_t) SSID_len;
    memcpy(ssid_s->ssid, SSID, ssid_s->length);
    len += sizeof(*ssid_s) + SSID_len;

    if (len + sizeof(*rates) > buf_size)
        return ODID_WIFI_ENOMEM;

    rates = (struct ieee80211_supported_rates *)(buf + len);
    rates->element_id = IEEE80211_ELEMID_RATES;
    rates->length = 1;
    rates->supported_rates = 0x8C; // 6 Mbps
    len += sizeof(*rates);

    if (len + sizeof(*vendor) > buf_size)
        return ODID_WIFI_ENOMEM;

    vendor = (struct ieee80211_vendor_specific *)(buf + len);
    vendor->element_id = IEEE80211_ELEMID_VENDOR;
    vendor->length = 0x00;
    memcpy(vendor->oui, asd_stan_oui, sizeof(vendor->oui));
    vendor->oui_type = 0x0D;
    len += sizeof(*vendor);

    if (len + sizeof(*si) > buf_size)
        return ODID_WIFI_ENOMEM;

    si = (struct ODID_service_info *)(buf + len);
    memset(si, 0, sizeof(*si));
    si->message_counter = send_counter;
    len += sizeof(*si);

    ret = odid_message_build_pack(UAS_Data, buf + len, buf_size - len);
    if (ret < 0)
        return ret;
    len += ret;

    vendor->length = sizeof(vendor->oui) + sizeof(vendor->oui_type) + sizeof(*si) + ret;

    return (int) len;
}
