/**
* Copyright (c) 2017 China Mobile IOT.
* All rights reserved.
**/

#ifndef NBIOT_INCLUDE_CONFIG_H_
#define NBIOT_INCLUDE_CONFIG_H_

/**
 * @def NBIOT_SOCK_RECV_BUF_SIZE
 *
 * 接收数据缓存大小
**/
#ifdef HAVE_DTLS
#define NBIOT_SOCK_RECV_BUF_SIZE        512
#else
#define NBIOT_SOCK_RECV_BUF_SIZE        128
#endif

/**
 * @def NBIOT_DEBUG
 *
 * 调试模式标记
 * 打开后，会增加调试相关的代码
 */
/* #define NBIOT_DEBUG                     1 */

#endif /* NBIOT_INCLUDE_CONFIG_H_ */
