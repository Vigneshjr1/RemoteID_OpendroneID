#include <string.h>
#include "odid_ble.h"
#include "app.h"
#include "app_ble.h"
#include "definitions.h"
#include "osal/osal_freertos_extend.h"
#include "FreeRTOS.h"
#include "timers.h"

// ASTM F3411 BLE advertising header constants
#define ODID_AD_TYPE_SERVICE_DATA   0x16
#define ODID_SERVICE_UUID_LSB       0xFA
#define ODID_SERVICE_UUID_MSB       0xFF
#define ODID_APP_CODE               0x0D

// Legacy BLE4 advertising set parameters
#define ODID_LEGACY_PRI_INTERVAL_MIN   192   // 120ms
#define ODID_LEGACY_PRI_INTERVAL_MAX   267   // 167ms
#define ODID_LEGACY_TX_POWER           0     // 0 dBm

// Extended BLE5 Coded PHY parameters
#define ODID_CODED_PRI_INTERVAL_MIN    1200  // 750ms
#define ODID_CODED_PRI_INTERVAL_MAX    1600  // 1000ms
#define ODID_CODED_TX_POWER            12    // +12 dBm

static uint8_t s_legacyPayload[ODID_LEGACY_PAYLOAD_LEN];
static uint8_t s_longrangePayload[ODID_LONGRANGE_PAYLOAD_MAX];
static uint8_t s_legacyPhase;
static uint8_t s_msgCounters[ODID_MSG_COUNTER_AMOUNT];

static TimerHandle_t s_legacyTimer;
static TimerHandle_t s_longrangeTimer;
static bool s_bleReady;
static volatile bool s_legacyUpdatePending;
static volatile bool s_longrangeUpdatePending;

static void ODID_LegacyTimerCallback(TimerHandle_t xTimer);
static void ODID_LongRangeTimerCallback(TimerHandle_t xTimer);

static void ODID_BLE_Log(const char *message)
{
    SERCOM1_USART_Write((uint8_t *)message, strlen(message));
}

