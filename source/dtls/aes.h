/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_AES_H_
#define NBIOT_SOURCE_DTLS_AES_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AES_MAXKEYBITS  (256)
#define AES_MAXKEYBYTES (AES_MAXKEYBITS/8)
    /* for 256-bit keys we need 14 rounds for a 128 we only need 10 round */
#define AES_MAXROUNDS   10

/* bergmann: to avoid conflicts with typedefs from certain Contiki platforms,
 * the following type names have been prefixed with "aes_":
*/
typedef unsigned char   u_char;
typedef uint8_t         aes_u8;
typedef uint16_t        aes_u16;
typedef uint32_t        aes_u32;

/* The structure for key information */
typedef struct
{
#ifdef WITH_AES_DECRYPT
    int     enc_only;                    /* context contains only encrypt schedule */
#endif
    int     Nr;                          /* key-length-dependent number of rounds */
    aes_u32 ek[4 * (AES_MAXROUNDS + 1)]; /* encrypt key schedule */
#ifdef WITH_AES_DECRYPT
    aes_u32 dk[4 * (AES_MAXROUNDS + 1)];/* decrypt key schedule */
#endif
} rijndael_ctx;

int rijndael_set_key( rijndael_ctx *ctx,
                      const u_char *key,
                      int           bits );
int rijndael_set_key_enc_only( rijndael_ctx *ctx,
                               const u_char *key,
                               int           bits );
void rijndael_decrypt( rijndael_ctx *ctx,
                       const u_char *src,
                       u_char       *dst );
void rijndael_encrypt( rijndael_ctx *ctx,
                       const u_char *src,
                       u_char       *dst );

int rijndaelKeySetupEnc( aes_u32      rk[/*4*(Nr + 1)*/],
                         const aes_u8 cipherKey[],
                         int          keyBits );
int rijndaelKeySetupDec( aes_u32      rk[/*4*(Nr + 1)*/],
                         const aes_u8 cipherKey[],
                         int          keyBits);
void rijndaelEncrypt( const aes_u32 rk[/*4*(Nr + 1)*/],
                      int           Nr,
                      const aes_u8  pt[16],
                      aes_u8        ct[16] );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_AES_H_ */