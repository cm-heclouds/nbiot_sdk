/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <nbiot.h>
#include "struct.h"

/* 全局唯一实例 */
static nbiot_device_t g_dev;

static inline lwm2m_object_t* nbiot_object_find( nbiot_device_t *dev,
                                                 uint16_t        objid )
{
    if ( NULL == dev )
    {
        return NULL;
    }

    return (lwm2m_object_t*)LWM2M_LIST_FIND( dev->objlist, objid );
}

static inline int nbiot_object_add( nbiot_device_t *dev,
                                    lwm2m_object_t *obj )
{
    if ( NULL == dev ||
         NULL == obj )
    {
        return NBIOT_ERR_BADPARAM;
    }

    dev->objlist = (lwm2m_object_t*)LWM2M_LIST_ADD( dev->objlist, obj );

    return NBIOT_ERR_OK;
}

#ifdef HAVE_DTLS
static int send_to_peer( dtls_context_t  *ctx,
                         const session_t *session,
                         uint8_t         *data,
                         size_t           len )
{
    int ret;
    size_t sent;
    size_t offset;
    connection_t *conn;
    nbiot_device_t *dev;

    dev = (nbiot_device_t*)ctx->app;
    if ( NULL == dev )
    {
        return -1;
    }

    conn = connection_find( dev->connlist, session );
    if ( NULL == conn )
    {
        return -1;
    }

    offset = 0;
    while ( offset < len )
    {
        ret = nbiot_udp_send( dev->sock,
                              data + offset,
                              len - offset,
                              &sent,
                              conn->addr );
        if ( ret < 0 )
        {
            return -1;
        }
        else
        {
            offset += sent;
        }
    }

    return 0;
}

static int read_from_peer( dtls_context_t  *ctx,
                           const session_t *session,
                           uint8_t         *data,
                           size_t           len )
{
    connection_t *conn;
    nbiot_device_t *dev;

    dev = (nbiot_device_t*)ctx->app;
    if ( NULL == dev )
    {
        return -1;
    }

    conn = connection_find( dev->connlist, session );
    if ( NULL == conn )
    {
        return -1;
    }

    lwm2m_handle_packet( &dev->lwm2m,
                         data,
                         len,
                         conn );

    return 0;
}

static dtls_handler_t dtls_cb =
{
    .write = send_to_peer,
    .read  = read_from_peer,
    .event = NULL,
};
#endif

int nbiot_device_create( nbiot_device_t **dev,
                         uint16_t         local_port )
{
    nbiot_device_t *tmp;

    if ( NULL == dev )
    {
        return NBIOT_ERR_BADPARAM;
    }

    tmp = &g_dev;
    if ( NULL == tmp )
    {
        return NBIOT_ERR_NO_MEMORY;
    }
    else
    {
        nbiot_memzero( tmp, sizeof(nbiot_device_t) );
    }

    if ( nbiot_udp_create( &tmp->sock ) )
    {
        nbiot_free( tmp );

        return NBIOT_ERR_INTERNAL;
    }

    if ( nbiot_udp_bind(tmp->sock,NULL,local_port) )
    {
        nbiot_free( tmp );

        return NBIOT_ERR_INTERNAL;
    }

#ifdef HAVE_DTLS
    if ( dtls_init_context(&tmp->dtls,&dtls_cb,tmp) )
    {
        nbiot_udp_close( tmp->sock );
        nbiot_free( tmp );

        return NBIOT_ERR_DTLS;
    }
#endif

    if ( lwm2m_init(&tmp->lwm2m,tmp) )
    {
#ifdef HAVE_DTLS
        dtls_close_context( &tmp->dtls );
#endif
        nbiot_udp_close( tmp->sock );
        lwm2m_close( &tmp->lwm2m );
        nbiot_free( tmp );

        return NBIOT_ERR_INTERNAL;
    }

    *dev = tmp;
    return NBIOT_ERR_OK;
}

void nbiot_device_destroy( nbiot_device_t *dev )
{
    if ( NULL != dev )
    {
        lwm2m_object_t *obj;

        /* close */
        nbiot_device_close( dev );

        /* objects */
        while ( dev->objlist )
        {
            obj = dev->objlist;
            dev->objlist = obj->next;

            switch ( obj->objID )
            {
                case LWM2M_SECURITY_OBJECT_ID:
                case LWM2M_SERVER_OBJECT_ID:
                case LWM2M_ACL_OBJECT_ID:
                case LWM2M_DEVICE_OBJECT_ID:
                case LWM2M_CONN_MONITOR_OBJECT_ID:
                case LWM2M_FIRMWARE_UPDATE_OBJECT_ID:
                case LWM2M_LOCATION_OBJECT_ID:
                case LWM2M_CONN_STATS_OBJECT_ID:
                {
                    /* not supported */
                }
                break;

                default:
                {
                    clear_resource_object( obj );
                }
                break;
            }

            nbiot_free( obj );
        }

        /* free */
        nbiot_free( dev->addr );
        /* g_dev */
        /* nbiot_free( dev ); */
    }
}

