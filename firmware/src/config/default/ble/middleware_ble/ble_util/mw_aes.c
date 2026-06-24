/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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

/*******************************************************************************
  Middleware AES Source File

  Company:
    Microchip Technology Inc.

  File Name:
    mw_aes.c

  Summary:
    Implements AES encryption/decryption functions for use in applications.

  Description:
    This source file provides a set of functions to handle AES (Advanced
    Encryption Standard) encryption and decryption processes, facilitating
    secure data handling in application-level code. It abstracts the complexity
    of cryptographic operations, offering a user-friendly interface for
    integrating AES into various software solutions.
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stddef.h>
#include <string.h>
#include "mba_error_defs.h"
#include "mw_aes.h"

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
/** 
 * @brief Initializes AES CBC block cipher decryption.
 *
 * @param[out] p_ctx               Pointer to the AES context structure.
 * @param[in] p_aesKey             Pointer to the 16-byte encryption key.
 * @param[in] p_iv                 Pointer to the 16-byte initialization vector (IV).
 *
 * @retval MBA_RES_SUCCESS         Initialize successfully.
 * @retval MBA_RES_FAIL            Initialization failed.
 */
uint16_t MW_AES_CbcDecryptInit(MW_AES_Ctx_T * p_ctx, uint8_t *p_aesKey, uint8_t *p_iv)
{
    if (0 == CRYPT_AES_KeySet(&p_ctx->aesCtx, p_aesKey, 16, p_iv, CRYPT_AES_DECRYPTION))
    {
        return MBA_RES_SUCCESS;
    }
    else
    {
        return MBA_RES_FAIL;
    }
}


/**
 * @brief Decrypts a block of data using AES CBC mode.
 *
 * @param[in] p_ctx                Pointer to the AES context structure.
 * @param[in] length               The length of the data to be decrypted.
 * @param[out] p_plainText         Pointer to the buffer where the decrypted data will be stored.
 * @param[in] p_cipherText         Pointer to the buffer containing the data to be decrypted.
 *
 * @retval MBA_RES_SUCCESS         Decryption successful.
 * @retval MBA_RES_FAIL            Decryption failed.
 */
uint16_t MW_AES_AesCbcDecrypt(MW_AES_Ctx_T * p_ctx, uint16_t length, uint8_t *p_plainText, uint8_t *p_cipherText)
{
    if (0 == CRYPT_AES_CBC_Decrypt(&p_ctx->aesCtx, p_plainText, p_cipherText, length))
    {
        return MBA_RES_SUCCESS;
    }
    else
    {
        return MBA_RES_FAIL;
    }
}


/**
 * @brief Initializes AES ECB block cipher encryption.
 *
 * @param[out] p_ctx               Pointer to the AES context structure.
 * @param[in] p_aesKey             Pointer to the 16-byte encryption key.
 *
 * @retval MBA_RES_SUCCESS         Initialization successful.
 * @retval MBA_RES_FAIL            Initialization failed.
 */
uint16_t MW_AES_EcbEncryptInit(MW_AES_Ctx_T * p_ctx, uint8_t *p_aesKey)
{
    if (0 == CRYPT_AES_KeySet(&p_ctx->aesCtx, p_aesKey, 16, NULL, CRYPT_AES_ENCRYPTION))
    {
        return MBA_RES_SUCCESS;
    }
    else
    {
        return MBA_RES_FAIL;
    }
}


/**
 * @brief Encrypts a block of data using AES ECB mode.
 *
 * @param[in] p_ctx                Pointer to the AES context structure.
 * @param[in] length               The length of the data to be encrypted.
 * @param[out] p_cipherText        Pointer to the buffer where the encrypted data will be stored.
 * @param[in] p_plainText          Pointer to the buffer containing the data to be encrypted.
 *
 * @retval MBA_RES_SUCCESS         Encryption successful.
 * @retval MBA_RES_FAIL            Encryption failed.
 */
uint16_t MW_AES_AesEcbEncrypt(MW_AES_Ctx_T * p_ctx, uint16_t length, uint8_t *p_cipherText, uint8_t *p_plainText)
{
    if (0 == CRYPT_AES_DIRECT_Encrypt(&p_ctx->aesCtx, p_cipherText, p_plainText))
    {
        return MBA_RES_SUCCESS;
    }
    else
    {
        return MBA_RES_FAIL;
    }

}

