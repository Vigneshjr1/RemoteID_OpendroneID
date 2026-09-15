#ifndef ODID_VSIE_H
#define ODID_VSIE_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/driver_common.h"
#include "third_party/opendroneid/opendroneid.h"

#ifdef __cplusplus
extern "C" {
#endif

void ODID_VSIE_Init(void);
bool ODID_VSIE_EnableBeaconFilter(DRV_HANDLE wincHandle);
bool ODID_VSIE_UpdatePayload(DRV_HANDLE wincHandle, ODID_UAS_Data *pUasData, uint8_t sendCounter);

#ifdef __cplusplus
}
#endif

#endif /* ODID_VSIE_H */
