#include <string.h>
#include "odid_ble.h"
#include "app.h"
#include "app_ble.h"
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

static void ODID_LegacyTimerCallback(TimerHandle_t xTimer);
static void ODID_LongRangeTimerCallback(TimerHandle_t xTimer);

void ODID_BLE_Init(void)
{
    int8_t selectedTxPower;
    BLE_GAP_ExtAdvParams_T advParams;

    memset(s_msgCounters, 0, sizeof(s_msgCounters));
    s_legacyPhase = 0;

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
    BLE_GAP_SetExtAdvParams(&advParams, &selectedTxPower);

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
    BLE_GAP_SetExtAdvParams(&advParams, &selectedTxPower);

    // Set initial empty advertising data for both sets
    uint8_t emptyData[] = {0x02, 0x01, 0x06}; // Flags only
    BLE_GAP_ExtAdvDataParams_T advDataParams;

    advDataParams.advHandle = ODID_ADV_HANDLE_LEGACY;
    advDataParams.operation = BLE_GAP_EXT_ADV_DATA_OP_COMPLETE;
    advDataParams.fragPreference = BLE_GAP_EXT_ADV_DATA_FRAG_ALL;
    advDataParams.advLen = sizeof(emptyData);
    advDataParams.p_advData = emptyData;
    BLE_GAP_SetExtAdvData(&advDataParams);

    advDataParams.advHandle = ODID_ADV_HANDLE_CODED;
    advDataParams.advLen = sizeof(emptyData);
    advDataParams.p_advData = emptyData;
    BLE_GAP_SetExtAdvData(&advDataParams);

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
}

void ODID_BLE_StartAdvertising(void)
{
    BLE_GAP_ExtAdvEnableParams_T enableParams[2];

    // Enable legacy set
    enableParams[0].advHandle = ODID_ADV_HANDLE_LEGACY;
    enableParams[0].duration = 0;       // Continuous
    enableParams[0].maxExtAdvEvts = 0;  // No limit

    // Enable coded set
    enableParams[1].advHandle = ODID_ADV_HANDLE_CODED;
    enableParams[1].duration = 0;
    enableParams[1].maxExtAdvEvts = 0;

    BLE_GAP_SetExtAdvEnable(true, 2, enableParams);

    // Start the cycling timers
    if (s_legacyTimer != NULL) {
        xTimerStart(s_legacyTimer, 0);
    }
    if (s_longrangeTimer != NULL) {
        xTimerStart(s_longrangeTimer, 0);
    }
}

void ODID_BLE_UpdateLegacy(ODID_UAS_Data *pUasData)
{
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
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_LOCATION]++;
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
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_BASIC_ID]++;
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
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_SELF_ID]++;
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
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_SYSTEM]++;
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
                    s_legacyPayload[legacyLength] = s_msgCounters[ODID_MSG_COUNTER_OPERATOR_ID]++;
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

    // Advance to next phase
    s_legacyPhase = (s_legacyPhase + 1) % ODID_LEGACY_PHASE_COUNT;

    // Update the advertising data if we encoded something
    if (legacyLength > (int)sizeof(header)) {
        BLE_GAP_ExtAdvDataParams_T advDataParams;
        advDataParams.advHandle = ODID_ADV_HANDLE_LEGACY;
        advDataParams.operation = BLE_GAP_EXT_ADV_DATA_OP_COMPLETE;
        advDataParams.fragPreference = BLE_GAP_EXT_ADV_DATA_FRAG_ALL;
        advDataParams.advLen = (uint16_t)legacyLength;
        advDataParams.p_advData = s_legacyPayload;
        BLE_GAP_SetExtAdvData(&advDataParams);
    }
}

void ODID_BLE_UpdateLongRange(ODID_UAS_Data *pUasData)
{
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
        s_msgCounters[ODID_MSG_COUNTER_PACKED]++
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
    BLE_GAP_SetExtAdvData(&advDataParams);
}

static void ODID_LegacyTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    APP_Msg_T appMsg;
    appMsg.msgId = APP_MSG_ODID_LEGACY_UPDATE;
    OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
}

static void ODID_LongRangeTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    APP_Msg_T appMsg;
    appMsg.msgId = APP_MSG_ODID_LONGRANGE_UPDATE;
    OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
}
