#ifndef ODID_MAVLINK_H
#define ODID_MAVLINK_H

#include <stdint.h>
#include <stdbool.h>
#include "third_party/opendroneid/opendroneid.h"

#ifdef __cplusplus
extern "C" {
#endif

void ODID_MAVLink_Init(void);
void ODID_MAVLink_ProcessByte(uint8_t byte);
ODID_UAS_Data* ODID_MAVLink_GetUasData(void);
bool ODID_MAVLink_CopyUasData(ODID_UAS_Data *pSnapshot);
bool ODID_MAVLink_HasValidData(void);

#ifdef __cplusplus
}
#endif

#endif /* ODID_MAVLINK_H */
