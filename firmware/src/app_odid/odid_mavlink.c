#include <string.h>
#include <stdio.h>
#include "odid_mavlink.h"
#include "definitions.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "third_party/mavlink/common/mavlink.h"

static ODID_UAS_Data s_uasData;
static volatile bool s_hasValidData;
static mavlink_message_t s_mavMsg;
static mavlink_status_t s_mavStatus;
static SemaphoreHandle_t s_dataMutex;

void ODID_MAVLink_Init(void)
{
    odid_initUasData(&s_uasData);
    s_hasValidData = false;
    memset(&s_mavMsg, 0, sizeof(s_mavMsg));
    memset(&s_mavStatus, 0, sizeof(s_mavStatus));
    s_dataMutex = xSemaphoreCreateMutex();
}

static void ODID_HandleMavlinkMessage(mavlink_message_t *msg)
{
    switch (msg->msgid) {
        case MAVLINK_MSG_ID_OPEN_DRONE_ID_BASIC_ID: {
            mavlink_open_drone_id_basic_id_t basic_id;
            mavlink_msg_open_drone_id_basic_id_decode(msg, &basic_id);

            s_uasData.BasicID[0].UAType = (ODID_uatype_t)basic_id.ua_type;
            s_uasData.BasicID[0].IDType = (ODID_idtype_t)basic_id.id_type;
            memcpy(s_uasData.BasicID[0].UASID, basic_id.uas_id, ODID_ID_SIZE);
            s_uasData.BasicIDValid[0] = 1;
            s_hasValidData = true;
            break;
        }

        case MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION: {
            mavlink_open_drone_id_location_t location;
            mavlink_msg_open_drone_id_location_decode(msg, &location);

            s_uasData.Location.Status = (ODID_status_t)location.status;
            s_uasData.Location.Direction = location.direction / 100.0f;
            s_uasData.Location.SpeedHorizontal = location.speed_horizontal / 100.0f;
            s_uasData.Location.SpeedVertical = location.speed_vertical / 100.0f;
            s_uasData.Location.Latitude = location.latitude / 1.0e7;
            s_uasData.Location.Longitude = location.longitude / 1.0e7;
            s_uasData.Location.AltitudeBaro = location.altitude_barometric;
            s_uasData.Location.AltitudeGeo = location.altitude_geodetic;
            s_uasData.Location.HeightType = (ODID_Height_reference_t)location.height_reference;
            s_uasData.Location.Height = location.height;
            s_uasData.Location.HorizAccuracy = (ODID_Horizontal_accuracy_t)location.horizontal_accuracy;
            s_uasData.Location.VertAccuracy = (ODID_Vertical_accuracy_t)location.vertical_accuracy;
            s_uasData.Location.BaroAccuracy = (ODID_Vertical_accuracy_t)location.barometer_accuracy;
            s_uasData.Location.SpeedAccuracy = (ODID_Speed_accuracy_t)location.speed_accuracy;
            s_uasData.Location.TSAccuracy = (ODID_Timestamp_accuracy_t)location.timestamp_accuracy;
            s_uasData.Location.TimeStamp = location.timestamp;
            s_uasData.LocationValid = 1;
            s_hasValidData = true;
            break;
        }

        case MAVLINK_MSG_ID_OPEN_DRONE_ID_SELF_ID: {
            mavlink_open_drone_id_self_id_t self_id;
            mavlink_msg_open_drone_id_self_id_decode(msg, &self_id);

            s_uasData.SelfID.DescType = (ODID_desctype_t)self_id.description_type;
            memcpy(s_uasData.SelfID.Desc, self_id.description, ODID_STR_SIZE);
            s_uasData.SelfIDValid = 1;
            s_hasValidData = true;
            break;
        }

        case MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM: {
            mavlink_open_drone_id_system_t system;
            mavlink_msg_open_drone_id_system_decode(msg, &system);

            s_uasData.System.OperatorLocationType = (ODID_operator_location_type_t)system.operator_location_type;
            s_uasData.System.ClassificationType = (ODID_classification_type_t)system.classification_type;
            s_uasData.System.OperatorLatitude = system.operator_latitude / 1.0e7;
            s_uasData.System.OperatorLongitude = system.operator_longitude / 1.0e7;
            s_uasData.System.AreaCount = system.area_count;
            s_uasData.System.AreaRadius = system.area_radius;
            s_uasData.System.AreaCeiling = system.area_ceiling;
            s_uasData.System.AreaFloor = system.area_floor;
            s_uasData.System.CategoryEU = (ODID_category_EU_t)system.category_eu;
            s_uasData.System.ClassEU = (ODID_class_EU_t)system.class_eu;
            s_uasData.System.OperatorAltitudeGeo = system.operator_altitude_geo;
            s_uasData.System.Timestamp = system.timestamp;
            s_uasData.SystemValid = 1;
            s_hasValidData = true;
            break;
        }

        case MAVLINK_MSG_ID_OPEN_DRONE_ID_OPERATOR_ID: {
            mavlink_open_drone_id_operator_id_t operator_id;
            mavlink_msg_open_drone_id_operator_id_decode(msg, &operator_id);

            s_uasData.OperatorID.OperatorIdType = (ODID_operatorIdType_t)operator_id.operator_id_type;
            memcpy(s_uasData.OperatorID.OperatorId, operator_id.operator_id, ODID_ID_SIZE);
            s_uasData.OperatorIDValid = 1;
            s_hasValidData = true;
            break;
        }

        default:
            break;
    }
}

static volatile uint32_t s_byteCount = 0;
static volatile uint8_t s_parseResult = 0;

void ODID_MAVLink_ProcessByte(uint8_t byte)
{
    s_byteCount++;
    uint8_t result = mavlink_frame_char(MAVLINK_COMM_0, byte, &s_mavMsg, &s_mavStatus);
    if (result != MAVLINK_FRAMING_INCOMPLETE) {
        s_parseResult = result;
        if (result == MAVLINK_FRAMING_OK) {
            if (NULL != s_dataMutex) {
                if (pdTRUE == xSemaphoreTake(s_dataMutex, portMAX_DELAY)) {
                    ODID_HandleMavlinkMessage(&s_mavMsg);
                    (void)xSemaphoreGive(s_dataMutex);
                }
            } else {
                ODID_HandleMavlinkMessage(&s_mavMsg);
            }
        }
    }
}

uint32_t ODID_MAVLink_GetByteCount(void) { return s_byteCount; }
uint8_t ODID_MAVLink_GetParseResult(void) { return s_parseResult; }

ODID_UAS_Data* ODID_MAVLink_GetUasData(void)
{
    return &s_uasData;
}

bool ODID_MAVLink_CopyUasData(ODID_UAS_Data *pSnapshot)
{
    if ((NULL == pSnapshot) || (NULL == s_dataMutex)) {
        return false;
    }

    if (pdTRUE != xSemaphoreTake(s_dataMutex, portMAX_DELAY)) {
        return false;
    }

    memcpy(pSnapshot, &s_uasData, sizeof(*pSnapshot));
    (void)xSemaphoreGive(s_dataMutex);
    return true;
}

bool ODID_MAVLink_HasValidData(void)
{
    return s_hasValidData;
}
