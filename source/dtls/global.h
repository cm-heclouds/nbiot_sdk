/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_GLOBAL_H_
#define NBIOT_SOURCE_DTLS_GLOBAL_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum size of DTLS message.
    When Peers are sending bigger messages this causes problems. Californium
    with ECDSA needs at least 220 */
#define DTLS_MAX_BUF                512
/** Number of message retransmissions. */
#define DTLS_DEFAULT_MAX_RETRANSMIT 7

/* Define our own types as at least uint32_t does not work on my amd64. */
typedef unsigned char uint8;
typedef unsigned char uint16[2];
typedef unsigned char uint24[3];
typedef unsigned char uint32[4];
typedef unsigned char uint48[6];

/** Known cipher suites.*/
typedef enum
{
    TLS_NULL_WITH_NULL_NULL            = 0x0000, /**< NULL cipher  */
    TLS_PSK_WITH_AES_128_CCM_8         = 0xC0A8, /**< see RFC 6655 */
    TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 = 0xC0AE  /**< see RFC 7251 */
} dtls_cipher_t;

/** Known compression suites.*/
typedef enum
{
    TLS_COMPRESSION_NULL               = 0x0000   /* NULL compression */
} dtls_compression_t;

#define TLS_EXT_ELLIPTIC_CURVES                10 /* see RFC 4492 */
#define TLS_EXT_EC_POINT_FORMATS               11 /* see RFC 4492 */
#define TLS_EXT_SIG_HASH_ALGO                  13 /* see RFC 5246 */
#define TLS_EXT_CLIENT_CERTIFICATE_TYPE        19 /* see RFC 7250 */
#define TLS_EXT_SERVER_CERTIFICATE_TYPE        20 /* see RFC 7250 */
#define TLS_EXT_ENCRYPT_THEN_MAC               22 /* see RFC 7366 */

#define TLS_CERT_TYPE_RAW_PUBLIC_KEY           2  /* see RFC 7250 */

#define TLS_EXT_ELLIPTIC_CURVES_SECP256R1      23 /* see RFC 4492 */

#define TLS_EXT_EC_POINT_FORMATS_UNCOMPRESSED  0  /* see RFC 4492 */

#define TLS_EC_CURVE_TYPE_NAMED_CURVE          3  /* see RFC 4492 */

#define TLS_CLIENT_CERTIFICATE_TYPE_ECDSA_SIGN 64 /* see RFC 4492 */

#define TLS_EXT_SIG_HASH_ALGO_SHA256           4  /* see RFC 5246 */
#define TLS_EXT_SIG_HASH_ALGO_ECDSA            3  /* see RFC 5246 */

/** 
* XORs \p n bytes byte-by-byte starting at \p y to the memory area
* starting at \p x. */
static inline void memxor( unsigned char       *x,
                           const unsigned char *y,
                           size_t               n )
{
    while ( n-- )
    {
        *x ^= *y;
        x++; y++;
    }
}

/**
 * Compares \p len bytes from @p a with @p b in constant time. This
 * functions always traverses the entire length to prevent timing
 * attacks.
 *
 * \param a Byte sequence to compare
 * \param b Byte sequence to compare
 * \param len Number of bytes to compare.
 * \return \c 1 if \p a and \p b are equal, \c 0 otherwise.
 */
static inline int equals( unsigned char *a,
                          unsigned char *b,
                          size_t         len)
{
    int result = 1;
    while ( len-- )
    {
        result &= (*a++ == *b++);
    }
    return result;
}

static inline int dtls_fls( unsigned int i )
{
    int n;
    for ( n = 0; i; n++ )
        i >>= 1;
    return n;
}

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_GLOBAL_H_ */