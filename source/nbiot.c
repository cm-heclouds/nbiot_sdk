/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <nbiot.h>
#include "struct.h"

#define DEFAULT_SERVER_ID   1
#define DEFAULT_BOOTSTRAP   false
#define DEFAULT_BINDING     "U"
#define DEFAULT_STORING     true

lwm2m_object_t*
nbiot_object_find( nbiot_device_t *dev,
                   uint16_t        objid )
{
    int i;

    if ( NULL == dev )
    {
        return NULL;
    }

    for ( i = 0; i < (int)dev->obj_num; ++i )
    {
        if ( dev->obj_arr[i] )
        {
            if ( dev->obj_arr[i]->objID == objid )
            {
                return dev->obj_arr[i];
            }
        }
    }

    return NULL;
}

int nbiot_object_add( nbiot_device_t *dev,
                      lwm2m_object_t *obj )
{
    if ( NULL == dev ||
         NULL == obj )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( dev->obj_num >= dev->obj_max )
    {
        size_t num;
        lwm2m_object_t **arr;

        num = dev->obj_max * 2;
        arr = (lwm2m_object_t**)nbiot_malloc( num*sizeof(lwm2m_object_t*) );
        if ( NULL == arr )
        {
            return NBIOT_ERR_NO_MEMORY;
        }

        nbiot_memzero( arr, num*sizeof(lwm2m_object_t*) );
        nbiot_memmove( arr,
                       dev->obj_arr,
                       dev->obj_num*sizeof(lwm2m_object_t*) );
        nbiot_free( dev->obj_arr );
        dev->obj_arr = arr;
        dev->obj_max = num;
    }

    dev->obj_arr[dev->obj_num++] = obj;
    return NBIOT_ERR_OK;
}

void nbiot_object_del( nbiot_device_t *dev,
                       lwm2m_object_t *obj )
{
    if ( NULL != dev && NULL != obj )
    {
        int i;

        for ( i = 0; i < (int)dev->obj_num; ++i )
        {
            if ( obj == dev->obj_arr[i] )
            {
                break;
            }
        }

        if ( i < (int)dev->obj_num )
        {
            nbiot_memmove( dev->obj_arr+i,
                           dev->obj_arr+i+1,
                           sizeof(lwm2m_object_t*)*(dev->obj_num-i-1) );
            dev->obj_arr[--dev->obj_num] = NULL;
        }
    }
}

int nbiot_device_create( nbiot_device_t **dev,
                         uint16_t         port,
                         const char      *serial_number )
{
    nbiot_device_t *tmp;
    lwm2m_object_t *dev_obj;

    if ( NULL == dev )
    {
        return NBIOT_ERR_BADPARAM;
    }

    tmp = nbiot_malloc( sizeof(nbiot_device_t) );
    if ( NULL == tmp )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    nbiot_memzero( tmp, sizeof(nbiot_device_t) );
    tmp->obj_max = 4;
    tmp->obj_num = 0;
    tmp->obj_arr = (lwm2m_object_t**)nbiot_malloc( tmp->obj_max*sizeof(lwm2m_object_t*) );
    if ( NULL == tmp->obj_arr )
    {
        nbiot_free( tmp );

        return NBIOT_ERR_NO_MEMORY;
    }

    nbiot_memzero( tmp->obj_arr, tmp->obj_max*sizeof(lwm2m_object_t*) );
    if ( nbiot_udp_create( &tmp->sock ) )
    {
        nbiot_free( tmp->obj_arr );
        nbiot_free( tmp );

        return NBIOT_ERR_INTERNAL;
    }

    if ( nbiot_udp_bind(tmp->sock,NULL,port) )
    {
        nbiot_free( tmp->obj_arr );
        nbiot_free( tmp );

        return NBIOT_ERR_INTERNAL;
    }

    dev_obj = create_device_object( serial_number );
    if ( NULL == dev_obj )
    {
        nbiot_udp_close( tmp->sock );
        nbiot_free( tmp );

        return NBIOT_ERR_INTERNAL;
    }

    tmp->lwm2m = lwm2m_init( tmp );
    if ( NULL == tmp->lwm2m )
    {
        clear_device_object( dev_obj );
        nbiot_udp_close( tmp->sock );
        nbiot_free( tmp->obj_arr );
        nbiot_free( dev_obj );
        nbiot_free( tmp );

        return NBIOT_ERR_INTERNAL;
    }

    *dev = tmp;
    nbiot_object_add( *dev, dev_obj );

    return NBIOT_ERR_OK;
}