/** 
 * @brief Performs AES Mac calculation using Cipher-Block-Chaining (CBC).
 *
 * @param[in]  p_ctx               Pointer to the AES context structure. See @ref MW_AES_Ctx_T.
 * @param[out] out                 Pointer to the 16-byte encryption key.
 * @param[in]  in                  Pointer to buffer holding the data to be decrypted.
 * @param[in]  inSz                Size of the input data, in bytes.
 * @param[in]  nonce               Pointer to the 7-13 byte unique value that is used only once in a cryptographic communication.
 * @param[in]  aad                 Pointer to the additional authentication data.
 * @param[in]  aadSz               The size of the additional authentication data (AAD).
 * @param[in]  tagSz               Specifies the number of bytes used to represent the authenticaion tag.

 * @retval MBA_RES_SUCCESS         Initialization successful.
 * @retval MBA_RES_FAIL            Initialization failed.
 */

int MW_AES_CcmAuthTag(MW_AES_Ctx_T* p_ctx, uint8_t* out, const uint8_t *in, unsigned int inSz, const uint8_t *nonce,const uint8_t* aad, uint16_t aadSz,uint8_t tagSz) 
{
    CRYPT_AES_CTX backup ;
    uint8_t M = tagSz,L=2;
    uint8_t mac[CRYPT_AES_BLOCK_SIZE] = {0};
    uint8_t buffer[CRYPT_AES_BLOCK_SIZE] = {0};
    uint8_t s0[CRYPT_AES_BLOCK_SIZE] = {0};
    uint8_t counter[CRYPT_AES_BLOCK_SIZE] = {0};

    uint8_t paddedSz = (inSz / CRYPT_AES_BLOCK_SIZE + 3)  * CRYPT_AES_BLOCK_SIZE;
    uint8_t padded[paddedSz];

    memset(padded, 0x00, paddedSz);

    padded[0] = (aadSz > 0 ? 0x40 : 0x00) | (((M - 2) / 2) << 3) | (L - 1);
    memcpy(padded + 1, nonce, 13);
    for (uint8_t i = 0; i < L; i++)
    {
        padded[15 - i] = (inSz >> (8 * i)) & 0xFF;
    }

    padded[CRYPT_AES_BLOCK_SIZE] = (aadSz >> 8) & 0xFF;
    padded[CRYPT_AES_BLOCK_SIZE+1] = aadSz & 0xFF;
    memcpy(padded + CRYPT_AES_BLOCK_SIZE + 2, aad, aadSz);

    uint8_t plaintextBlocks = 0;

    while(inSz > 0)
    {
        uint8_t copy = inSz >= CRYPT_AES_BLOCK_SIZE ? CRYPT_AES_BLOCK_SIZE:inSz;
        uint8_t wIdx=  (plaintextBlocks+2 )* CRYPT_AES_BLOCK_SIZE;
        memcpy(&padded[wIdx], in+ (plaintextBlocks << 4), copy);
        inSz -= copy;
        plaintextBlocks++;
    }
    memcpy(&backup,&p_ctx->aesCtx,sizeof(CRYPT_AES_CTX));

    for (uint8_t i = 0; i < paddedSz; i += CRYPT_AES_BLOCK_SIZE)
    {
        for (uint8_t j = 0; j < CRYPT_AES_BLOCK_SIZE; j++)
        {
            buffer[j] = padded[i + j] ^ mac[j];
        }
        if (CRYPT_AES_CBC_Encrypt(&p_ctx->aesCtx, mac, buffer, CRYPT_AES_BLOCK_SIZE))
        {
            return MBA_RES_FAIL;
        }
        memcpy(&p_ctx->aesCtx,&backup,sizeof(CRYPT_AES_CTX));
    }

    counter[0] = 0x01;
    memcpy(&counter[1], nonce, 13);

    if (CRYPT_AES_DIRECT_Encrypt(&p_ctx->aesCtx, s0, counter))
    {
        return MBA_RES_FAIL;
    }

    for (uint8_t i = 0; i < M; i++)
    {
        out[i] = mac[i] ^ s0[i];
    }
    memcpy(&p_ctx->aesCtx,&backup,sizeof(CRYPT_AES_CTX));
    return MBA_RES_SUCCESS;
}

/**
 * @brief Initializes AES CCM encryption.
 *
 * @param[out] p_ctx               Pointer to the AES context structure.
 * @param[in] p_aesKey             Pointer to the 16-byte encryption key.
 * @param[in] p_nonce              Pointer to the nonce used for encryption.
 * @param[in] nonceSz              The size of p_nonce, between 7 and 13 bytes.
 * @param[in] tagSz                The tag size used for encryption, must be a value in {4, 6, 8, 10, 12, 14, 16}.
 * @param[in] p_aad                Pointer to the additional authentication data.
 * @param[in] aadSz                The size of p_aad, can be 0 if p_aad is NULL.
 * @param[in] dataSz               The size of the data to be encrypted.
 *
 * @retval MBA_RES_SUCCESS         Initialization successful.
 * @retval MBA_RES_FAIL            Initialization failed.
 */
