#include "configuration.h"

#ifdef ODID_WIFI_DISABLE

#include "odid_vsie.h"
void ODID_VSIE_Init(void) {}
bool ODID_VSIE_EnableBeaconFilter(DRV_HANDLE wincHandle) { (void)wincHandle; return false; }
bool ODID_VSIE_UpdatePayload(DRV_HANDLE wincHandle, ODID_UAS_Data *pUasData, uint8_t sendCounter) {
    (void)wincHandle; (void)pUasData; (void)sendCounter; return false;
}

#else /* WiFi enabled */

#include <string.h>
#include "odid_vsie.h"
#include "definitions.h"
#include "wdrv_winc.h"

#define ODID_VSIE_OUI_0     0xFA
#define ODID_VSIE_OUI_1     0x0B
#define ODID_VSIE_OUI_2     0xBC
#define ODID_VSIE_OUI_TYPE  0x0D

#define ODID_VSIE_STORE_SIZE  (WDRV_WINC_CUSTIE_DATA_OFFSET + WDRV_WINC_CUSTIE_MAX_STORAGE_LEN)

static uint8_t s_ieStoreBuf[ODID_VSIE_STORE_SIZE];

static void vsie_log(const char *msg)
{
    SERCOM0_USART_Write((uint8_t *)msg, strlen(msg));
}

void ODID_VSIE_Init(void)
{
}

bool ODID_VSIE_EnableBeaconFilter(DRV_HANDLE wincHandle)
{
    (void)wincHandle;
    return true;
}

bool ODID_VSIE_UpdatePayload(DRV_HANDLE wincHandle, ODID_UAS_Data *pUasData, uint8_t sendCounter)
{
    if ((DRV_HANDLE_INVALID == wincHandle) || (NULL == pUasData))
    {
        return false;
    }

    uint8_t packBuf[250];
    int packLen = odid_message_build_pack(pUasData, packBuf, sizeof(packBuf));
    if (packLen <= 0)
    {
        return false;
    }

    /* Build the vendor-specific IE in standard 802.11 format:
     * [0xDD] [body_len] [FA 0B BC] [0D] [counter] [ODID_MessagePack...] */
    uint8_t ieBuf[266];
    uint16_t ieLen = 0;
    uint8_t bodyLen = (uint8_t)(3U + 1U + 1U + (uint8_t)packLen);

    ieBuf[ieLen++] = 0xDD;
    ieBuf[ieLen++] = bodyLen;
    ieBuf[ieLen++] = ODID_VSIE_OUI_0;
    ieBuf[ieLen++] = ODID_VSIE_OUI_1;
    ieBuf[ieLen++] = ODID_VSIE_OUI_2;
    ieBuf[ieLen++] = ODID_VSIE_OUI_TYPE;
    ieBuf[ieLen++] = sendCounter;
    memcpy(&ieBuf[ieLen], packBuf, (size_t)packLen);
    ieLen += (uint16_t)packLen;

    WDRV_WINC_CUST_IE_STORE_CONTEXT *pStore =
        WDRV_WINC_CustIEStoreCtxSetStorage(s_ieStoreBuf, sizeof(s_ieStoreBuf));
    if (NULL == pStore)
    {
        vsie_log("VSIE:store fail\r\n");
        return false;
    }

    if (ieLen > pStore->maxLength)
    {
        vsie_log("VSIE:too big\r\n");
        return false;
    }

    memcpy(pStore->ieData, ieBuf, ieLen);
    pStore->curLength = ieLen;

    WDRV_WINC_STATUS status = WDRV_WINC_IECustTxDataSet(
        wincHandle,
        (WDRV_WINC_IE_FRAME_TYPE_MASK)(WDRV_WINC_IE_FRAME_TYPE_BEACON | WDRV_WINC_IE_FRAME_TYPE_PROBE_RSP),
        pStore);

    if (WDRV_WINC_STATUS_OK != status)
    {
        vsie_log("VSIE:tx fail\r\n");
        return false;
    }

    vsie_log("VSIE:ok\r\n");
    return true;
}

#endif /* ODID_WIFI_DISABLE */
