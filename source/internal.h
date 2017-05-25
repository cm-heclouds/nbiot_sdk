/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef ONENET_INTERNAL_H_
#define ONENET_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "coap.h"
#include <utils.h>
#include <nbiot.h>

/**
 * coap.h
**/
static inline
int nbiot_token( const uint8_t *buffer,
                 uint8_t       *token )
{
    const uint8_t *tmp;
    uint8_t token_len;

    token_len = coap_token( buffer, &tmp );
    if ( token )
    {
        nbiot_memmove( token, tmp, token_len );
    }

    return token_len;
}

static inline
int nbiot_uri_query( const uint8_t *buffer,
                     size_t         buffer_len,
                     char         **uri_query,
                     uint16_t      *uri_query_len,
                     bool           first )
{
    int ret;
    const char *str;

    ret = coap_uri_query( buffer,
                          buffer_len,
                          &str,
                          uri_query_len,
                          first );
    if ( ret )
    {
        *uri_query = nbiot_strdup( str, *uri_query_len );
    }

    return ret;
}

static inline
int nbiot_payload( const uint8_t *buffer,
                   size_t         buffer_len,
                   uint8_t      **payload,
                   uint16_t      *payload_len )
{
    int ret;
    const uint8_t *source;

    ret = coap_payload( buffer,
                        buffer_len,
                        &source,
                        payload_len );
    if ( ret )
    {
        *payload = nbiot_strdup( (char*)source, *payload_len );
    }

    return ret;
}

typedef struct _nbiot_list_t
{
    struct _nbiot_list_t *next;
    uint16_t              id;
} nbiot_list_t;

/**
 * list.c
**/
uint16_t nbiot_list_newid( nbiot_list_t *list );
nbiot_list_t* nbiot_list_add( nbiot_list_t *list,
                              nbiot_list_t *node );
nbiot_list_t* nbiot_list_get( nbiot_list_t *list,
                              uint16_t      id );
nbiot_list_t* nbiot_list_del( nbiot_list_t  *list,
                              uint16_t       id,
                              nbiot_list_t **node );
void nbiot_list_free( nbiot_list_t *list );

#define NBIOT_LIST_ADD(l,n)   nbiot_list_add((nbiot_list_t*)(l),(nbiot_list_t*)(n))
#define NBIOT_LIST_GET(l,i)   nbiot_list_get((nbiot_list_t*)(l),(i))
#define NBIOT_LIST_DEL(l,i,n) nbiot_list_del((nbiot_list_t*)(l),(i),(nbiot_list_t**)(n))
#define NBIOT_LIST_FREE(l)    nbiot_list_free((nbiot_list_t*)(l))

/* LWM2M content format */
#define LWM2M_CONTENT_TEXT     0
#define LWM2M_CONTENT_LINK     40
#define LWM2M_CONTENT_TLV_OLD  1542
#define LWM2M_CONTENT_TLV      11542

/* TLV Type */
#define TLV_TYPE_UNKNOWN           (uint8_t)0xff
#define TLV_TYPE_OBJECT            (uint8_t)0x10
#define TLV_TYPE_OBJECT_INSTANCE   (uint8_t)0x00
#define TLV_TYPE_RESOURCE          (uint8_t)0xc0
#define TLV_TYPE_MULTIPLE_RESOURCE (uint8_t)0x80
#define TLV_TYPE_RESOURCE_INSTANCE (uint8_t)0x40

/**
 * tlv.c
**/
int nbiot_tlv_length( uint16_t id, size_t len );
int nbiot_tlv_decode( const uint8_t  *buffer,
                      size_t          buffer_len,
                      uint8_t        *type,
                      uint16_t       *id,
                      const uint8_t **value,
                      size_t         *value_len );
int nbiot_tlv_encode( uint8_t       *buffer,
                      size_t         buffer_len,
                      uint8_t        type,
                      uint16_t       id,
                      const uint8_t *value,
                      size_t         value_len );

/**
 * resource uri flag
**/
#define NBIOT_SET_OBJID     0x1
#define NBIOT_SET_INSTID    0x2
#define NBIOT_SET_RESID     0x4

/**
 * resource uri
**/
typedef struct _nbiot_uri_t
{
    uint16_t objid;
    uint16_t instid;
    uint16_t resid;
    uint8_t  flag;
} nbiot_uri_t;

/**
 * uri.c
**/
int nbiot_uri_decode( nbiot_uri_t   *uri,
                      const uint8_t *buffer,
                      size_t         buffer_len );

/**
 * utils.c
**/
int nbiot_init_token( uint8_t *token,
                      uint8_t  token_len,
                      uint16_t mid );
int nbiot_add_string( const char *src,
                      char       *dest,
                      size_t      size );
int nbiot_add_integer( int64_t src,
                       char   *dest,
                       size_t  size );
int nbiot_send_buffer( nbiot_socket_t         *socket,
                       const nbiot_sockaddr_t *address,
                       const uint8_t          *buffer,
                       size_t                  buffer_len );
int nbiot_recv_buffer( nbiot_socket_t    *socket,
                       nbiot_sockaddr_t **address,
                       uint8_t           *buffer,
                       size_t             buffer_len );

typedef enum
{
    STATE_DEREGISTERED = 0,   /* not registered or boostrap not started */
    STATE_REG_PENDING,        /* registration pending */
    STATE_REGISTERED,         /* successfully registered */
    STATE_REG_FAILED,         /* last registration failed */
    STATE_REG_UPDATE_PENDING, /* registration update pending */
    STATE_REG_UPDATE_NEEDED,  /* registration update required */
    STATE_DEREG_PENDING,      /* deregistration pending */
    STATE_SERVER_RESET           /* server reset */
} nbiot_status_t;

