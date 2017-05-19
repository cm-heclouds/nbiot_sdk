/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef NBIOT_INCLUDE_NBIOT_H_
#define NBIOT_INCLUDE_NBIOT_H_

#include "error.h"
#include "config.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * value类型
**/
#define NBIOT_VALUE_UNKNOWN       0x0
#define NBIOT_VALUE_BOOLEAN       0x1
#define NBIOT_VALUE_INTEGER       0x2
#define NBIOT_VALUE_FLOAT         0x3
#define NBIOT_VALUE_STRING        0x4
#define NBIOT_VALUE_BINARY        0x5

/**
 * resource标记
**/
#define NBIOT_RESOURCE_READABLE   0x1
#define NBIOT_RESOURCE_WRITABLE   0x2
#define NBIOT_RESOURCE_EXECUTABLE 0x4

/**
 * value定义
**/
typedef union nbiot_value_t
{
    bool     as_bool;
    int64_t  as_int;
    double   as_float;

    /* string */
    struct
    {
        char   *str;
        size_t  len;
    } as_str;

    /* binary */
    struct
    {
        uint8_t *bin;
        size_t   len;
    } as_bin;
} nbiot_value_t;

/**
 * resource声明
**/
typedef struct nbiot_resource_t nbiot_resource_t;

/**
 * write回调函数（write成功后执行）
 * @param res 指向nbiot_resource_t内存
**/
typedef void(*nbiot_write_callback_t)(nbiot_resource_t *res);

/**
 * execute回调函数（收到execute后执行）
 * @param res    指向nbiot_resource_t内存
 *        buffer 指向接收到的数据缓存
 *        length 数据总字节数
**/
typedef void(*nbiot_execute_callback_t)(nbiot_resource_t *res,
                                        const uint8_t    *buffer,
                                        int               length);

/**
 * resource定义
**/
struct nbiot_resource_t
{
    uint16_t                 objid;
    uint16_t                 instid;
    uint16_t                 resid;
    uint8_t                  flag;
    uint8_t                  type;
    nbiot_value_t            value;
    nbiot_write_callback_t   write;
    nbiot_execute_callback_t execute;
};

/**
 * device声明
**/
typedef struct nbiot_device_t nbiot_device_t;

/**
 * 创建OneNET接入设备实例
 * @param dev           [OUT] 指向nbiot_device_t指针的内存
 *        port          本地UDP绑定端口
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_create( nbiot_device_t **dev,
                         uint16_t         local_port );

/**
 * 销毁OneNET接入设备实例
 * @param dev 指向nbiot_devie_t的内存
**/
void nbiot_device_destroy( nbiot_device_t *dev );

/**
 * 连接OneNET服务
 * @param dev 指向nbiot_device_t的内存
 *        server_uri 服务链接地址（例如coap://127.0.0.1:5683）
 *        life_time  保活时间（秒）
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_connect( nbiot_device_t *dev,
                          const char     *server_uri,
                          time_t          life_time );

/**
 * 关闭与OneNET服务的连接
 * @param dev 指向nbiot_device_t的内存
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_close( nbiot_device_t *dev );

/**
 * 配置设备
 * @param dev 指向nbiot_device_t的内存
 *        endpoint_name device名称("imei;imsi")
 *        auth_code     device鉴权码（OneNET自动生成）
 *        res_array     resources指针数组
 *        res_num       resources总数
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_configure( nbiot_device_t   *dev,
                            const char       *endpoint_name,
                            nbiot_resource_t *res_array[],
                            size_t            res_num );

/**
 * 设备与OneNET服务的连接是否就绪
 * @param dev 指向nbiot_device_t的内存
 * @return 就绪返回true，否则返回false
**/
bool nbiot_device_ready( nbiot_device_t *dev );

/**
 * 数据驱动以及设备保活
 * @param dev     指向nbiot_device_t的内存
 *        timeout 执行超时时间（秒）
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_step( nbiot_device_t *dev,
                       time_t          timeout );

/**
 * 主动上报资源数据
 * @param dev    指向nbiot_device_t的内存
 *        objid  object id（对象）
 *        instid object instance id（对象实例）
 *        resid  resource id（属性）
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_device_notify( nbiot_device_t *dev,
                         uint16_t        objid,
                         uint16_t        instid,
                         uint16_t        resid );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_INCLUDE_NBIOT_H_ */
