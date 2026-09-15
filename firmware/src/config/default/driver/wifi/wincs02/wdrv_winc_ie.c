/*******************************************************************************
  WINC Wireless Driver IE Source File

  File Name:
    wdrv_winc_ie.c

  Summary:
    WINC wireless driver IE implementation.

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>

#include "wdrv_winc.h"

#ifndef WDRV_WINC_MOD_DISABLE_IE

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver IE Internal Implementation
// *****************************************************************************
// *****************************************************************************

static void ieCmdRspCallbackHandler
(
    uintptr_t context,
    WINC_DEVICE_HANDLE devHandle,
    WINC_CMD_REQ_HANDLE cmdReqHandle,
    WINC_DEV_CMDREQ_EVENT_TYPE event,
    uintptr_t eventArg
)
{
    WDRV_WINC_DCPT *pDcpt = (WDRV_WINC_DCPT*)context;

    if (NULL == pDcpt)
    {
        return;
    }

    switch (event)
    {
        case WINC_DEV_CMDREQ_EVENT_TX_COMPLETE:
        {
            break;
        }

        case WINC_DEV_CMDREQ_EVENT_STATUS_COMPLETE:
        {
            OSAL_Free((WINC_COMMAND_REQUEST*)cmdReqHandle);
            break;
        }

        case WINC_DEV_CMDREQ_EVENT_CMD_STATUS:
        {
            /* Do nothing. */
            break;
        }

        case WINC_DEV_CMDREQ_EVENT_RSP_RECEIVED:
        {
            /* Do nothing. */
            break;
        }

        default:
        {
            /* Do nothing. */
            break;
        }
    }
}

static void ieProcessAEC
(
    const WDRV_WINC_DCPT *const pDcpt,
    uint16_t aecId,
    int numElems,
    const WINC_DEV_PARAM_ELEM *const pElems
)
{
    /* No AECs for TX-only VSIE. */
    (void)pDcpt;
    (void)aecId;
    (void)numElems;
    (void)pElems;
}

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver IE Implementation
// *****************************************************************************
// *****************************************************************************

void WDRV_WINC_IEProcessAEC
(
    uintptr_t context,
    WINC_DEVICE_HANDLE devHandle,
    const WINC_DEV_EVENT_RSP_ELEMS *const pElems
)
{
    WDRV_WINC_DCPT *pDcpt = (WDRV_WINC_DCPT *)context;
    const WINC_DEV_EVENT_RSP_ELEMS *pAEC = pElems;

    if ((NULL == pDcpt) || (NULL == pDcpt->pCtrl) || (NULL == pElems))
    {
        return;
    }

    ieProcessAEC(pDcpt, pAEC->rspId, pAEC->numElems, pAEC->elems);
}

WDRV_WINC_STATUS WDRV_WINC_IECustTxDataSet
(
    DRV_HANDLE handle,
    WDRV_WINC_IE_FRAME_TYPE_MASK frameMask,
    const WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx
)
{
    WDRV_WINC_DCPT *const pDcpt = (WDRV_WINC_DCPT *const)handle;
    WINC_CMD_REQ_HANDLE cmdReqHandle;

    /* Ensure the driver handle is valid. */
    if ((DRV_HANDLE_INVALID == handle) || (NULL == pDcpt) || (NULL == pDcpt->pCtrl))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the driver instance has been opened for use. */
    if (false == pDcpt->isOpen)
    {
        return WDRV_WINC_STATUS_NOT_OPEN;
    }

    /* Mask to valid frame type bits. */
    frameMask &= (WDRV_WINC_IE_FRAME_TYPE_BEACON | WDRV_WINC_IE_FRAME_TYPE_PROBE_RSP);

    /* If enabling TX, ensure a valid IE store context is provided. */
    if ((0U != frameMask) && (NULL == pCustIECtx))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    cmdReqHandle = WDRV_WINC_CmdReqInit(1, (0U != frameMask) ? pCustIECtx->curLength : 0U, ieCmdRspCallbackHandler, (uintptr_t)pDcpt);

    if (WINC_CMD_REQ_INVALID_HANDLE == cmdReqHandle)
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    if (0U != frameMask)
    {
        (void)WINC_CmdVSIE(cmdReqHandle, (uint8_t)frameMask, pCustIECtx->ieData, pCustIECtx->curLength);
    }
    else
    {
        (void)WINC_CmdVSIE(cmdReqHandle, (uint8_t)0, NULL, (uint16_t)0);
    }

    if (false == WDRV_WINC_DevTransmitCmdReq(pDcpt->pCtrl, cmdReqHandle))
    {
        return WDRV_WINC_STATUS_REQUEST_ERROR;
    }

    return WDRV_WINC_STATUS_OK;
}

#endif /* WDRV_WINC_MOD_DISABLE_IE */