void nbiot_device_destroy( nbiot_device_t *dev )
{
    if ( NULL != dev )
    {
        int i;

        /* close */
        nbiot_device_close( dev );

        /* objects */
        for ( i = 0; i < (int)dev->obj_num; ++i )
        {
            if ( NULL != dev->obj_arr[i] )
            {
                switch ( dev->obj_arr[i]->objID )
                {
                    case LWM2M_SECURITY_OBJECT_ID:
                        clear_security_object( dev->obj_arr[i] );
                        break;

                    case LWM2M_SERVER_OBJECT_ID:
                        clear_server_object( dev->obj_arr[i] );
                        break;

                    case LWM2M_DEVICE_OBJECT_ID:
                        clear_device_object( dev->obj_arr[i] );
                        break;

                    case LWM2M_ACL_OBJECT_ID:
                    case LWM2M_CONN_MONITOR_OBJECT_ID:
                    case LWM2M_FIRMWARE_UPDATE_OBJECT_ID:
                    case LWM2M_LOCATION_OBJECT_ID:
                    case LWM2M_CONN_STATS_OBJECT_ID:
                        /* not supported */
                        break;

                    default:
                        clear_resource_object( dev->obj_arr[i] );
                        break;
                }

                nbiot_free( dev->obj_arr[i] );
                dev->obj_arr[i] = NULL;
            }
        }

        /* free */
        nbiot_free( dev->obj_arr );
        nbiot_free( dev->addr );
        nbiot_free( dev );
    }
}

int nbiot_device_connect( nbiot_device_t *dev,
                          const char     *server_uri,
                          time_t          keep_alive,
                          bool            bootstrap )
{
    lwm2m_object_t *sec_obj;
    lwm2m_object_t *svr_obj;

    if ( NULL == dev ||
         NULL == server_uri )
    {
        return NBIOT_ERR_BADPARAM;
    }

    sec_obj = create_security_object( DEFAULT_SERVER_ID,
                                      server_uri,
                                      (uint32_t)keep_alive,
                                      DEFAULT_BOOTSTRAP );
    if ( NULL == sec_obj )
    {
        return NBIOT_ERR_INTERNAL;
    }

    if ( nbiot_object_add(dev,sec_obj) )
    {
        clear_security_object( sec_obj );
        nbiot_free( sec_obj );

        return NBIOT_ERR_INTERNAL;
    }

    svr_obj = create_server_object( DEFAULT_SERVER_ID,
                                    DEFAULT_BINDING,
                                    (uint32_t)keep_alive,
                                    DEFAULT_STORING );
    if ( NULL == svr_obj )
    {
        return NBIOT_ERR_INTERNAL;
    }

    if ( nbiot_object_add(dev,svr_obj) )
    {
        clear_server_object( svr_obj );
        nbiot_free( svr_obj );

        return NBIOT_ERR_INTERNAL;
    }

    return NBIOT_ERR_OK;
}

int nbiot_device_close( nbiot_device_t *dev )
{
    if ( NULL == dev )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( NULL != dev->lwm2m )
    {
        lwm2m_close( dev->lwm2m );
        dev->lwm2m = NULL;
    }

    if ( NULL != dev->sock )
    {
        nbiot_udp_close( dev->sock );
        dev->sock = NULL;
    }

    if ( NULL != dev->connlist )
    {
        connection_destroy( dev->connlist );
        dev->connlist = NULL;
    }

    return NBIOT_ERR_OK;
}

int nbiot_device_configure( nbiot_device_t *dev,
                            const char     *endpoint_name,
                            const char     *auth_code,
                            nbiot_resource_t    *res_array[],
                            size_t          res_array_num )
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

    if ( lwm2m_configure(dev->lwm2m,
                         endpoint_name,
                         auth_code,
                         NULL,
                         NULL,
                         dev->obj_num,
                         dev->obj_arr) )
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

    if ( lwm2m_step(dev->lwm2m,&timeout) )
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
            conn = connection_find( dev->connlist,
                                    dev->addr );
            if ( NULL != conn )
            {
                lwm2m_handle_packet( dev->lwm2m,
                                     buff,
                                     read,
                                     conn );
            }
        }
        else
        {
            break;
        }
    } while(1);

    if ( STATE_RESET == dev->lwm2m->state )
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
    lwm2m_resource_value_changed( dev->lwm2m, &uri );

    return NBIOT_ERR_OK;
}