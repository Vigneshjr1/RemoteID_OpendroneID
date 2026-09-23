#ifndef ODID_AUTH_H
#define ODID_AUTH_H

#include <stdint.h>
#include <stdbool.h>
#include "third_party/opendroneid/opendroneid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ODID_AUTH_KEY_SLOT      0
#define ODID_AUTH_SIG_LEN       64

bool ODID_Auth_Init(void);
bool ODID_Auth_Sign(ODID_UAS_Data *pUasData);
bool ODID_Auth_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* ODID_AUTH_H */
