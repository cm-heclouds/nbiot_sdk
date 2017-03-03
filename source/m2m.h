/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef NBIOT_SOURCE_M2M_H_
#define NBIOT_SOURCE_M2M_H_

#include <utils.h>
#include <nbiot.h>
#include <lwm2m.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * connection定义
**/
typedef struct connection_t
{
    struct connection_t *next;
    nbiot_sockaddr_t    *addr;
}connection_t;

/**
 * 创建连接，然后加入连接list中
 * @param connlist 连接list
 *        addr 目标地址
 *        port 目标端口
 * @return 成功返回连接句柄，否则返回NULL
**/
connection_t* connection_create( connection_t   *connlist,
                                 nbiot_socket_t *sock,
                                 const char     *addr,
                                 uint16_t        port );

/**
 * 查找连接
 * @param connlist 连接list
 *        addr 指向socket地址信息的内存
 * @return 成功返回连接句柄，否则返回NULL
**/
connection_t* connection_find( connection_t           *connlist,
                               const nbiot_sockaddr_t *addr );

/**
 * 移除连接
 * @param connlist 连接list
 *        conn 连接句柄
 * @return 返回连接list
**/
connection_t* connection_remove( connection_t *connlist,
                                 connection_t *conn );

/**
 * 销毁连接list
 * @param connlist 连接list
**/
void connection_destroy( connection_t *connlist );

/**
 * 创建security object
 * @param svr_id  服务器id
 *        svr_uri 服务器链接地址(例如coap://127.0.0.1:5683)
 *                在运行过程中必须有效
 *        holdoff_time client关闭时间
 *        bootstrap 是否为bootstrap标记
 * @return 成功返回security object指针，否则返回NULL
**/
lwm2m_object_t* create_security_object( uint16_t    svr_id,
                                        const char *svr_uri,
                                        uint32_t    holdoff_time,
                                        bool        bootstrap,
                                        bool        svr_uri_free );

/**
 * 清理security object
 * @param sec_obj 指向security object内存
**/
void clear_security_object( lwm2m_object_t *sec_obj );

/**
 * 获取服务的coap链接
 * @param sec_obj 指向security object内存
 *        sec_instid security instance id
 * @return coap连接字符串，使用完需用lwm2m_free释放内存，失败返回NULL
**/
const char* get_server_uri( lwm2m_object_t *sec_obj,
                            uint16_t        sec_instid );

/**
 * 创建server object
 * @param svr_id 服务器id
 *        binding 传输形式（默认U）
 *        lifetime 生存时间（秒）
 *        storing 是否保存Notify
 * @return 成功返回server object指针，否则返回NULL
**/
lwm2m_object_t* create_server_object( uint16_t    svr_id,
                                      const char *binding,
                                      uint32_t    lifetime,
                                      bool        storing );

/**
 * 清理server object
 * @param svr_obj 指向server object内存
**/
void clear_server_object( lwm2m_object_t *svr_obj );

/**
 * 创建device object
 * @param serial_number 序列号,在运行过程中必须有效
 * @return 成功返回device object指针，否则返回NULL
**/
lwm2m_object_t* create_device_object( const char *serial_number );

/**
 * 清理device object
 * @param dev_obj 指向device object内存
**/
void clear_device_object( lwm2m_object_t *dev_obj );

/**
 * 添加resource object
 * @param obj 指向lwm2m_object_t内存
 *        inst_id object instance id
 *        res_id  resource id
 *        type    指向resource类型的内存
 *        value   指向resource值的内存
 * @return 成功返回NBIOT_ERR_OK
**/
int create_resource_object( lwm2m_object_t   *res_obj,
                            nbiot_resource_t *resource );

/**
 * 判定resource是否存在
**/
bool check_resource_object( lwm2m_object_t *res_obj,
                            uint16_t        inst_id,
                            uint16_t        res_id );

/**
 * 清理resource object
 * @param obj 指向lwm2m_object_t内存
**/
void clear_resource_object( lwm2m_object_t *res_obj );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_M2M_H_ */