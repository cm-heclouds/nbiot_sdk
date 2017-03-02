/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_ECC_H_
#define NBIOT_SOURCE_DTLS_ECC_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ECC_KEY_LENGTH   32 /* bytes */
#define ECC_ARRAY_LENGTH 8

extern const uint32_t ecc_g_point_x[8];
extern const uint32_t ecc_g_point_y[8];

void ecc_ec_mult( const uint32_t *px,
                  const uint32_t *py,
                  const uint32_t *secret,
                  uint32_t       *resultx,
                  uint32_t       *resulty );

static inline void ecc_ecdh( const uint32_t *px,
                             const uint32_t *py,
                             const uint32_t *secret,
                             uint32_t       *resultx,
                             uint32_t       *resulty )
{
    ecc_ec_mult( px, py, secret, resultx, resulty );
}

int ecc_ecdsa_validate( const uint32_t *x,
                        const uint32_t *y,
                        const uint32_t *e,
                        const uint32_t *r,
                        const uint32_t *s );

int ecc_ecdsa_sign( const uint32_t *d,
                    const uint32_t *e,
                    const uint32_t *k,
                    uint32_t       *r,
                    uint32_t       *s );

int ecc_is_valid_key( const uint32_t * priv_key );

static inline void ecc_gen_pub_key( const uint32_t *priv_key,
                                    uint32_t       *pub_x,
                                    uint32_t       *pub_y )
{
    ecc_ec_mult( ecc_g_point_x, ecc_g_point_y, priv_key, pub_x, pub_y );
}

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_ECC_H_ */