/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_SHA2_H_
#define NBIOT_SOURCE_DTLS_SHA2_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SHA-256 Various Length Definitions */
#define SHA256_BLOCK_LENGTH         64
#define SHA256_DIGEST_LENGTH        32
#define SHA256_DIGEST_STRING_LENGTH (SHA256_DIGEST_LENGTH * 2 + 1)

typedef struct _SHA256_CTX
{
    uint32_t state[8];
    uint64_t bitcount;
    uint8_t  buffer[SHA256_BLOCK_LENGTH];
} SHA256_CTX;

void SHA256_Init( SHA256_CTX *context );
void SHA256_Update( SHA256_CTX    *context,
                    const uint8_t *data,
                    size_t         len );
void SHA256_Final( uint8_t     digest[SHA256_DIGEST_LENGTH],
                   SHA256_CTX *context );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_SHA2_H_ */