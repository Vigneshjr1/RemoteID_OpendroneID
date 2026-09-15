/*******************************************************************************
  WINC Wireless Driver Custom IE Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_custie.h

  Summary:
    WINC wireless driver custom IE interface.

  Description:
    Provides an interface to manage custom Information Elements for the WINC.
 *******************************************************************************/

/*
Copyright (C) 2024-25 Microchip Technology Inc. and its subsidiaries. All rights reserved.

Subject to your compliance with these terms, you may use this Microchip software and any derivatives
exclusively with Microchip products. You are responsible for complying with third party license terms
applicable to your use of third party software (including open source software) that may accompany this
Microchip software. SOFTWARE IS "AS IS." NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR
STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-
INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL
MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS,
DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER
CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
CLAIMS RELATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY
TO MICROCHIP FOR THIS SOFTWARE.
*/

#ifndef WDRV_WINC_CUSTIE_H
#define WDRV_WINC_CUSTIE_H

#ifndef WDRV_WINC_MOD_DISABLE_CUSTIE

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Custom IE Data Types
// *****************************************************************************
// *****************************************************************************

#define WDRV_WINC_CUSTIE_MAX_STORAGE_LEN    256U
#define WDRV_WINC_CUSTIE_MIN_STORAGE_LEN    4U
#define WDRV_WINC_CUSTIE_DATA_OFFSET        4U

// *****************************************************************************
/*  Custom IE Structure

  Summary:
    Defines the format of a custom IE.

  Description:
    Custom IEs consist of an ID, length and data.

  Remarks:
    This definition does not allocate any storage for data.
*/

typedef struct
{
    /* ID. */
    uint8_t id;

    /* Length of data. */
    uint8_t length;

    /* Data. */
    uint8_t data[];
} WDRV_WINC_CUST_IE;

// *****************************************************************************
/*  Custom IE Store Structure

  Summary:
    Defines the storage used for holding custom IEs.

  Description:
    Custom IEs are passed to the Soft-AP via the custom IE store which packages
    the IEs together.

  Remarks:
    None.
*/

typedef struct
{
    /* Maximum length of the IE store data. */
    uint16_t maxLength;

    /* Current length of data in the store. */
    uint16_t curLength;

    /* IE data in store. */
    uint8_t ieData[];
} WDRV_WINC_CUST_IE_STORE_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Custom IE Routines
// *****************************************************************************
// *****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

WDRV_WINC_CUST_IE_STORE_CONTEXT* WDRV_WINC_CustIEStoreCtxSetStorage
(
    uint8_t *const pStorage,
    uint16_t lenStorage
);

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxAddIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id,
    const uint8_t *const pData,
    uint8_t dataLength
);

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxRemoveIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id
);

#ifdef __cplusplus
}
#endif

#endif /* WDRV_WINC_MOD_DISABLE_CUSTIE */
#endif /* WDRV_WINC_CUSTIE_H */
