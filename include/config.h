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
#define NBIOT_SOCK_RECV_BUF_SIZE        512

/**
 * @def NBIOT_DEBUG
 *
 * 调试模式标记
 * 打开后，会增加调试相关的代码
 */
/* #define NBIOT_DEBUG                     1 */

/**
 * @def NBIOT_DEVICE_MANUFACTURER
 *
 * 设备厂商
**/
#define NBIOT_DEVICE_MANUFACTURER       "Open Mobile Alliance"

/**
 * @def NBIOT_DEVICE_MODEL_NUMBER
 *
 * 设备型号
**/
#define NBIOT_DEVICE_MODEL_NUMBER       "Lightweight M2M Client"

/**
 * @def NBIOT_DEVICE_FIRMWARE_VERSION
 *
 * 设备固件版本
**/
#define NBIOT_DEVICE_FIRMWARE_VERSION   "1.0"

#endif /* NBIOT_INCLUDE_CONFIG_H_ */