void ODID_BLE_Init(void)
{
    int8_t selectedTxPower;
    BLE_GAP_ExtAdvParams_T advParams;

    memset(s_msgCounters, 0, sizeof(s_msgCounters));
    s_legacyPhase = 0;
    s_bleReady = true;
    s_legacyUpdatePending = false;
    s_longrangeUpdatePending = false;

    // Configure Advertising Set 0: Legacy BLE4 (1M PHY, non-connectable)
    memset(&advParams, 0, sizeof(advParams));
    advParams.advHandle = ODID_ADV_HANDLE_LEGACY;
    advParams.evtProperies = BLE_GAP_EXT_ADV_EVT_PROP_LEGACY_ADV; // Legacy PDU
    advParams.priIntervalMin = ODID_LEGACY_PRI_INTERVAL_MIN;
    advParams.priIntervalMax = ODID_LEGACY_PRI_INTERVAL_MAX;
    advParams.priChannelMap = BLE_GAP_ADV_CHANNEL_ALL;
    advParams.filterPolicy = BLE_GAP_ADV_FILTER_DEFAULT;
    advParams.txPower = ODID_LEGACY_TX_POWER;
    advParams.priPhy = BLE_GAP_PHY_TYPE_LE_1M;
    advParams.secMaxSkip = 0;
    advParams.secPhy = BLE_GAP_PHY_TYPE_LE_1M;
    advParams.sid = 0;
    advParams.scanReqNotifiEnable = false;
    if (MBA_RES_SUCCESS != BLE_GAP_SetExtAdvParams(&advParams, &selectedTxPower)) {
        ODID_BLE_Log("BLE: legacy params failed\r\n");
        s_bleReady = false;
    }

    // Configure Advertising Set 1: Extended BLE5 (Coded PHY S8, long range)
    memset(&advParams, 0, sizeof(advParams));
    advParams.advHandle = ODID_ADV_HANDLE_CODED;
    advParams.evtProperies = 0; // Non-connectable, non-scannable extended
    advParams.priIntervalMin = ODID_CODED_PRI_INTERVAL_MIN;
    advParams.priIntervalMax = ODID_CODED_PRI_INTERVAL_MAX;
    advParams.priChannelMap = BLE_GAP_ADV_CHANNEL_ALL;
    advParams.filterPolicy = BLE_GAP_ADV_FILTER_DEFAULT;
    advParams.txPower = ODID_CODED_TX_POWER;
    advParams.priPhy = BLE_GAP_PHY_TYPE_LE_CODED;
    advParams.secMaxSkip = 0;
    advParams.secPhy = BLE_GAP_PHY_TYPE_LE_CODED;
    advParams.sid = 1;
    advParams.scanReqNotifiEnable = false;
    advParams.priPhyOptions = BLE_GAP_CODED_PHY_HOST_PREFER_S8;
    advParams.secPhyOptions = BLE_GAP_CODED_PHY_HOST_PREFER_S8;
    if (MBA_RES_SUCCESS != BLE_GAP_SetExtAdvParams(&advParams, &selectedTxPower)) {
        ODID_BLE_Log("BLE: coded params failed\r\n");
        s_bleReady = false;
    }

    // Set initial empty advertising data for both sets
    uint8_t emptyData[] = {0x02, 0x01, 0x06}; // Flags only
    BLE_GAP_ExtAdvDataParams_T advDataParams;

    advDataParams.advHandle = ODID_ADV_HANDLE_LEGACY;
    advDataParams.operation = BLE_GAP_EXT_ADV_DATA_OP_COMPLETE;
    advDataParams.fragPreference = BLE_GAP_EXT_ADV_DATA_FRAG_ALL;
    advDataParams.advLen = sizeof(emptyData);
    advDataParams.p_advData = emptyData;
    if (MBA_RES_SUCCESS != BLE_GAP_SetExtAdvData(&advDataParams)) {
        ODID_BLE_Log("BLE: legacy initial data failed\r\n");
        s_bleReady = false;
    }

    advDataParams.advHandle = ODID_ADV_HANDLE_CODED;
    advDataParams.advLen = sizeof(emptyData);
    advDataParams.p_advData = emptyData;
    if (MBA_RES_SUCCESS != BLE_GAP_SetExtAdvData(&advDataParams)) {
        ODID_BLE_Log("BLE: coded initial data failed\r\n");
        s_bleReady = false;
    }

    // Create FreeRTOS software timers
    s_legacyTimer = xTimerCreate(
        "ODID_Legacy",
        pdMS_TO_TICKS(ODID_LEGACY_TIMER_PERIOD_MS),
        pdTRUE,  // Auto-reload
        NULL,
        ODID_LegacyTimerCallback
    );

    s_longrangeTimer = xTimerCreate(
        "ODID_LR",
        pdMS_TO_TICKS(ODID_LONGRANGE_TIMER_PERIOD_MS),
        pdTRUE,  // Auto-reload
        NULL,
        ODID_LongRangeTimerCallback
    );

    if ((NULL == s_legacyTimer) || (NULL == s_longrangeTimer)) {
        ODID_BLE_Log("BLE: timer creation failed\r\n");
        s_bleReady = false;
    }
}

void ODID_BLE_StartAdvertising(void)
{
    BLE_GAP_ExtAdvEnableParams_T enableParams[2];

    if (!s_bleReady) {
        ODID_BLE_Log("BLE: not ready\r\n");
        return;
    }

    // Enable legacy set
    enableParams[0].advHandle = ODID_ADV_HANDLE_LEGACY;
    enableParams[0].duration = 0;       // Continuous
    enableParams[0].maxExtAdvEvts = 0;  // No limit

    // Enable coded set
    enableParams[1].advHandle = ODID_ADV_HANDLE_CODED;
    enableParams[1].duration = 0;
    enableParams[1].maxExtAdvEvts = 0;

    if (MBA_RES_SUCCESS != BLE_GAP_SetExtAdvEnable(true, 2, enableParams)) {
        ODID_BLE_Log("BLE: advertising enable failed\r\n");
        return;
    }

    // Start the cycling timers
    if (s_legacyTimer != NULL) {
        if (pdPASS != xTimerStart(s_legacyTimer, 0)) {
            ODID_BLE_Log("BLE: legacy timer start failed\r\n");
        }
    }
    if (s_longrangeTimer != NULL) {
        if (pdPASS != xTimerStart(s_longrangeTimer, 0)) {
            ODID_BLE_Log("BLE: coded timer start failed\r\n");
        }
    }
}

