#include "configuration.h"

#ifdef ODID_WIFI_DISABLE

#include "odid_wifi.h"
void ODID_WiFi_Init(void) {}
void ODID_WiFi_StartAdvertising(void) {}
void ODID_WiFi_UpdateBeacon(ODID_UAS_Data *pUasData) { (void)pUasData; }

#else /* WiFi enabled */

#include <string.h>
#include <stdio.h>
#include "odid_wifi.h"
#include "app.h"
#include "definitions.h"
#include "wdrv_winc.h"
#include "wdrv_winc_client_api.h"
#include "wdrv_winc_wifi.h"
#include "wdrv_winc_dhcps.h"
#include "osal/osal_freertos_extend.h"
#include "FreeRTOS.h"
#include "task.h"

#include "odid_vsie.h"
#include "odid_mavlink.h"

#define ODID_WIFI_SSID        "UAS_ID_OPEN"
#define ODID_WIFI_CHANNEL     6

typedef enum {
    WIFI_STATE_WAIT_READY,
    WIFI_STATE_OPEN,
    WIFI_STATE_SET_REG_DOMAIN,
    WIFI_STATE_WAIT_REG_DOMAIN,
    WIFI_STATE_CONFIGURE_AP,
    WIFI_STATE_START_AP,
    WIFI_STATE_RUNNING,
    WIFI_STATE_ERROR
} ODID_WiFi_State_t;

static uint8_t s_sendCounter;
static bool s_vsieEnabled;
static ODID_UAS_Data s_uasDataSnapshot;

static DRV_HANDLE s_wincHandle = DRV_HANDLE_INVALID;
static ODID_WiFi_State_t s_wifiState = WIFI_STATE_WAIT_READY;

static WDRV_WINC_BSS_CONTEXT s_bssCtx;
static WDRV_WINC_AUTH_CONTEXT s_authCtx;
static WDRV_WINC_CONN_CFG s_wifiCfg;

static volatile bool s_regDomainAcked = false;

static void ODID_WiFi_RunStateMachine(void);

static void ODID_WiFi_Log(const char *message)
{
    SERCOM1_USART_Write((uint8_t *)message, strlen(message));
}

static void ODID_WiFi_RegDomainCallback(DRV_HANDLE handle,
    uint8_t index, uint8_t ofTotal, bool isCurrent,
    const WDRV_WINC_REGDOMAIN_INFO *const pRegDomInfo)
{
    (void)handle;
    if ((1 == index) && (1 == ofTotal) && isCurrent && (pRegDomInfo != NULL))
    {
        s_regDomainAcked = true;
    }
}

static void ODID_WiFi_APNotifyCallback(DRV_HANDLE handle,
    WDRV_WINC_ASSOC_HANDLE assocHandle, WDRV_WINC_CONN_STATE currentState)
{
    (void)handle;
    (void)assocHandle;
    (void)currentState;
}

static TaskHandle_t s_wifiTaskHandle;

static void ODID_WiFiTask(void *pvParameters)
{
    TickType_t lastWakeTime;

    (void)pvParameters;
    ODID_WiFi_Log("WiFi: task started\r\n");

    while (s_wifiState != WIFI_STATE_RUNNING)
    {
        ODID_WiFi_RunStateMachine();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ODID_WiFi_Log("WiFi: AP running\r\n");

    if (!s_vsieEnabled) {
        ODID_VSIE_EnableBeaconFilter(s_wincHandle);
        s_vsieEnabled = true;
    }

    lastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (ODID_MAVLink_CopyUasData(&s_uasDataSnapshot)) {
            bool ok = ODID_VSIE_UpdatePayload(
                s_wincHandle, &s_uasDataSnapshot, s_sendCounter);
            if (!ok) {
                vTaskDelay(pdMS_TO_TICKS(100));
                ok = ODID_VSIE_UpdatePayload(
                    s_wincHandle, &s_uasDataSnapshot, s_sendCounter);
            }
            if (ok) {
                s_sendCounter++;
            }
        }

        /* Keep updates on an absolute one-second cadence without drift. */
        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(ODID_WIFI_BEACON_TIMER_PERIOD_MS));
    }
}

void ODID_WiFi_Init(void)
{
    s_sendCounter = 0;
    s_vsieEnabled = false;
    s_wifiState = WIFI_STATE_WAIT_READY;
    s_wincHandle = DRV_HANDLE_INVALID;
    s_regDomainAcked = false;
    s_wifiTaskHandle = NULL;

    ODID_VSIE_Init();
    ODID_WiFi_Log("WiFi: init\r\n");
}

void ODID_WiFi_StartAdvertising(void)
{
    if (s_wifiTaskHandle == NULL) {
        xTaskCreate(ODID_WiFiTask, "ODID_WiFi", 1280, NULL, 1, &s_wifiTaskHandle);
    }
}