uint16_t MW_AES_CcmEncryptInit(MW_AES_Ctx_T * p_ctx, uint8_t *p_aesKey, uint8_t *p_nonce, uint8_t nonceSz, uint8_t tagSz, uint8_t *p_aad, uint16_t aadSz, uint16_t dataSz)
{
    p_ctx->ccmCtx.p_nonce = p_nonce;
    p_ctx->ccmCtx.nonceSz = nonceSz;
    p_ctx->ccmCtx.tagSz = tagSz;
    p_ctx->ccmCtx.p_aad = p_aad;
    p_ctx->ccmCtx.aadSz = aadSz;
    p_ctx->ccmCtx.dataSz = dataSz;

    uint8_t zeroNonce[CRYPT_AES_BLOCK_SIZE] = {0};
    uint8_t paddedKey[CRYPT_AES_BLOCK_SIZE] = {0};
    uint8_t keysize = (sizeof(paddedKey) / sizeof(paddedKey[0])) * sizeof(uint8_t);
    keysize = keysize <= CRYPT_AES_BLOCK_SIZE ? keysize : CRYPT_AES_BLOCK_SIZE;
    memcpy(paddedKey, p_aesKey, keysize);

    if (0 == CRYPT_AES_KeySet(&p_ctx->aesCtx, paddedKey, CRYPT_AES_BLOCK_SIZE, zeroNonce, CRYPT_AES_ENCRYPTION))
    {
        return MBA_RES_SUCCESS;
    }
    else
    {
        return MBA_RES_FAIL;
    }
}


/**
 * @brief Encrypts data using AES CCM mode.
 *
 * @param[in] p_ctx                Pointer to the AES context structure.
 * @param[in] length               The length of the data to be encrypted. 
 *                                 Must be a multiple of 16 bytes, except for the last data fragment.
 * @param[in] p_plainText          Pointer to the buffer containing the data to be encrypted.
 * @param[out] p_cipherText        Pointer to the buffer where the encrypted data will be stored.
 * @param[out] p_tag               Pointer to the buffer where the authentication tag will be stored.
 *                                 Only valid if p_plainText is the last data fragment.
 *
 * @retval MBA_RES_SUCCESS         Encryption successful.
 * @retval MBA_RES_FAIL            Encryption failed.
 * @retval MBA_RES_AUTHENTICATION_FAILURE Authentication failed.
 * @retval MBA_RES_INVALID_PARA    Invalid parameter input.
 */
uint16_t MW_AES_AesCcmEncrypt(MW_AES_Ctx_T * p_ctx, uint16_t length, uint8_t *p_plainText, uint8_t *p_cipherText, uint8_t *p_tag)
{
    uint8_t count = 0;
    uint8_t *p_nonce = p_ctx->ccmCtx.p_nonce;
    uint8_t *p_aad = p_ctx->ccmCtx.p_aad;
    int aadSz = p_ctx->ccmCtx.aadSz;
    uint8_t tagSz = p_ctx->ccmCtx.tagSz;
    uint8_t plaintextSz = p_ctx->ccmCtx.dataSz;
    
    uint8_t blocks = plaintextSz / CRYPT_AES_BLOCK_SIZE +1;

    if(tagSz < 4 || tagSz > 16 || tagSz % 2)
        return MBA_RES_INVALID_PARA;

    if (MW_AES_CcmAuthTag(p_ctx, p_tag, p_plainText, plaintextSz,p_nonce, p_aad,aadSz,tagSz))
    {
        return MBA_RES_AUTHENTICATION_FAILURE;
    }
    count++;

    for(uint16_t i=count;i <=blocks;i++)
    {
        uint8_t counter[CRYPT_AES_BLOCK_SIZE] ={0}; 
        uint8_t buffer[CRYPT_AES_BLOCK_SIZE] = {0};
        uint8_t shift = (i-1)*CRYPT_AES_BLOCK_SIZE;
        int end = plaintextSz - shift < CRYPT_AES_BLOCK_SIZE ? plaintextSz: shift+CRYPT_AES_BLOCK_SIZE;

        counter[0] = 0x01;
        memcpy(&counter[1], p_nonce, 13);
        counter[14] = (i>>8) & 0xFF;
        counter[15] = i & 0xFF;

        if (CRYPT_AES_DIRECT_Encrypt(&p_ctx->aesCtx, buffer, counter)) 
        {
            return MBA_RES_FAIL;
        }

        for (int j= shift; j < end; j++) 
        { 
            p_cipherText[j] = p_plainText[j] ^ buffer[j % CRYPT_AES_BLOCK_SIZE];
        }
    }

    return MBA_RES_SUCCESS;
}

