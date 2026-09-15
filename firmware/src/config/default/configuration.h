/*******************************************************************************
  System Configuration Header

  File Name:
    configuration.h

  Summary:
    Build-time configuration header for the system defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    This configuration header must not define any prototypes or data
    definitions (or include any files that do).  It only provides macro
    definitions for build-time configuration options

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section Includes other configuration headers necessary to completely
    define this configuration.
*/

#include "user.h"
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: System Configuration
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: System Service Configuration
// *****************************************************************************
// *****************************************************************************
#define SYS_CONSOLE_DEVICE_MAX_INSTANCES   			(1U)
#define SYS_CONSOLE_UART_MAX_INSTANCES 	   			(1U)
#define SYS_CONSOLE_USB_CDC_MAX_INSTANCES 	   		(0U)
#define SYS_CONSOLE_PRINT_BUFFER_SIZE        		(200U)


#define SYS_CONSOLE_INDEX_0                       0






// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************

/* Broadcast the built-in Remote ID fixture when no MAVLink source is connected. */
#define ODID_USE_TEST_DATA
#define ODID_WIFI_REG_DOMAIN  "GEN"


/*** WiFi WINC Driver Configuration ***/
#define WDRV_WINC_EIC_SOURCE
#define WDRV_WINC_DEVICE_USE_SYS_DEBUG
#define WDRV_WINC_RTOS_STACK_SIZE           1024
#define WDRV_WINC_RTOS_TASK_PRIORITY        2
#define WDRV_WINC_DEV_RX_BUFF_SZ            2048
#define WINC_SOCK_SLAB_ALLOC_MODE           1
#define WDRV_WINC_DEV_SOCK_SLAB_NUM         1
#define WDRV_WINC_DEV_SOCK_SLAB_SZ          1472
#define WINC_SOCK_NUM_SOCKETS               1
#define WINC_SOCK_BUF_RX_SZ                 1024
#define WINC_SOCK_BUF_TX_SZ                 1024
#define WINC_SOCK_BUF_RX_PKT_BUF_NUM        3
#define WINC_SOCK_BUF_TX_PKT_BUF_NUM        2
#define WDRV_WINC_MOD_DISABLE_OTA
#define WDRV_WINC_MOD_DISABLE_SYSLOG

/*** System Time Configuration ***/
#define SYS_TIME_INDEX_0                            (0)
#define SYS_TIME_MAX_TIMERS                         (5)
#define SYS_TIME_HW_COUNTER_WIDTH                   (16)
#define SYS_TIME_HW_COUNTER_PERIOD                  (0xFFFFU)
#define SYS_TIME_HW_COUNTER_HALF_PERIOD             (SYS_TIME_HW_COUNTER_PERIOD>>1)
#define SYS_TIME_CPU_CLOCK_FREQUENCY                (64000000)
#define SYS_TIME_COMPARE_UPDATE_EXECUTION_CYCLES    (232)


// *****************************************************************************
// *****************************************************************************
// Section: Middleware & Other Library Configuration
// *****************************************************************************
// *****************************************************************************

/*** wolfCrypt Library Configuration ***/
#define MICROCHIP_PIC32
#define MICROCHIP_MPLAB_HARMONY
#define MICROCHIP_MPLAB_HARMONY_3
#define HAVE_MCAPI
#define SIZEOF_LONG_LONG 8
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_FILESYSTEM
#define USE_FAST_MATH
#define NO_PWDBASED
#define HAVE_MCAPI
#define WOLF_CRYPTO_CB  // provide call-back support
#define WOLFCRYPT_ONLY
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO
// ---------- CRYPTO HARDWARE MANIFEST START ----------
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_ECC_HW_PUKCC
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_AES_HW_U2238
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_SHA_HW_11105
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_SHA_HW_U2010
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_TRNG_HW_U2242
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_RSA_HW_PUKCC
// ---------- CRYPTO HARDWARE MANIFEST END ----------
#undef WOLFSSL_HAVE_MIN
#undef WOLFSSL_HAVE_MAX
// ---------- FUNCTIONAL CONFIGURATION START ----------
#define WOLFSSL_AES_SMALL_TABLES
#define NO_MD4
#define NO_MD5
#define NO_SHA // specifically, no SHA1 (legacy name)
#define NO_SHA256
#define NO_SHA224
#define NO_HMAC
#define NO_DES3
#define WOLFSSL_AES_128
#define NO_AES_192 // not supported by HW accelerator
#define NO_AES_256 // not supported by HW accelerator
#define WOLFSSL_AES_DIRECT
#define HAVE_AES_DECRYPT
#define WOLFSSL_HAVE_MCHP_HW_AES_DIRECT
#define HAVE_AES_CBC
#define WOLFSSL_HAVE_MCHP_HW_AES_CBC
#define NO_RC4
#define NO_HC128
#define NO_RABBIT
#define NO_DH
#define NO_DSA
#define NO_RSA
#define NO_DEV_RANDOM
#define WC_NO_RNG
#define WC_NO_HASHDRBG
#define WC_NO_HARDEN
#define SINGLE_THREADED
#define NO_ASN
#define NO_SIG_WRAPPER
#define NO_ERROR_STRINGS
#define WOLFSSL_MAX_ERROR_SZ 38 // Fix Mandatory Misra 21.18 caused by removing error strings with defining NO_ERROR_STRINGS
#define NO_WOLFSSL_MEMORY
// ---------- FUNCTIONAL CONFIGURATION END ----------



