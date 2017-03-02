/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_CCM_H_
#define NBIOT_SOURCE_DTLS_CCM_H_

#include "aes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* implementation of Counter Mode CBC-MAC, RFC 3610 */
#define DTLS_CCM_BLOCKSIZE  16 /**< size of hmac blocks */
#define DTLS_CCM_MAX        16 /**< max number of bytes in digest */
#define DTLS_CCM_NONCE_SIZE 12 /**< size of nonce */

/** 
 * Authenticates and encrypts a message using AES in CCM mode. Please
 * see also RFC 3610 for the meaning of \p M, \p L, \p lm and \p la.
 * 
 * \param ctx The initialized rijndael_ctx object to be used for AES operations.
 * \param M   The number of authentication octets.
 * \param L   The number of bytes used to encode the message length.
 * \param N   The nonce value to use. You must provide \c DTLS_CCM_BLOCKSIZE 
 *            nonce octets, although only the first \c 16 - \p L are used.
 * \param msg The message to encrypt. The first \p la octets are additional
 *            authentication data that will be cleartext. Note that the 
 *            encryption operation modifies the contents of \p msg and adds 
 *            \p M bytes MAC. Therefore, the buffer must be at least
 *            \p lm + \p M bytes large.
 * \param lm  The actual length of \p msg.
 * \param aad A pointer to the additional authentication data (can be \c NULL if
 *            \p la is zero).
 * \param la  The number of additional authentication octets (may be zero).
 * \return FIXME
 */
long int dtls_ccm_encrypt_message( rijndael_ctx        *ctx,
                                   size_t               M,
                                   size_t               L,
                                   unsigned char        nonce[DTLS_CCM_BLOCKSIZE],
                                   unsigned char       *msg,
                                   size_t               lm,
                                   const unsigned char *aad,
                                   size_t               la );

long int dtls_ccm_decrypt_message( rijndael_ctx        *ctx,
                                   size_t               M,
                                   size_t               L,
                                   unsigned char        nonce[DTLS_CCM_BLOCKSIZE],
                                   unsigned char       *msg,
                                   size_t               lm,
                                   const unsigned char *aad,
                                   size_t               la );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_CCM_H_ */
