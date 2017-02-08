/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef NBIOT_SOURCE_STRUCT_H_
#define NBIOT_SOURCE_STRUCT_H_

#include "lwm2m.h"
#include <nbiot.h>

struct nbiot_device_t
{
    lwm2m_context_t  *lwm2m;
    lwm2m_object_t  **obj_arr;
    size_t            obj_num;
    size_t            obj_max;
    nbiot_socket_t   *sock;
    nbiot_sockaddr_t *addr;
    connection_t     *connlist;
};

/**
 * 查找lwm2m_object_t
 * @param dev 指向nbiot_device_t内存
 *        obj_id object id
 * @return 成功返回lwm2m_object_t指针，否则返回NULL
**/
lwm2m_object_t*
nbiot_object_find( nbiot_device_t *dev,
                   uint16_t        obj_id );

/**
 * 添加lwm2m_object_t
 * @param dev 指向nbiot_device_t内存
 *        obj 指向lwm2m_object_t内存
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_object_add( nbiot_device_t *dev,
                      lwm2m_object_t *obj );

/**
 * 移除lwm2m_object_t
 * @param dev 指向nbiot_device_t内存
 *        obj 指向lwm2m_object_t内存
**/
void nbiot_object_del( nbiot_device_t *dev,
                       lwm2m_object_t *obj );

#endif /* NBIOT_SOURCE_STRUCT_H_ */