// *****************************************************************************
// *****************************************************************************
// Section: Application Configuration
// *****************************************************************************
// *****************************************************************************
#define CONFIG_BLE_GAP_DEV_NAME_VALUE                    {"Microchip"}
// GAP Service option
#define CONFIG_BLE_GAP_SVC_DEV_NAME_WRITE                false             /* Enable Device Name Write Property */
#define CONFIG_BLE_GAP_SVC_APPEARANCE                    0x0                 /* Appearance */
#define CONFIG_BLE_GAP_SVC_PERI_PRE_CP                   false                /* Enable Peripheral Preferred Connection Parameters */
#define CONFIG_BLE_GAP_SVC_ENC_DATA_KEY_MATL             false                   /* Enable Encrypted Data Key Material */
#define CONFIG_BLE_GAP_SVC_LE_GATT_SEC_LVLS              false                    /* Enable LE GATT Security Levels */

//Extended Advertising Set 1
#define CONFIG_BLE_GAP_EXT_ADV_DATA                  {0x02, 0x01, 0x05, 0x0A, 0x09, 0x4D, 0x69, 0x63, 0x72, 0x6F, 0x63, 0x68, 0x69, 0x70, 0x04, 0x16, 0xDA, 0xFE, 0x0F}
#define CONFIG_BLE_GAP_EXT_ADV_DATA_ORIG_LEN         19
#define CONFIG_BLE_GAP_EXT_SCAN_RSP_DATA             {0x0C, 0x09, 0x70, 0x69, 0x63, 0x33, 0x32, 0x77, 0x6D, 0x2D, 0x62, 0x77, 0x31}
#define CONFIG_BLE_GAP_EXT_SCAN_RSP_DATA_ORIG_LEN    13
#define CONFIG_BLE_GAP_EXT_ADV_ADV_SET_HANDLE        1        /* Advertising Handle */
#define CONFIG_BLE_GAP_EXT_ADV_EVT_PROPERTIES        (0 )  /* Advertising Event Properties */
#define CONFIG_BLE_GAP_EXT_ADV_PRI_INTERVAL_MIN      1600   /* Primary Advertising Interval Min */
#define CONFIG_BLE_GAP_EXT_ADV_PRI_INTERVAL_MAX      1600   /* Primary Advertising Interval Max */
#define CONFIG_BLE_GAP_EXT_ADV_PRI_CHANNEL_MAP       BLE_GAP_ADV_CHANNEL_ALL       /* Primary Advertising Channel Map */
#define CONFIG_BLE_GAP_EXT_ADV_FILT_POLICY           BLE_GAP_ADV_FILTER_DEFAULT    /* Advertising Filter Policy */
#define CONFIG_BLE_GAP_EXT_ADV_TX_POWER              12    /* Advertising TX Power */
#define CONFIG_BLE_GAP_EXT_ADV_PRI_ADV_PHY           BLE_GAP_PHY_TYPE_LE_CODED      /* Primary Advertising PHY */
#define CONFIG_BLE_GAP_EXT_ADV_MAX_SKIP              10     /* Secondary Advertising Max Skip */
#define CONFIG_BLE_GAP_EXT_ADV_SEC_ADV_PHY           BLE_GAP_PHY_TYPE_LE_CODED      /* Secondary Advertising PHY */
#define CONFIG_BLE_GAP_EXT_ADV_SID                   1     /* Advertising SID */
#define CONFIG_BLE_GAP_EXT_ADV_SCAN_ENABLE           false   /* Scan Request Notification Enable */
#define CONFIG_BLE_GAP_EXT_ADV_PRI_ADV_PHY_OPT       BLE_GAP_CODED_PHY_HOST_NO_PREFERRED  /* Primary Advertising PHY Option */
#define CONFIG_BLE_GAP_EXT_ADV_SEC_ADV_PHY_OPT       BLE_GAP_CODED_PHY_HOST_NO_PREFERRED  /* Secondary Advertising PHY Option */


#define CONFIG_BLE_GAP_CONN_TX_PWR               15 /* Connection TX Power */

// Configure SMP parameters
#define CONFIG_BLE_SMP_IOCAP_TYPE   BLE_SMP_IO_NOINPUTNOOUTPUT  /* IO Capability */
#define CONFIG_BLE_SMP_OPTION       (0 |BLE_SMP_OPTION_BONDING |BLE_SMP_OPTION_SECURE_CONNECTION) /* Authentication Setting */

// Configure BLE_DM middleware parameters
#define CONFIG_BLE_DM_SEC_AUTO_ACCEPT      true /* Auto Accept Security Request */
#define CONFIG_BLE_DM_AUTO_REPLY_UPD_REQ   true      /* Auto Accept Connection Parameter Update Request */
#define CONFIG_BLE_DM_MIN_CONN_INTERVAL    6    /* Minimum Connection Interval */
#define CONFIG_BLE_DM_MAX_CONN_INTERVAL    3200   /* Maximum Connection Interval */
#define CONFIG_BLE_DM_MIN_CONN_LATENCY     0    /* Minimum Connection Latency */
#define CONFIG_BLE_DM_MAX_CONN_LATENCY     499    /* Maximum Connection Latency */





//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // CONFIGURATION_H
/*******************************************************************************
 End of File
*/
