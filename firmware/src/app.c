// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "app.h"
#include "definitions.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app_ble.h"
#include "app_odid/odid_ble.h"
#include "app_odid/odid_mavlink.h"
#include "app_odid/odid_uart.h"
#include "app_odid/odid_wifi.h"
#include "app_odid/odid_test.h"
#include "app_odid/odid_auth.h"




// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************





// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/



// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/



// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

void GPIO_LED_Initialize ( void )
{
    CFG_REGS->CFG_CFGCON0CLR = CFG_CFGCON0_JTAGEN_Msk;

    /*  PB  */
    /* RB2, RB3 and RB5 are reserved for WINCS02 QSPI. */
}

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;


    appData.appQueue = xQueueCreate( 64, sizeof(APP_Msg_T) );
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    APP_Msg_T    appMsg[1];
    APP_Msg_T   *p_appMsg;
    p_appMsg=appMsg;




    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
            GPIO_LED_Initialize();
#ifndef ODID_WIFI_ONLY_TEST
            APP_BleStackInit();
#endif
            if (!(RTC_REGS->MODE0.RTC_CTRLA & RTC_MODE0_CTRLA_ENABLE_Msk))
            {
                RTC_Timer32Start();
            }

            // Initialize OpenDroneID subsystem
            ODID_MAVLink_Init();
            ODID_UART_Init();

            ODID_Auth_Init();

#ifdef ODID_USE_TEST_DATA
            ODID_Test_PopulateData(ODID_MAVLink_GetUasData());
#endif

            // Sign the initial UAS data with ATECC608
            ODID_Auth_Sign(ODID_MAVLink_GetUasData());

            // The shared ODID data drives both BLE and WiFi broadcasts.
            ODID_WiFi_Init();
#ifndef ODID_WIFI_ONLY_TEST
            /* Load real RID data before enabling the advertising sets. */
            ODID_BLE_UpdateLegacy(ODID_MAVLink_GetUasData());
            ODID_BLE_UpdateLongRange(ODID_MAVLink_GetUasData());
            ODID_BLE_StartAdvertising();
#endif
            ODID_WiFi_StartAdvertising();

            SERCOM0_USART_Write((uint8_t *)"ODID Started\r\n", 14);

            if (appInitialized)
            {
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            /* Keep byte parsing and message conversion out of the USART ISR. */
            ODID_UART_Tasks();

            if (OSAL_QUEUE_Receive(&appData.appQueue, &appMsg, pdMS_TO_TICKS(1)))
            {
                if(p_appMsg->msgId == APP_MSG_BLE_STACK_EVT)
                {
                    APP_BleStackEvtHandler((STACK_Event_T *)p_appMsg->msgData);
                }
                else if(p_appMsg->msgId == APP_MSG_ODID_LEGACY_UPDATE)
                {
                    ODID_BLE_UpdateLegacy(ODID_MAVLink_GetUasData());
                }
                else if(p_appMsg->msgId == APP_MSG_ODID_LONGRANGE_UPDATE)
                {
                    ODID_BLE_UpdateLongRange(ODID_MAVLink_GetUasData());
                }
            }
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