#define STATE_ERROR(x) \
        ((x)->state == STATE_DEREGISTERED ? NBIOT_ERR_NOT_REGISTER : \
        ((x)->state == STATE_REGISTERED ? NBIOT_ERR_OK : \
        ((x)->state == STATE_REG_UPDATE_PENDING ? NBIOT_ERR_OK : \
        ((x)->state == STATE_REG_UPDATE_NEEDED ? NBIOT_ERR_OK : \
        ((x)->state == STATE_REG_FAILED ? NBIOT_ERR_REG_FAILED : \
        ((x)->state == STATE_SERVER_RESET ? NBIOT_ERR_SERVER_RESET : NBIOT_ERR_INTERNAL))))))

typedef struct _nbiot_node_t
{
    struct _nbiot_node_t *next;
    uint16_t              id;
    void                 *data;
} nbiot_node_t;

#ifdef LOCATION_MALLOC
typedef struct _nbiot_location_t
{
    struct _nbiot_location_t *next;
    uint8_t                   size;
    uint8_t                   name[1];
} nbiot_location_t;
#define LOCATION_SIZE(x) (sizeof(nbiot_location_t)+(x)-1)
#endif

typedef struct _nbiot_observe_t
{
    struct _nbiot_observe_t *next;
    uint16_t                 id;
    uint16_t                 lastmid;
    uint32_t                 counter;
    time_t                   lasttime;
    struct _nbiot_observe_t *list;
    uint8_t                  token[8];
    uint8_t                  token_len;
    bool                     active;
} nbiot_observe_t;

typedef void(*nbiot_transaction_callback_t)(nbiot_device_t*,const uint8_t*,size_t);
typedef struct _nbiot_transaction_t
{
    struct _nbiot_transaction_t *next;
    uint16_t                     mid;
    uint8_t                      ack;
    uint8_t                      counter;
    time_t                       timeout;
    time_t                       time;
    uint8_t                     *buffer;
    size_t                       buffer_len;
    nbiot_transaction_callback_t callback;
} nbiot_transaction_t;

struct _nbiot_device_t
{
    uint16_t                 next_mid;
    uint8_t                  state;
    int                      life_time;
    time_t                   registraction;
    const char              *endpoint_name;
    nbiot_sockaddr_t        *server;        /* server address */
    nbiot_socket_t          *socket;        /* client socket */
    nbiot_sockaddr_t        *address;       /* temporary address */
    nbiot_node_t            *nodes;
    nbiot_observe_t         *observes;
#ifdef LOCATION_MALLOC
    nbiot_location_t        *locations;
#endif
    nbiot_transaction_t     *transactions;
    nbiot_write_callback_t   write_func;
    nbiot_execute_callback_t execute_func;
};

/**
 * node.c
**/
int nbiot_node_read( nbiot_node_t *node,
                     uint8_t       flag,
                     uint8_t      *buffer,
                     size_t        buffer_len,
                     bool          updated );
int nbiot_node_write( nbiot_node_t          *node,
                      const nbiot_uri_t     *uri,
                      const uint8_t         *buffer,
                      size_t                 buffer_len,
                      nbiot_write_callback_t write_func );
int nbiot_node_discover( nbiot_node_t      *node,
                         const nbiot_uri_t *uri,
                         uint8_t           *buffer,
                         size_t             buffer_len,
                         bool               first );
nbiot_node_t* nbiot_node_find( nbiot_device_t    *dev,
                               const nbiot_uri_t *uri );

/**
 * registraction.c
**/
int nbiot_register_start( nbiot_device_t *dev,
                          uint8_t        *buffer,
                          size_t          buffer_len );
int nbiot_register_update( nbiot_device_t *dev,
                           uint8_t        *buffer,
                           size_t          buffer_len,
                           bool            with_objs );
int nbiot_deregister( nbiot_device_t *dev,
                      uint8_t        *buffer,
                      size_t          buffer_len );
void nbiot_register_step( nbiot_device_t *dev,
                          time_t          now,
                          uint8_t        *buffer,
                          size_t          buffer_len );

/**
 * transaction.c
**/
int nbiot_transaction_add( nbiot_device_t              *dev,
                           const uint8_t               *buffer,
                           size_t                       buffer_len,
                           nbiot_transaction_callback_t callback );
int nbiot_transaction_del( nbiot_device_t *dev,
                           uint16_t        mid,
                           const uint8_t  *buffer,
                           size_t          buffer_len );
void nbiot_transaction_step( nbiot_device_t *dev,
                             time_t          now,
                             uint8_t        *buffer,
                             size_t          buffer_len );

/**
 * observe.c
**/
nbiot_observe_t* nbiot_observe_add( nbiot_device_t    *dev,
                                    const nbiot_uri_t *uri,
                                    const uint8_t     *token,
                                    uint8_t            token_len );
int nbiot_observe_del( nbiot_device_t    *dev,
                       const nbiot_uri_t *uri );
void nbiot_observe_step( nbiot_device_t *dev,
                         time_t          now,
                         uint8_t        *buffer,
                         size_t          buffer_len );

/**
 * nbiot.c
**/

#ifdef __cplusplus
} /* extern "C"{ */
#endif

#endif /* ONENET_INTERNAL_H_ */