/**
 * @brief Initializes AES CCM decryption.
 *
 * @param[out] p_ctx               Pointer to the AES context structure.
 * @param[in] p_aesKey             Pointer to the 16-byte encryption key.
 * @param[in] p_nonce              Pointer to the nonce used for encryption.
 * @param[in] nonceSz              The size of p_nonce, between 7 and 13 bytes.
 * @param[in] tagSz                The tag size used for encryption, must be a value in {4, 6, 8, 10, 12, 14, 16}.
 * @param[in] p_aad                Pointer to the additional authentication data.
 * @param[in] aadSz                The size of p_aad, can be 0 if p_aad is NULL.
 * @param[in] dataSz               The size of the data to be decrypted.
 *
 * @retval MBA_RES_SUCCESS         Initialization successful.
 * @retval MBA_RES_FAIL            Initialization failed.
 */
uint16_t MW_AES_CcmDecryptInit(MW_AES_Ctx_T * p_ctx, uint8_t *p_aesKey, uint8_t *p_nonce, uint8_t nonceSz, uint8_t tagSz, uint8_t *p_aad, uint16_t aadSz, uint16_t dataSz)
{
    if(0 == MW_AES_CcmEncryptInit(p_ctx, p_aesKey,p_nonce,nonceSz,tagSz,p_aad,aadSz,dataSz))
    {
        return MBA_RES_SUCCESS;
    }
    else
    {
        return MBA_RES_FAIL;
    }
}

/**
 * @brief Decrypts data using AES CCM mode.
 *
 * @param[in] p_ctx                Pointer to the AES context structure.
 * @param[in] length               The length of the data to be decrypted. 
 *                                 Must be a multiple of 16 bytes, except for the last data fragment.
 * @param[in] p_cipherText         Pointer to the buffer containing the data to be decrypted.
 * @param[in] p_tag                Pointer to the buffer containing the authentication tag.
 *                                 Only be used if p_cipherText is the last data fragment.
 * @param[out] p_plainText         Pointer to the buffer where the decrypted data will be stored.
 *
 * @retval MBA_RES_SUCCESS         Encryption successful.
 * @retval MBA_RES_FAIL            Encryption failed.
 * @retval MBA_RES_AUTHENTICATION_FAILURE Authentication failed.
 * @retval MBA_RES_INVALID_PARA    Invalid parameter input.
 */
uint16_t MW_AES_AesCcmDecrypt(MW_AES_Ctx_T *p_ctx, uint16_t length, uint8_t *p_cipherText, uint8_t *p_tag, uint8_t *p_plainText)
{
    CRYPT_AES_CTX backup;
    uint8_t *p_nonce = p_ctx->ccmCtx.p_nonce;
    uint8_t *p_aad = p_ctx->ccmCtx.p_aad;
    int aadSz = p_ctx->ccmCtx.aadSz;
    uint8_t tagSz = p_ctx->ccmCtx.tagSz;
    uint8_t ciphertextSz = p_ctx->ccmCtx.dataSz;
    
    uint8_t mac[CRYPT_AES_BLOCK_SIZE] = {0};
    uint8_t count = 1;
    uint8_t shift = 0;

    if(tagSz < 4 || tagSz > 16 || tagSz % 2)
        return MBA_RES_INVALID_PARA;

    uint8_t blocks = ciphertextSz / CRYPT_AES_BLOCK_SIZE + 1;
    memcpy(&backup, &p_ctx->aesCtx, sizeof(CRYPT_AES_CTX));
    for (int i = count; i <= blocks; i++) 
    {
        uint8_t counter[CRYPT_AES_BLOCK_SIZE] = {0};
        uint8_t buffer[CRYPT_AES_BLOCK_SIZE] = {0};
        uint8_t end;
        counter[0] = 0x01;
        memcpy(&counter[1], p_nonce, 13);
        counter[14] = (i >> 8) & 0xFF;
        counter[15] = i & 0xFF;

        if (CRYPT_AES_DIRECT_Encrypt(&p_ctx->aesCtx, buffer, counter)) 
        {
            return MBA_RES_FAIL;
        }
        if (!i) continue;
    
        shift = (i - 1) * CRYPT_AES_BLOCK_SIZE;
        end = ciphertextSz - shift < CRYPT_AES_BLOCK_SIZE ? ciphertextSz : shift + CRYPT_AES_BLOCK_SIZE;

        for (int j = shift; j < end; j++) 
        {
            p_plainText[j] = p_cipherText[j] ^ buffer[j % CRYPT_AES_BLOCK_SIZE];
        }
    }

    memcpy(&p_ctx->aesCtx, &backup, sizeof(CRYPT_AES_CTX));

    if (MW_AES_CcmAuthTag(p_ctx, mac, p_plainText, ciphertextSz, p_nonce, p_aad, aadSz,tagSz))
    {
        return MBA_RES_FAIL;
    }
    if(memcmp(p_tag, mac, tagSz))
    {
        return MBA_RES_AUTHENTICATION_FAILURE;
    }
    
    return MBA_RES_SUCCESS; 
}
