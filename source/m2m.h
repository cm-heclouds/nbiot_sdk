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
 *        addr     目标地址
 *        port     目标端口
 * @return 成功返回连接句柄，否则返回NULL
**/
connection_t* connection_create( connection_t   *connlist,
                                 nbiot_socket_t *sock,
                                 const char     *addr,
                                 uint16_t        port );

/**
 * 查找连接
 * @param connlist 连接list
 *        addr     指向socket地址信息的内存
 * @return 成功返回连接句柄，否则返回NULL
**/
connection_t* connection_find( connection_t           *connlist,
                               const nbiot_sockaddr_t *addr );

/**
 * 移除连接
 * @param connlist 连接list
 *        conn     连接句柄
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
 * 添加resource object
 * @param obj  指向lwm2m_object_t内存
 *        data 指向nbiot_resource_t内存
 * @return 成功返回NBIOT_ERR_OK
**/
int create_resource_object( lwm2m_object_t   *obj,
                            nbiot_resource_t *data );

/**
 * 判定resource是否存在
 * @param obj    指向lwm2m_object_t内存
 *        instid object instance id
 *        resid  resource id
 * @return 存在返回true，否则返回false
**/
bool check_resource_object( lwm2m_object_t *obj,
                            uint16_t        instid,
                            uint16_t        resid );

/**
 * 清理resource object
 * @param obj 指向lwm2m_object_t内存
**/
void clear_resource_object( lwm2m_object_t *obj );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_M2M_H_ */