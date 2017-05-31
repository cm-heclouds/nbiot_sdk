/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef ONENET_NBIOT_H_
#define ONENET_NBIOT_H_

#include "error.h"
#include "config.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * value type
**/
#define NBIOT_UNKNOWN       0x0
#define NBIOT_BOOLEAN       0x1
#define NBIOT_INTEGER       0x2
#define NBIOT_FLOAT         0x3
#define NBIOT_STRING        0x4
#define NBIOT_BINARY        0x5

/**
 * value flag
**/
#define NBIOT_READABLE      0x1
#define NBIOT_WRITABLE      0x2
#define NBIOT_EXECUTABLE    0x4
#define NBIOT_UPDATED       0x8

/**
 * value定义
**/
typedef struct _nbiot_value_t
{
    union
    {
        bool    as_bool;
        int64_t as_int;
        double  as_float;
        struct
        {
            char  *val;
            size_t len;
        } as_buf;
    } value;

    uint8_t type;
    uint8_t flag;
} nbiot_value_t;

/**
 * device声明
**/
typedef struct _nbiot_device_t nbiot_device_t;

/**
 * write回调函数(write后调用)
 * @param uri  资源路径信息
 *        data 资源数据
**/
typedef void(*nbiot_write_callback_t)(uint16_t       objid,
                                      uint16_t       instid,
                                      uint16_t       resid,
                                      nbiot_value_t *data);

/**
 * execute回调函数
 * @param uri  资源路径信息
 *        data 资源数据
 *        buff 指向执行数据缓存
 *        size 执行数据缓存大小
**/
typedef void(*nbiot_execute_callback_t)(uint16_t       objid,
                                        uint16_t       instid,
                                        uint16_t       resid,
                                        nbiot_value_t *data,
                                        const void    *buff,
                                        size_t         size);

/**
 * 创建OneNET接入设备实例
 * @param dev           [OUT] 设备实例
 *        endpoint_name 终端名称（imei;imsi）
 *        life_time     存活时长（秒）
 *        local_port    本地UDP绑定端口
 *        write_func    写回调函数
 *        execute_func  执行回调函数
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_create( nbiot_device_t         **dev,
                         const char              *endpoint_name,
                         int                      life_time,
                         uint16_t                 local_port,
                         nbiot_write_callback_t   write_func,
                         nbiot_execute_callback_t execute_func );

/**
 * 销毁OneNET接入设备实例
 * @param dev 设备实例
**/
void nbiot_device_destroy( nbiot_device_t *dev );

/**
 * 连接OneNET服务
 * @param dev        设备实例
 *        server_uri 服务链接地址（例如coap://127.0.0.1:5683）
 *        timeout    超时时长（秒）
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_connect( nbiot_device_t *dev,
                          const char     *server_uri,
                          int             timeout );

/**
* 关闭与OneNET服务的连接
* @param dev 设备实例
**/
void nbiot_device_close( nbiot_device_t *dev,
                         int             timeout );

/**
 * 驱动设备的数据收发和逻辑处理
 * @param dev     设备实例
 *        timeout 超时时长（秒）
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_step( nbiot_device_t *dev,
                       int             timeout );

/**
 * 设备资源添加（只修改状态，未通知server；可用于资源更新）
 * @param dev  设备实例
 *        uri  资源地址信息
 *        data 资源数据
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_resource_add( nbiot_device_t *dev,
                        uint16_t        objid,
                        uint16_t        instid,
                        uint16_t        resid,
                        nbiot_value_t  *data );

/**
 * 设备资源删除（只修改状态，未通知server）
 * @param dev  设备实例
 *        uri  资源地址信息
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_resource_del( nbiot_device_t *dev,
                        uint16_t        objid,
                        uint16_t        instid,
                        uint16_t        resid );

/**
 * 主动同步设备信息（资源改变等）
 * @param dev     设备实例
**/
void nbiot_device_sync( nbiot_device_t *dev );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* ONENET_NBIOT_H_ */
