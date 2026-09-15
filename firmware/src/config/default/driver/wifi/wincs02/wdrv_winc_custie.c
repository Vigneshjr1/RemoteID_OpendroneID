/*******************************************************************************
  WINC Wireless Driver Custom IE Source File

  File Name:
    wdrv_winc_custie.c

  Summary:
    WINC wireless driver custom IE implementation.

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>

#include "wdrv_winc.h"

#ifndef WDRV_WINC_MOD_DISABLE_CUSTIE

// *****************************************************************************
// *****************************************************************************
// Section: WINC Driver Custom IE Implementation
// *****************************************************************************
// *****************************************************************************

WDRV_WINC_CUST_IE_STORE_CONTEXT* WDRV_WINC_CustIEStoreCtxSetStorage
(
    uint8_t *const pStorage,
    uint16_t lenStorage
)
{
    WDRV_WINC_CUST_IE_STORE_CONTEXT *pCustIECtx;

    /* Ensure user application provided storage is valid. */
    if ((NULL == pStorage) || (lenStorage < WDRV_WINC_CUSTIE_MIN_STORAGE_LEN))
    {
        return NULL;
    }

    /* Initialize the storage area. */
    pCustIECtx = (WDRV_WINC_CUST_IE_STORE_CONTEXT*)pStorage;

    (void)memset(pStorage, 0, lenStorage);

    if (lenStorage > (WDRV_WINC_CUSTIE_MAX_STORAGE_LEN + WDRV_WINC_CUSTIE_DATA_OFFSET))
    {
        pCustIECtx->maxLength = WDRV_WINC_CUSTIE_MAX_STORAGE_LEN;
    }
    else
    {
        pCustIECtx->maxLength = lenStorage - WDRV_WINC_CUSTIE_DATA_OFFSET;
    }

    return pCustIECtx;
}

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxAddIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id,
    const uint8_t *const pData,
    uint8_t dataLength
)
{
    WDRV_WINC_CUST_IE *pIE;
    uint16_t dataOffset;

    /* Ensure the storage context is valid. */
    if ((NULL == pCustIECtx) || (NULL == pData))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Ensure the new IE will fit. */
    if ((pCustIECtx->maxLength - pCustIECtx->curLength) < (dataLength + WDRV_WINC_CUSTIE_DATA_OFFSET))
    {
        return WDRV_WINC_STATUS_NO_SPACE;
    }

    /* Walk the IEs until the end is found. */
    dataOffset = 0;
    pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];

    while ((0U != pIE->id) && (dataOffset < pCustIECtx->curLength))
    {
        dataOffset += (WDRV_WINC_CUSTIE_DATA_OFFSET + pIE->length);
        pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];
    }

    /* Copy in new IE. */
    pIE->id = id;
    pIE->length = dataLength;
    (void)memcpy(&pIE->data[0], pData, dataLength);

    pCustIECtx->curLength += (dataLength + WDRV_WINC_CUSTIE_DATA_OFFSET);

    return WDRV_WINC_STATUS_OK;
}

WDRV_WINC_STATUS WDRV_WINC_CustIEStoreCtxRemoveIE
(
    WDRV_WINC_CUST_IE_STORE_CONTEXT *const pCustIECtx,
    uint8_t id
)
{
    WDRV_WINC_CUST_IE *pIE;
    uint16_t dataOffset;

    /* Ensure the storage context and ID are valid. */
    if ((NULL == pCustIECtx) || (0U == id))
    {
        return WDRV_WINC_STATUS_INVALID_ARG;
    }

    /* Walk the IEs looking for the supplied ID. */
    dataOffset = 0;
    pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];

    while ((0U != pIE->id) && (dataOffset < pCustIECtx->curLength))
    {
        if (pIE->id == id)
        {
            /* The ID has been found, copy remaining IEs over the top to remove it. */
            pCustIECtx->curLength -= (WDRV_WINC_CUSTIE_DATA_OFFSET + pIE->length);

            (void)memcpy(&pCustIECtx->ieData[dataOffset],
                    &pCustIECtx->ieData[dataOffset + (WDRV_WINC_CUSTIE_DATA_OFFSET + pIE->length)],
                    pCustIECtx->maxLength - dataOffset - (WDRV_WINC_CUSTIE_DATA_OFFSET + pIE->length));

            pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[pCustIECtx->curLength];

            pIE->id = 0;
            pIE->length = 0;
            break;
        }

        dataOffset += (WDRV_WINC_CUSTIE_DATA_OFFSET + pIE->length);
        pIE = (WDRV_WINC_CUST_IE*)&pCustIECtx->ieData[dataOffset];
    }

    return WDRV_WINC_STATUS_OK;
}

#endif /* WDRV_WINC_MOD_DISABLE_CUSTIE */
