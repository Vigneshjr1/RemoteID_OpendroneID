/*******************************************************************************
  WINC Wireless Driver IE Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_ie.h

  Summary:
    WINC wireless driver IE interface.

  Description:
    Provides an interface to manage Information Elements for the WINC.
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

#ifndef WDRV_WINC_IE_H
#define WDRV_WINC_IE_H

#ifndef WDRV_WINC_MOD_DISABLE_IE

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#include "wdrv_winc_common.h"

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver IE Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/*  Vendor IE Frame Filter Mask

  Summary:
    Defines frame filter mask values for TX of vendor IE tags.

  Description:
    Filter mask enables the application to choose the management frame types
    into which custom IEs will be added.

  Remarks:
    Bit position value 1 enables and 0 disables TX of vendor IE data.
*/

typedef enum
{
    /* Custom IE data will be added to beacon frames. */
    WDRV_WINC_IE_FRAME_TYPE_BEACON    = 0x01,

    /* Custom IE data will be added to probe response frames. */
    WDRV_WINC_IE_FRAME_TYPE_PROBE_RSP = 0x04
} WDRV_WINC_IE_FRAME_TYPE_MASK;

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver IE Routines
// *****************************************************************************
// *****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

//*******************************************************************************
/*
  Function:
    void WDRV_WINC_IEProcessAEC
    (
        uintptr_t context,
        WINC_DEVICE_HANDLE devHandle,
        const WINC_DEV_EVENT_RSP_ELEMS *const pElems
    )

  Summary:
    AEC process callback.

  Description:
    Callback will be called to process any AEC messages received.

  Precondition:
    WINC_DevAECCallbackRegister must be called to register the callback.

  Parameters:
    context   - Pointer to user context supplied when callback was registered.
    devHandle - WINC device handle.
    pElems    - Pointer to element structure.

  Returns:
    None.

  Remarks:
    Callback should call WINC_CmdReadParamElem to extract elements.

*/

void WDRV_WINC_IEProcessAEC
(
    uintptr_t context,
    WINC_DEVICE_HANDLE devHandle,
    const WINC_DEV_EVENT_RSP_ELEMS *const pElems
);

//*******************************************************************************
/*
  Function:
    WDRV_WINC_STATUS WDRV_WINC_IECustTxDataSet
    (
        DRV_HANDLE handle,
        WDRV_WINC_IE_FRAME_TYPE_MASK frameMask,
        const WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx
    )

  Summary:
    Configures the custom IE for transmission.

  Description:
    Management frames such as beacons and probe responses may contain an
    application provided custom IE. This function sends the custom IE store
    to the WINC for inclusion in the specified management frame types.

  Precondition:
    WDRV_WINC_Initialize must have been called.
    WDRV_WINC_Open must have been called to obtain a valid handle.
    WDRV_WINC_CustIEStoreCtxSetStorage must have been called to create a
    valid IE store context.

  Parameters:
    handle     - Client handle obtained by a call to WDRV_WINC_Open.
    frameMask  - Frame filter mask to add custom IE store.
    pCustIECtx - Pointer to custom IE store context.

  Returns:
    WDRV_WINC_STATUS_OK            - The request has been accepted.
    WDRV_WINC_STATUS_NOT_OPEN      - The driver instance is not open.
    WDRV_WINC_STATUS_INVALID_ARG   - The parameters were incorrect.
    WDRV_WINC_STATUS_REQUEST_ERROR - The request to the WINC was rejected.

  Remarks:
    Before calling this API, create IE storage using
    WDRV_WINC_CustIEStoreCtxSetStorage. IEs can be added or removed using
    WDRV_WINC_CustIEStoreCtxAddIE and WDRV_WINC_CustIEStoreCtxRemoveIE.
    To stop transmitting custom vendor specific IEs, call with frameMask = 0.

*/

WDRV_WINC_STATUS WDRV_WINC_IECustTxDataSet
(
    DRV_HANDLE handle,
    WDRV_WINC_IE_FRAME_TYPE_MASK frameMask,
    const WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx
);

#ifdef __cplusplus
}
#endif

#endif /* WDRV_WINC_MOD_DISABLE_IE */
#endif /* WDRV_WINC_IE_H */
