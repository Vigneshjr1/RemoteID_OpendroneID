#include <string.h>
#include "odid_test.h"

void ODID_Test_PopulateData(ODID_UAS_Data *pUasData)
{
    odid_initUasData(pUasData);

    // Basic ID - Serial Number
    pUasData->BasicID[0].UAType = ODID_UATYPE_HELICOPTER_OR_MULTIROTOR;
    pUasData->BasicID[0].IDType = ODID_IDTYPE_SERIAL_NUMBER;
    strncpy((char *)pUasData->BasicID[0].UASID, "MCHP00TEST1234AB", ODID_ID_SIZE);
    pUasData->BasicIDValid[0] = 1;

    // Location - Microchip Technology HQ, Chandler AZ
    pUasData->Location.Status = ODID_STATUS_AIRBORNE;
    pUasData->Location.Direction = 45.0f;
    pUasData->Location.SpeedHorizontal = 5.5f;
    pUasData->Location.SpeedVertical = 0.5f;
    pUasData->Location.Latitude = 33.3025;
    pUasData->Location.Longitude = -111.8514;
    pUasData->Location.AltitudeBaro = 120.0f;
    pUasData->Location.AltitudeGeo = 122.0f;
    pUasData->Location.HeightType = ODID_HEIGHT_REF_OVER_TAKEOFF;
    pUasData->Location.Height = 20.0f;
    pUasData->Location.HorizAccuracy = ODID_HOR_ACC_10_METER;
    pUasData->Location.VertAccuracy = ODID_VER_ACC_10_METER;
    pUasData->Location.BaroAccuracy = ODID_VER_ACC_10_METER;
    pUasData->Location.SpeedAccuracy = ODID_SPEED_ACC_1_METERS_PER_SECOND;
    pUasData->Location.TSAccuracy = ODID_TIME_ACC_1_0_SECOND;
    pUasData->Location.TimeStamp = 60.0f;
    pUasData->LocationValid = 1;

    // Self ID
    pUasData->SelfID.DescType = ODID_DESC_TYPE_TEXT;
    strncpy(pUasData->SelfID.Desc, "PIC32 ODID Test Flight", ODID_STR_SIZE);
    pUasData->SelfIDValid = 1;

    // System - Operator at ground level nearby
    pUasData->System.OperatorLocationType = ODID_OPERATOR_LOCATION_TYPE_TAKEOFF;
    pUasData->System.ClassificationType = ODID_CLASSIFICATION_TYPE_EU;
    pUasData->System.OperatorLatitude = 33.3020;
    pUasData->System.OperatorLongitude = -111.8510;
    pUasData->System.AreaCount = 1;
    pUasData->System.AreaRadius = 0;
    pUasData->System.AreaCeiling = 150.0f;
    pUasData->System.AreaFloor = 0.0f;
    pUasData->System.CategoryEU = ODID_CATEGORY_EU_OPEN;
    pUasData->System.ClassEU = ODID_CLASS_EU_CLASS_1;
    pUasData->System.OperatorAltitudeGeo = 100.0f;
    pUasData->System.Timestamp = 1719200000;
    pUasData->SystemValid = 1;

    // Operator ID
    pUasData->OperatorID.OperatorIdType = ODID_OPERATOR_ID;
    strncpy((char *)pUasData->OperatorID.OperatorId, "OP-MCHP-TEST-01", ODID_ID_SIZE);
    pUasData->OperatorIDValid = 1;
}