static void ODID_WiFi_RunStateMachine(void)
{
    WDRV_WINC_STATUS status;

    switch (s_wifiState)
    {
        case WIFI_STATE_WAIT_READY:
        {
            SYS_STATUS drvStatus = WDRV_WINC_Status(sysObj.drvWifiWinc);
            if (SYS_STATUS_READY == drvStatus)
            {
                ODID_WiFi_Log("WiFi: driver ready\r\n");
                s_wifiState = WIFI_STATE_OPEN;
            }
            break;
        }

        case WIFI_STATE_OPEN:
        {
            s_wincHandle = WDRV_WINC_Open(0, 0);
            if (DRV_HANDLE_INVALID != s_wincHandle)
            {
                ODID_WiFi_Log("WiFi: open\r\n");
                s_wifiState = WIFI_STATE_SET_REG_DOMAIN;
            }
            else
            {
                ODID_WiFi_Log("WiFi: open failed\r\n");
                s_wifiState = WIFI_STATE_ERROR;
            }
            break;
        }

        case WIFI_STATE_SET_REG_DOMAIN:
        {
            s_regDomainAcked = false;
            status = WDRV_WINC_WifiRegDomainSet(s_wincHandle,
                (const uint8_t *)ODID_WIFI_REG_DOMAIN,
                strlen(ODID_WIFI_REG_DOMAIN),
                ODID_WiFi_RegDomainCallback);
            if (WDRV_WINC_STATUS_OK == status)
            {
                ODID_WiFi_Log("WiFi: reg domain set\r\n");
                s_wifiState = WIFI_STATE_WAIT_REG_DOMAIN;
            }
            else if (WDRV_WINC_STATUS_RETRY_REQUEST == status)
            {
                break;
            }
            else
            {
                ODID_WiFi_Log("WiFi: reg domain failed\r\n");
                s_wifiState = WIFI_STATE_ERROR;
            }
            break;
        }

        case WIFI_STATE_WAIT_REG_DOMAIN:
        {
            if (s_regDomainAcked)
            {
                ODID_WiFi_Log("WiFi: reg domain ACK\r\n");
                s_wifiState = WIFI_STATE_CONFIGURE_AP;
            }
            break;
        }

        case WIFI_STATE_CONFIGURE_AP:
        {
            status = WDRV_WINC_BSSCtxSetDefaults(&s_bssCtx);
            if (WDRV_WINC_STATUS_OK != status) {
                s_wifiState = WIFI_STATE_ERROR;
                break;
            }

            status = WDRV_WINC_BSSCtxSetSSID(&s_bssCtx,
                (uint8_t *)ODID_WIFI_SSID, strlen(ODID_WIFI_SSID));
            if (WDRV_WINC_STATUS_OK != status) {
                s_wifiState = WIFI_STATE_ERROR;
                break;
            }

            status = WDRV_WINC_BSSCtxSetChannel(&s_bssCtx, ODID_WIFI_CHANNEL);
            if (WDRV_WINC_STATUS_OK != status) {
                s_wifiState = WIFI_STATE_ERROR;
                break;
            }

            status = WDRV_WINC_AuthCtxSetOpen(&s_authCtx);
            if (WDRV_WINC_STATUS_OK != status) {
                s_wifiState = WIFI_STATE_ERROR;
                break;
            }

            WDRV_WINC_APDefaultWiFiCfg(&s_wifiCfg);

            WDRV_WINC_IP_MULTI_ADDRESS apAddr;
            apAddr.v4.v[0] = 192;
            apAddr.v4.v[1] = 168;
            apAddr.v4.v[2] = 4;
            apAddr.v4.v[3] = 1;
            WDRV_WINC_NetIfIPAddrSet(s_wincHandle, WDRV_WINC_NETIF_IDX_0,
                WDRV_WINC_IP_ADDRESS_TYPE_IPV4, &apAddr, 24);

            WDRV_WINC_IPV4_ADDR poolStart;
            poolStart.v[0] = 192;
            poolStart.v[1] = 168;
            poolStart.v[2] = 4;
            poolStart.v[3] = 2;
            WDRV_WINC_DHCPSPoolAddressSet(s_wincHandle, WDRV_WINC_DHCPS_IDX_0, &poolStart);
            WDRV_WINC_DHCPSEnableSet(s_wincHandle, WDRV_WINC_DHCPS_IDX_0, true, NULL);

            ODID_WiFi_Log("WiFi: AP configured\r\n");
            s_wifiState = WIFI_STATE_START_AP;
            break;
        }

        case WIFI_STATE_START_AP:
        {
            status = WDRV_WINC_APStart(s_wincHandle, &s_bssCtx, &s_authCtx,
                &s_wifiCfg, ODID_WiFi_APNotifyCallback);
            if (WDRV_WINC_STATUS_OK == status) {
                ODID_WiFi_Log("WiFi: AP accepted\r\n");
                {
                    WDRV_WINC_DCPT *pD = (WDRV_WINC_DCPT *)s_wincHandle;
                    if ((NULL != pD) && (NULL != pD->pCtrl) && pD->pCtrl->fwVersion.isValid)
                    {
                        char vbuf[40];
                        snprintf(vbuf, sizeof(vbuf), "FW:%d.%d.%d\r\n",
                            pD->pCtrl->fwVersion.version.major,
                            pD->pCtrl->fwVersion.version.minor,
                            pD->pCtrl->fwVersion.version.patch);
                        ODID_WiFi_Log(vbuf);
                    }
                }
                s_wifiState = WIFI_STATE_RUNNING;
            } else {
                ODID_WiFi_Log("WiFi: AP failed\r\n");
                s_wifiState = WIFI_STATE_ERROR;
            }
            break;
        }

        case WIFI_STATE_RUNNING:
            break;

        case WIFI_STATE_ERROR:
            ODID_WiFi_Log("WiFi: error state\r\n");
            break;

        default:
            break;
    }
}

void ODID_WiFi_UpdateBeacon(ODID_UAS_Data *pUasData)
{
    (void)pUasData;
}

#endif /* ODID_WIFI_DISABLE */