void ODID_BLE_UpdateLegacy(ODID_UAS_Data *pUasData)
{
    uint8_t counterIndex = ODID_MSG_COUNTER_AMOUNT;

    s_legacyUpdatePending = false;

    if (pUasData == NULL) {
        return;
    }

    int legacyLength = 0;

    // ASTM F3411 BLE4 header: [AD_LEN, AD_TYPE, UUID_LSB, UUID_MSB, APP_CODE]
    const uint8_t header[] = {
        0x1E,                    // AD Length = 30 (total 31 bytes in buffer)
        ODID_AD_TYPE_SERVICE_DATA,
        ODID_SERVICE_UUID_LSB,
        ODID_SERVICE_UUID_MSB,
        ODID_APP_CODE
    };

    memset(s_legacyPayload, 0, sizeof(s_legacyPayload));
    memcpy(s_legacyPayload, header, sizeof(header));
    legacyLength = sizeof(header);

    switch (s_legacyPhase) {
        case ODID_LEGACY_PHASE_LOCATION: {
            if (pUasData->LocationValid) {
                ODID_Location_encoded encoded;
                memset(&encoded, 0, sizeof(encoded));
                if (encodeLocationMessage(&encoded, &pUasData->Location) == ODID_SUCCESS) {
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_LOCATION];
                    counterIndex = ODID_MSG_COUNTER_LOCATION;
                    legacyLength++;
                    memcpy(&s_legacyPayload[legacyLength], &encoded, sizeof(encoded));
                    legacyLength += sizeof(encoded);
                }
            }
            break;
        }

        case ODID_LEGACY_PHASE_BASIC_ID: {
            if (pUasData->BasicIDValid[0]) {
                ODID_BasicID_encoded encoded;
                memset(&encoded, 0, sizeof(encoded));
                if (encodeBasicIDMessage(&encoded, &pUasData->BasicID[0]) == ODID_SUCCESS) {
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_BASIC_ID];
                    counterIndex = ODID_MSG_COUNTER_BASIC_ID;
                    legacyLength++;
                    memcpy(&s_legacyPayload[legacyLength], &encoded, sizeof(encoded));
                    legacyLength += sizeof(encoded);
                }
            }
            break;
        }

        case ODID_LEGACY_PHASE_SELF_ID: {
            if (pUasData->SelfIDValid) {
                ODID_SelfID_encoded encoded;
                memset(&encoded, 0, sizeof(encoded));
                if (encodeSelfIDMessage(&encoded, &pUasData->SelfID) == ODID_SUCCESS) {
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_SELF_ID];
                    counterIndex = ODID_MSG_COUNTER_SELF_ID;
                    legacyLength++;
                    memcpy(&s_legacyPayload[legacyLength], &encoded, sizeof(encoded));
                    legacyLength += sizeof(encoded);
                }
            }
            break;
        }

        case ODID_LEGACY_PHASE_SYSTEM: {
            if (pUasData->SystemValid) {
                ODID_System_encoded encoded;
                memset(&encoded, 0, sizeof(encoded));
                if (encodeSystemMessage(&encoded, &pUasData->System) == ODID_SUCCESS) {
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_SYSTEM];
                    counterIndex = ODID_MSG_COUNTER_SYSTEM;
                    legacyLength++;
                    memcpy(&s_legacyPayload[legacyLength], &encoded, sizeof(encoded));
                    legacyLength += sizeof(encoded);
                }
            }
            break;
        }

        case ODID_LEGACY_PHASE_OPERATOR_ID: {
            if (pUasData->OperatorIDValid) {
                ODID_OperatorID_encoded encoded;
                memset(&encoded, 0, sizeof(encoded));
                if (encodeOperatorIDMessage(&encoded, &pUasData->OperatorID) == ODID_SUCCESS) {
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_OPERATOR_ID];
                    counterIndex = ODID_MSG_COUNTER_OPERATOR_ID;
                    legacyLength++;
                    memcpy(&s_legacyPayload[legacyLength], &encoded, sizeof(encoded));
                    legacyLength += sizeof(encoded);
                }
            }
            break;
        }

        default:
            break;
    }

    // Update the advertising data if we encoded something
    if (legacyLength > (int)sizeof(header)) {
        BLE_GAP_ExtAdvDataParams_T advDataParams;
        advDataParams.advHandle = ODID_ADV_HANDLE_LEGACY;
        advDataParams.operation = BLE_GAP_EXT_ADV_DATA_OP_COMPLETE;
        advDataParams.fragPreference = BLE_GAP_EXT_ADV_DATA_FRAG_ALL;
        advDataParams.advLen = (uint16_t)legacyLength;
        advDataParams.p_advData = s_legacyPayload;
        if (MBA_RES_SUCCESS != BLE_GAP_SetExtAdvData(&advDataParams)) {
            ODID_BLE_Log("BLE: legacy data update failed\r\n");
        } else if (counterIndex < ODID_MSG_COUNTER_AMOUNT) {
            s_msgCounters[counterIndex]++;
        }
    }

    // Always advance so one failed/invalid type cannot block Location updates.
    s_legacyPhase = (s_legacyPhase + 1U) % ODID_LEGACY_PHASE_COUNT;
}

