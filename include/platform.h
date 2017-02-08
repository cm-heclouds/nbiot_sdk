/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef NBIOT_INClUDE_PLATFORM_H_
#define NBIOT_INClUDE_PLATFORM_H_

#if defined(NBIOT_OS_WIN) || defined(NBIOT_OS_POSIX)
#include <stddef.h>  /* for size_t */
#include <stdint.h>  /* for integer types */
#include <stdbool.h> /* for bool */
#include <time.h>    /* for time_t */
#endif

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化环境
**/
void nbiot_init_environment( void );

/**
 * 清理环境
**/
void nbiot_clear_environment( void );

/**
 * 分配内存
 * @param size 需要分配的内存字节数
 * @return 分配成功返回分配内存指针，否则返回NULL
**/
void *nbiot_malloc( size_t size );

/**
 * 释放由nbiot_malloc分配的内存
 * @param ptr 指向将要被释放的内存
**/
void nbiot_free( void *ptr );

/**
 * 获取当前时间
 * @return 返回当前距(00:00:00 UTC, January 1, 1970)的秒数
**/
time_t nbiot_time( void );

/**
 * 休眠
 * @param milliseconds 休眠时长
**/
void nbiot_sleep( int milliseconds );

/**
 * socket句柄，移植时自行定义其具体属性
**/
typedef struct nbiot_socket_t nbiot_socket_t;
typedef struct nbiot_sockaddr_t nbiot_sockaddr_t;

/**
 * 创建UDP socket句柄，必须将其设置为非阻塞模式
 * @param sock 指向保持UDP socket句柄内存指针的内存
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_udp_create( nbiot_socket_t **sock );

/**
 * 关闭UDP socket句柄
 * @param sock UDP 指向socket句柄的内存
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_udp_close( nbiot_socket_t *sock );

/**
 * UDP socket句柄绑定地址和端口
 * @param sock 指向保持UDP socket句柄内存指针的内存
 *        addr 绑定地址
 *        port 绑定端口
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_udp_bind( nbiot_socket_t *sock,
                    const char     *addr,
                    uint16_t        port );

/**
 * 连接UDP服务
 * @param sock 指向保持UDP socket句柄内存指针的内存
 *        addr 目标地址
 *        port 目标端口
 *        dest [OUT] 指向目标地址信息的内存指针的内存
 * @return 成功返回NBIOT_ERR_OK
**/
int nbiot_udp_connect( nbiot_socket_t    *sock,
                       const char        *addr,
                       uint16_t           port,
                       nbiot_sockaddr_t **dest );

/**
 * 发送数据
 * @param sock 指向UDP socket句柄的内存
 *        buff 指向将要被发送的数据
 *        size 将要被发送的数据字节数
 *        sent 实际发送成功的字节数
 *        dest 指向目标地址信息的内存
 * @return 发送数据正常返回NBIOT_ERR_OK
**/
int nbiot_udp_send( nbiot_socket_t         *sock,
                    const void             *buff,
                    size_t                  size,
                    size_t                 *sent,
                    const nbiot_sockaddr_t *dest );

/**
 * 接收数据
 * @param sock 指向UDP socket句柄的内存
 *        buff 指向存储接收数据的缓冲区
 *        size 接收数据缓存区的最大字节数
 *        read [OUT] 实际接收到的数据字节数
 *        src  [OUT] 指向源地址信息的内存,
 *                   如果接收成功，src=NULL时，自动创建nbiot_sockaddr_t结构
 * @return 接收数据正常返回NBIOT_ERR_OK
**/
int nbiot_udp_recv( nbiot_socket_t    *sock,
                    void              *buff,
                    size_t             size,
                    size_t            *read,
                    nbiot_sockaddr_t **src );

/**
 * 比较2个socket地址信息是否一致
 * @return 一致返回true，否则返回false
**/
bool nbiot_sockaddr_equal( const nbiot_sockaddr_t *addr1,
                           const nbiot_sockaddr_t *addr2 );

/**
 * 销毁nbiot_sockaddr_t
 * @param addr 指向nbiot_sockaddr_t的内存
**/
void nbiot_sockaddr_destroy( nbiot_sockaddr_t *addr );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_INClUDE_PLATFORM_H_ */