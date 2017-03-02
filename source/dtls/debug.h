/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_DEBUG_H_
#define NBIOT_SOURCE_DTLS_DEBUG_H_

#include <utils.h>
#include "session.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DTLS_WITH_LOGS
#define dsrv_log(...) nbiot_printf(__VA_ARGS__)
void dsrv_hexdump_log( const char *name,
                       const unsigned char *buf,
                       size_t               length,
                       int                  extend );
#else
#define dsrv_log(...)
#define dsrv_hexdump_log(...)
#endif

#define dtls_emerg(...)                       dsrv_log(__VA_ARGS__)
#define dtls_alert(...)                       dsrv_log(__VA_ARGS__)
#define dtls_crit(...)                        dsrv_log(__VA_ARGS__)
#define dtls_warn(...)                        dsrv_log(__VA_ARGS__)
#define dtls_notice(...)                      dsrv_log(__VA_ARGS__)
#define dtls_info(...)                        dsrv_log(__VA_ARGS__)
#define dtls_debug(...)                       dsrv_log(__VA_ARGS__)
#define dtls_debug_hexdump(name, buf, length) dsrv_hexdump_log(name, buf, length, 1)
#define dtls_debug_dump(name, buf, length)    dsrv_hexdump_log(name, buf, length, 0)

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_DEBUG_H_ */