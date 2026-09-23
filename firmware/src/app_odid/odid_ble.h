#ifndef ODID_BLE_H
#define ODID_BLE_H

#include <stdint.h>
#include <stdbool.h>
#include "third_party/opendroneid/opendroneid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ODID_ADV_HANDLE_LEGACY   0x00
#define ODID_ADV_HANDLE_CODED    0x01

#define ODID_LEGACY_PAYLOAD_LEN  31
#define ODID_LONGRANGE_PAYLOAD_MAX 250

typedef enum {
    ODID_LEGACY_PHASE_LOCATION = 0,
    ODID_LEGACY_PHASE_BASIC_ID,
    ODID_LEGACY_PHASE_SELF_ID,
    ODID_LEGACY_PHASE_SYSTEM,
    ODID_LEGACY_PHASE_OPERATOR_ID,
    ODID_LEGACY_PHASE_AUTH,
    ODID_LEGACY_PHASE_COUNT
} ODID_LegacyPhase_T;

/* Cycle all legacy message types once per second. */
#define ODID_LEGACY_TIMER_PERIOD_MS     (1000U / ODID_LEGACY_PHASE_COUNT)
#define ODID_LONGRANGE_TIMER_PERIOD_MS  1000U

void ODID_BLE_Init(void);
void ODID_BLE_StartAdvertising(void);
void ODID_BLE_UpdateLegacy(ODID_UAS_Data *pUasData);
void ODID_BLE_UpdateLongRange(ODID_UAS_Data *pUasData);

#ifdef __cplusplus
}
#endif

#endif /* ODID_BLE_H */