int nbiot_device_connect( nbiot_device_t *dev,
                          const char     *server_uri,
                          time_t          life_time )
{
    if ( NULL == dev ||
         NULL == server_uri )
    {
        return NBIOT_ERR_BADPARAM;
    }

    /*
     * 初始化
     * 默认值：storing  = true
     *         boostrap = true
     *         uri_free = false
    */
    dev->data.uri = server_uri;
#ifdef LWM2M_BOOTSTRAP
    dev->data.flag |= LWM2M_USERDATA_BOOTSTRAP;
#endif
    dev->data.flag |= LWM2M_USERDATA_STORING;
    LWM2M_UINT24( dev->data.lifetime, life_time );

    return NBIOT_ERR_OK;
}

int nbiot_device_close( nbiot_device_t *dev )
{
    if ( NULL == dev )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( NULL != dev->sock )
    {
        nbiot_udp_close( dev->sock );
        dev->sock = NULL;
    }

    lwm2m_close( &dev->lwm2m );
    nbiot_memzero( &dev->lwm2m, sizeof(dev->lwm2m) );

#ifdef HAVE_DTLS
    dtls_close_context( &dev->dtls );
    nbiot_memzero( &dev->dtls, sizeof(dev->dtls) );
#endif

    if ( NULL != dev->connlist )
    {
        connection_destroy( dev->connlist );
        dev->connlist = NULL;
    }

    return NBIOT_ERR_OK;
}

int nbiot_device_configure( nbiot_device_t   *dev,
                            const char       *endpoint_name,
                            const char       *auth_code,
                            nbiot_resource_t *res_array[],
                            size_t            res_array_num )
{
    int i;
    int ret;
    lwm2m_object_t *obj;

    if ( NULL == dev ||
         NULL == endpoint_name ||
         NULL == auth_code ||
         NULL == res_array )
    {
        return NBIOT_ERR_BADPARAM;
    }

    for ( i = 0; i < (int)res_array_num; ++i )
    {
        bool exist = true;

        obj = nbiot_object_find( dev, res_array[i]->objid );
        if ( NULL == obj )
        {
            obj = (lwm2m_object_t*)nbiot_malloc( sizeof(lwm2m_object_t) );
            if ( NULL == obj )
            {
                return NBIOT_ERR_NO_MEMORY;
            }

            exist = false;
            nbiot_memzero( obj, sizeof(lwm2m_object_t) );
            obj->objID = res_array[i]->objid;
        }

        ret = create_resource_object( obj, res_array[i] );
        if ( ret )
        {
            if ( !exist )
            {
                nbiot_free( obj );
            }

            return ret;
        }

        if ( !exist )
        {
            ret = nbiot_object_add( dev, obj );
            if ( ret )
            {
                clear_resource_object( obj );
                nbiot_free( obj );

                return ret;
            }
        }
    }

    if ( lwm2m_configure(&dev->lwm2m,
                          endpoint_name,
                          auth_code,
                          dev->objlist) )
    {
        return NBIOT_ERR_INTERNAL;
    }

    return NBIOT_ERR_OK;
}

int nbiot_device_step( nbiot_device_t *dev,
                       time_t          timeout )
{
    int ret;
    size_t read;
    connection_t *conn;
    uint8_t buff[NBIOT_SOCK_RECV_BUF_SIZE];

    if ( NULL == dev )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( lwm2m_step(&dev->lwm2m,&timeout) )
    {
        return NBIOT_ERR_INTERNAL;
    }

    do
    {
        ret = nbiot_udp_recv( dev->sock,
                              buff,
                              sizeof(buff),
                              &read,
                              &dev->addr );
        if ( ret )
        {
            return ret;
        }

        if ( read > 0 )
        {
            conn = connection_find( dev->connlist, dev->addr );
            if ( NULL != conn )
            {
#ifdef HAVE_DTLS
                ret = dtls_handle_message( &dev->dtls,
                                           conn->addr,
                                           buff,
                                           read );
                if ( ret != 0 )
                {
                    return NBIOT_ERR_DTLS;
                }
#else
                lwm2m_handle_packet( &dev->lwm2m,
                                     buff,
                                     read,
                                     conn );
#endif
            }
        }
        else
        {
            break;
        }
    } while(1);

    if ( STATE_RESET == dev->lwm2m.state )
    {
        return NBIOT_ERR_SERVER_RESET;
    }

    return NBIOT_ERR_OK;
}

int nbiot_device_notify( nbiot_device_t *dev,
                         uint16_t        objid,
                         uint16_t        instid,
                         uint16_t        resid )
{
    lwm2m_uri_t uri;
    lwm2m_object_t *res_obj;

    if ( NULL == dev )
    {
        return NBIOT_ERR_BADPARAM;
    }

    res_obj = nbiot_object_find( dev, objid );
    if ( NULL == res_obj )
    {
        return NBIOT_ERR_NO_RESOURCE;
    }

    if ( !check_resource_object(res_obj,
                                instid,
                                resid) )
    {
        return NBIOT_ERR_NO_RESOURCE;
    }

    /* notify */
    uri.objectId = objid;
    uri.instanceId = instid;
    uri.resourceId = resid;
    uri.flag = LWM2M_URI_FLAG_OBJECT_ID |
               LWM2M_URI_FLAG_INSTANCE_ID |
               LWM2M_URI_FLAG_RESOURCE_ID;
    lwm2m_resource_value_changed( &dev->lwm2m, &uri );

    return NBIOT_ERR_OK;
}
