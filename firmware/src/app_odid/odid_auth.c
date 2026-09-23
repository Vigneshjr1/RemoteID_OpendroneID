#include <string.h>
#include <stdio.h>
#include "odid_auth.h"
#include "definitions.h"
#include "library/cryptoauthlib.h"

static bool s_ready;
static uint8_t s_serial[9];

static void auth_log(const char *msg)
{
    SERCOM0_USART_Write((uint8_t *)msg, strlen(msg));
}

bool ODID_Auth_Init(void)
{
    extern ATCAIfaceCfg atecc608_0_init_data;
    ATCA_STATUS status;
    char buf[64];

    s_ready = false;

    /* Try init with retries — ATECC608 sleeps by default and needs wake pulse */
    auth_log("Auth: waking ECC608...\r\n");
    for (int attempt = 0; attempt < 3; attempt++) {
        status = atcab_init(&atecc608_0_init_data);
        if (ATCA_SUCCESS == status) {
            break;
        }
        snprintf(buf, sizeof(buf), "Auth: attempt %d fail %d\r\n", attempt, (int)status);
        auth_log(buf);
        hal_delay_ms(100);
    }
    if (ATCA_SUCCESS != status) {
        /* I2C bus scan as final diagnostic */
        uint8_t devList[8];
        uint8_t nDevices = 0;
        auth_log("Auth: I2C scan...\r\n");
        if (SERCOM1_I2C_BusScan(0x08, 0x77, devList, &nDevices)) {
            snprintf(buf, sizeof(buf), "Auth: found %d dev(s)\r\n", nDevices);
            auth_log(buf);
            for (uint8_t i = 0; i < nDevices; i++) {
                snprintf(buf, sizeof(buf), "Auth: addr 0x%02X\r\n", devList[i]);
                auth_log(buf);
            }
        }
        snprintf(buf, sizeof(buf), "Auth: FAILED err %d\r\n", (int)status);
        auth_log(buf);
        return false;
    }

    status = atcab_read_serial_number(s_serial);
    if (ATCA_SUCCESS != status) {
        int len = snprintf(buf, sizeof(buf), "Auth: serial fail %d\r\n", (int)status);
        auth_log(buf);
        return false;
    }

    {
        int len = snprintf(buf, sizeof(buf),
            "Auth: SN %02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
            s_serial[0], s_serial[1], s_serial[2], s_serial[3],
            s_serial[4], s_serial[5], s_serial[6], s_serial[7], s_serial[8]);
        auth_log(buf);
    }

    s_ready = true;
    auth_log("Auth: ready\r\n");
    return true;
}

bool ODID_Auth_IsReady(void)
{
    return s_ready;
}

