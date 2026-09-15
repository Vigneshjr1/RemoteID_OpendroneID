#ifndef ODID_WIFI_H
#define ODID_WIFI_H

#include <stdint.h>
#include "third_party/opendroneid/opendroneid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ODID_WIFI_BEACON_TIMER_PERIOD_MS  1000
#define ODID_WIFI_BEACON_BUF_SIZE         512

void ODID_WiFi_Init(void);
void ODID_WiFi_StartAdvertising(void);
void ODID_WiFi_UpdateBeacon(ODID_UAS_Data *pUasData);

#ifdef __cplusplus
}
#endif

#endif /* ODID_WIFI_H */
