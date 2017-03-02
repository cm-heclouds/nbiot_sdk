/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_SESSION_H_
#define NBIOT_SOURCE_DTLS_SESSION_H_

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef nbiot_sockaddr_t session_t;

/**
* Compares the given session objects. This function returns @c 0
* when @p a and @p b differ, @c 1 otherwise.
*/
static inline int dtls_session_equals( const session_t *a,
                                       const session_t *b )
{
    return nbiot_sockaddr_equal( a, b );
}

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_SESSION_H_ */