bool ODID_Auth_Sign(ODID_UAS_Data *pUasData)
{
    if (!s_ready || pUasData == NULL) {
        return false;
    }

    ODID_BasicID_encoded basicEncoded;
    ODID_Location_encoded locEncoded;
    ODID_System_encoded sysEncoded;
    uint8_t digest[32];
    uint8_t signature[ODID_AUTH_SIG_LEN];
    ATCA_STATUS status;

    memset(&basicEncoded, 0, sizeof(basicEncoded));
    memset(&locEncoded, 0, sizeof(locEncoded));
    memset(&sysEncoded, 0, sizeof(sysEncoded));

    if (pUasData->BasicIDValid[0]) {
        encodeBasicIDMessage(&basicEncoded, &pUasData->BasicID[0]);
    }
    if (pUasData->LocationValid) {
        encodeLocationMessage(&locEncoded, &pUasData->Location);
    }
    if (pUasData->SystemValid) {
        encodeSystemMessage(&sysEncoded, &pUasData->System);
    }

    /* Hash BasicID + Location + System using ATECC608 hardware SHA-256 */
    {
        uint8_t buf[sizeof(basicEncoded) + sizeof(locEncoded) + sizeof(sysEncoded)];
        size_t off = 0;
        memcpy(&buf[off], &basicEncoded, sizeof(basicEncoded)); off += sizeof(basicEncoded);
        memcpy(&buf[off], &locEncoded, sizeof(locEncoded));     off += sizeof(locEncoded);
        memcpy(&buf[off], &sysEncoded, sizeof(sysEncoded));     off += sizeof(sysEncoded);
        status = atcab_sha(off, buf, digest);
        if (ATCA_SUCCESS != status) {
            auth_log("Auth: sha fail\r\n");
            return false;
        }
    }

    status = atcab_sign(ODID_AUTH_KEY_SLOT, digest, signature);
    if (ATCA_SUCCESS != status) {
        auth_log("Auth: sign fail\r\n");
        return false;
    }

    /* Print digest, signature, and public key for verification */
    {
        char hex[140];
        uint8_t pubkey[64];

        auth_log("Auth: digest=");
        for (int i = 0; i < 32; i++) { snprintf(&hex[i*2], 3, "%02X", digest[i]); }
        hex[64] = '\0'; auth_log(hex); auth_log("\r\n");

        auth_log("Auth: sig_r=");
        for (int i = 0; i < 32; i++) { snprintf(&hex[i*2], 3, "%02X", signature[i]); }
        hex[64] = '\0'; auth_log(hex); auth_log("\r\n");

        auth_log("Auth: sig_s=");
        for (int i = 0; i < 32; i++) { snprintf(&hex[i*2], 3, "%02X", signature[32+i]); }
        hex[64] = '\0'; auth_log(hex); auth_log("\r\n");

        if (ATCA_SUCCESS == atcab_get_pubkey(ODID_AUTH_KEY_SLOT, pubkey)) {
            auth_log("Auth: pub_x=");
            for (int i = 0; i < 32; i++) { snprintf(&hex[i*2], 3, "%02X", pubkey[i]); }
            hex[64] = '\0'; auth_log(hex); auth_log("\r\n");

            auth_log("Auth: pub_y=");
            for (int i = 0; i < 32; i++) { snprintf(&hex[i*2], 3, "%02X", pubkey[32+i]); }
            hex[64] = '\0'; auth_log(hex); auth_log("\r\n");
        }
    }

    /*
     * ECDSA P-256 signature = 64 bytes.
     * Page 0: 17 bytes of AuthData
     * Page 1: 23 bytes
     * Page 2: 23 bytes
     * Page 3: 1 byte  (17 + 23 + 23 + 1 = 64)
     */
    uint32_t timestamp = RTC_Timer32CounterGet();

    /* Page 0 */
    memset(&pUasData->Auth[0], 0, sizeof(ODID_Auth_data));
    pUasData->Auth[0].DataPage = 0;
    pUasData->Auth[0].AuthType = ODID_AUTH_UAS_ID_SIGNATURE;
    pUasData->Auth[0].LastPageIndex = 3;
    pUasData->Auth[0].Length = ODID_AUTH_SIG_LEN;
    pUasData->Auth[0].Timestamp = timestamp;
    memcpy(pUasData->Auth[0].AuthData, &signature[0], ODID_AUTH_PAGE_ZERO_DATA_SIZE);
    pUasData->AuthValid[0] = 1;

    /* Page 1 */
    memset(&pUasData->Auth[1], 0, sizeof(ODID_Auth_data));
    pUasData->Auth[1].DataPage = 1;
    pUasData->Auth[1].AuthType = ODID_AUTH_UAS_ID_SIGNATURE;
    memcpy(pUasData->Auth[1].AuthData, &signature[ODID_AUTH_PAGE_ZERO_DATA_SIZE],
           ODID_AUTH_PAGE_NONZERO_DATA_SIZE);
    pUasData->AuthValid[1] = 1;

    /* Page 2 */
    memset(&pUasData->Auth[2], 0, sizeof(ODID_Auth_data));
    pUasData->Auth[2].DataPage = 2;
    pUasData->Auth[2].AuthType = ODID_AUTH_UAS_ID_SIGNATURE;
    memcpy(pUasData->Auth[2].AuthData,
           &signature[ODID_AUTH_PAGE_ZERO_DATA_SIZE + ODID_AUTH_PAGE_NONZERO_DATA_SIZE],
           ODID_AUTH_PAGE_NONZERO_DATA_SIZE);
    pUasData->AuthValid[2] = 1;

    /* Page 3 — last byte of signature */
    memset(&pUasData->Auth[3], 0, sizeof(ODID_Auth_data));
    pUasData->Auth[3].DataPage = 3;
    pUasData->Auth[3].AuthType = ODID_AUTH_UAS_ID_SIGNATURE;
    uint8_t remaining = ODID_AUTH_SIG_LEN -
        ODID_AUTH_PAGE_ZERO_DATA_SIZE - (2 * ODID_AUTH_PAGE_NONZERO_DATA_SIZE);
    memcpy(pUasData->Auth[3].AuthData,
           &signature[ODID_AUTH_PAGE_ZERO_DATA_SIZE + 2 * ODID_AUTH_PAGE_NONZERO_DATA_SIZE],
           remaining);
    pUasData->AuthValid[3] = 1;

    return true;
}