void ODID_BLE_UpdateLongRange(ODID_UAS_Data *pUasData)
{
    s_longrangeUpdatePending = false;

    if (pUasData == NULL) {
        return;
    }

    // Build packed message payload using opendroneid library
    uint8_t packPayload[250];
    int packLength = odid_message_build_pack(pUasData, packPayload, sizeof(packPayload));
    if (packLength <= 0) {
        return;
    }

    // Build ASTM F3411 BLE5 header
    uint8_t headerLen = (uint8_t)(packLength + 5);
    const uint8_t header[] = {
        headerLen,
        ODID_AD_TYPE_SERVICE_DATA,
        ODID_SERVICE_UUID_LSB,
        ODID_SERVICE_UUID_MSB,
        ODID_APP_CODE,
        s_msgCounters[ODID_MSG_COUNTER_PACKED]
    };

    memcpy(s_longrangePayload, header, sizeof(header));
    memcpy(&s_longrangePayload[sizeof(header)], packPayload, packLength);
    int totalLength = sizeof(header) + packLength;

    // Update extended advertising data
    BLE_GAP_ExtAdvDataParams_T advDataParams;
    advDataParams.advHandle = ODID_ADV_HANDLE_CODED;
    advDataParams.operation = BLE_GAP_EXT_ADV_DATA_OP_COMPLETE;
    advDataParams.fragPreference = BLE_GAP_EXT_ADV_DATA_FRAG_ALL;
    advDataParams.advLen = (uint16_t)totalLength;
    advDataParams.p_advData = s_longrangePayload;
    if (MBA_RES_SUCCESS != BLE_GAP_SetExtAdvData(&advDataParams)) {
        ODID_BLE_Log("BLE: coded data update failed\r\n");
        return;
    }

    s_msgCounters[ODID_MSG_COUNTER_PACKED]++;
}

static void ODID_LegacyTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    APP_Msg_T appMsg;
    memset(&appMsg, 0, sizeof(appMsg));
    appMsg.msgId = APP_MSG_ODID_LEGACY_UPDATE;
    if (!s_legacyUpdatePending) {
        s_legacyUpdatePending = true;
        if (OSAL_RESULT_TRUE != OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0)) {
            s_legacyUpdatePending = false;
        }
    }
}

static void ODID_LongRangeTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    APP_Msg_T appMsg;
    memset(&appMsg, 0, sizeof(appMsg));
    appMsg.msgId = APP_MSG_ODID_LONGRANGE_UPDATE;
    if (!s_longrangeUpdatePending) {
        s_longrangeUpdatePending = true;
        if (OSAL_RESULT_TRUE != OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0)) {
            s_longrangeUpdatePending = false;
        }
    }
}
