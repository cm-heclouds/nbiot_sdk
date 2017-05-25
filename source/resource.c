/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "m2m.h"

typedef struct _resource_t
{
    struct _resource_t *next;    /* matches lwm2m_list_t::next */
    uint16_t            resid;   /* matches lwm2m_list_t::id */
    nbiot_resource_t   *data;
}resource_t;

typedef struct _instance_t
{
    struct _instance_t *next;    /* matches lwm2m_list_t::next */
    uint16_t            instid;  /* matches lwm2m_list_t::id */
    resource_t         *reslist; /* matches lwm2m_list_t */
}instance_t;

static uint8_t prv_get_value( lwm2m_data_t *data,
                              resource_t   *res )
{
    nbiot_resource_t *tmp;

    tmp = res->data;
    switch ( tmp->type )
    {
        case NBIOT_VALUE_BOOLEAN:
        {
            lwm2m_data_encode_bool( tmp->value.as_bool, data );
            return COAP_205_CONTENT;
        }
        break;

        case NBIOT_VALUE_INTEGER:
        {
            lwm2m_data_encode_int( tmp->value.as_int, data );
            return COAP_205_CONTENT;
        }
        break;

        case NBIOT_VALUE_FLOAT:
        {
            lwm2m_data_encode_float( tmp->value.as_float, data );
            return COAP_205_CONTENT;
        }
        break;

        case NBIOT_VALUE_STRING:
        {
            lwm2m_data_encode_nstring( tmp->value.as_str.str,
                                       tmp->value.as_str.len,
                                       data );
            return COAP_205_CONTENT;
        }
        break;

        case NBIOT_VALUE_BINARY:
        {
            lwm2m_data_encode_opaque( tmp->value.as_bin.bin,
                                      tmp->value.as_bin.len,
                                      data );
            return COAP_205_CONTENT;
        }
        break;

        default:
        {
            return COAP_405_METHOD_NOT_ALLOWED;
        }
        break;
    }
}

static uint8_t prv_set_value( lwm2m_data_t *data,
                              resource_t   *res )
{
    nbiot_resource_t *tmp;

    switch ( data->type )
    {
        case LWM2M_TYPE_MULTIPLE_RESOURCE:
        {
            if ( !data->value.asChildren.count )
            {
                return COAP_400_BAD_REQUEST;
            }

            data = data->value.asChildren.array;
            if ( !data )
            {
                return COAP_400_BAD_REQUEST;
            }
        }
        break;

        default:
        {
            /* nothing */
        }
        break;
    }

    tmp = res->data;
    switch ( tmp->type )
    {
        case NBIOT_VALUE_BOOLEAN:
        {
            bool val;

            if ( 1 != lwm2m_data_decode_bool(data,&val) )
            {
                return COAP_400_BAD_REQUEST;
            }
            else
            {
                tmp->value.as_bool = val;
            }

            return COAP_204_CHANGED;
        }
        break;

        case NBIOT_VALUE_INTEGER:
        {
            int64_t val;

            if ( 1 != lwm2m_data_decode_int(data,&val) )
            {
                return COAP_400_BAD_REQUEST;
            }
            else
            {
                tmp->value.as_int = val;
            }

            return COAP_204_CHANGED;
        }
        break;

        case NBIOT_VALUE_FLOAT:
        {
            double val;

            if ( 1 != lwm2m_data_decode_float(data,&val) )
            {
                return COAP_400_BAD_REQUEST;
            }
            else
            {
                tmp->value.as_float = val;
            }

            return COAP_204_CHANGED;
        }
        break;

        case NBIOT_VALUE_STRING:
        case NBIOT_VALUE_BINARY:
        {
            size_t len;
            uint8_t *val;

            if ( LWM2M_TYPE_STRING != data->type &&
                 LWM2M_TYPE_OPAQUE != data->type )
            {
                return COAP_400_BAD_REQUEST;
            }

            if ( NULL == data->value.asBuffer.buffer )
            {
                return COAP_400_BAD_REQUEST;
            }

            val = data->value.asBuffer.buffer;
            len = data->value.asBuffer.length;
            data->value.asBuffer.buffer = NULL;
            data->value.asBuffer.length = 0;

            nbiot_free( tmp->value.as_bin.bin );
            tmp->value.as_bin.bin = val;
            tmp->value.as_bin.len = len;

            return COAP_204_CHANGED;
        }
        break;

        default:
            return COAP_405_METHOD_NOT_ALLOWED;
    }
}

static uint8_t prv_resource_read( uint16_t        instid,
                                  int            *num,
                                  lwm2m_data_t  **data,
                                  lwm2m_object_t *obj )
{
    int i;
    uint8_t ret;
    resource_t *res;
    instance_t *inst;

    inst = (instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == inst )
    {
        return COAP_404_NOT_FOUND;
    }

    /* is the server asking for the full instance ? */
    if ( 0 == *num )
    {
        i = 0;
        res = inst->reslist;
        while ( NULL != res )
        {
            if ( res->data->flag & NBIOT_RESOURCE_READABLE )
            {
                ++i;
            }
            res = res->next;
        }

        if ( i > 0 )
        {
            *data = lwm2m_data_new( i );
            if ( NULL == *data )
            {
                return COAP_500_INTERNAL_SERVER_ERROR;
            }
            *num = i;

            i = 0;
            res = inst->reslist;
            while ( NULL != res )
            {
                if ( res->data->flag & NBIOT_RESOURCE_READABLE )
                {
                    (*data)[i++].id = res->resid;
                }
                res = res->next;
            }
        }
        else
        {
            *data = NULL;
            *num = 0;
        }
    }

    ret = COAP_405_METHOD_NOT_ALLOWED;
    if ( *num > 0 )
    {
        i = 0;
        do
        {
            res = (resource_t*)LWM2M_LIST_FIND( inst->reslist, (*data)[i].id );
            if ( NULL == res )
            {
                ret = COAP_404_NOT_FOUND;
            }
            else if ( res->data->flag & NBIOT_RESOURCE_READABLE )
            {
                ret = prv_get_value( (*data) + i, res );
            }
            else
            {
                ret = COAP_405_METHOD_NOT_ALLOWED;
            }

            ++i;
        }
        while ( i < *num && COAP_205_CONTENT == ret );
    }

    return ret;
}

static uint8_t prv_resource_write( uint16_t        instid,
                                   int             num,
                                   lwm2m_data_t   *data,
                                   lwm2m_object_t *obj )
{
    int i;
    uint8_t ret;
    resource_t *res;
    instance_t *inst;
    nbiot_resource_t *tmp;

    inst = (instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == inst )
    {
        return COAP_404_NOT_FOUND;
    }

    i = 0;
    while ( i < num )
    {
        res = (resource_t*)LWM2M_LIST_FIND( inst->reslist, data[i].id );
        if ( NULL == res )
        {
            ret = COAP_404_NOT_FOUND;
            break;
        }

        tmp = res->data;
        if ( tmp->flag & NBIOT_RESOURCE_WRITABLE )
        {
            ret = prv_set_value( data+i, res );
            if ( COAP_204_CHANGED != ret )
            {
                break;
            }

            if ( NULL != tmp->write )
            {
                (*tmp->write)(tmp);
            }
        }
        else
        {
            ret = COAP_405_METHOD_NOT_ALLOWED;
            break;
        }

        ++i;
    }

    return ret;
}

static uint8_t prv_resource_execute( uint16_t        instid,
                                     uint16_t        resid,
                                     uint8_t        *buffer,
                                     int             length,
                                     lwm2m_object_t *obj )
{
    resource_t *res;
    instance_t *inst;
    nbiot_resource_t *tmp;

    inst = (instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == inst )
    {
        return COAP_404_NOT_FOUND;
    }

    res = (resource_t*)LWM2M_LIST_FIND( inst->reslist, resid );
    if ( NULL == res )
    {
        return COAP_404_NOT_FOUND;
    }

    tmp = res->data;
    if ( tmp->flag & NBIOT_RESOURCE_EXECUTABLE )
    {
        if ( NULL != tmp->execute )
        {
            (*tmp->execute)(tmp, buffer, length);
        }
    }
    else
    {
        return COAP_405_METHOD_NOT_ALLOWED;
    }

    return COAP_204_CHANGED;
}

static uint8_t prv_resource_discover( uint16_t        instid,
                                      int            *num,
                                      lwm2m_data_t  **data,
                                      lwm2m_object_t *obj )
{
    int i;
    resource_t *res;
    instance_t *inst;

    inst = (instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == inst )
    {
        return COAP_404_NOT_FOUND;
    }

    /* is the server asking for the full instance ? */
    if ( 0 == *num )
    {
        i = 0;
        res = inst->reslist;
        while ( NULL != res )
        {
            if ( res->data->flag & NBIOT_RESOURCE_READABLE )
            {
                ++i;
            }
            res = res->next;
        }

        if ( i > 0 )
        {
            *data = lwm2m_data_new( i );
            if ( NULL == *data )
            {
                return COAP_500_INTERNAL_SERVER_ERROR;
            }
            *num = i;

            i = 0;
            res = inst->reslist;
            while ( NULL != res )
            {
                if ( res->data->flag & NBIOT_RESOURCE_READABLE )
                {
                    (*data)[i++].id = res->resid;
                }
                res = res->next;
            }
        }
        else
        {
            *data = NULL;
            *num = 0;
        }
    }

    return COAP_205_CONTENT;
}

int create_resource_object( lwm2m_object_t   *obj,
                            nbiot_resource_t *data )
{
    resource_t *res;
    instance_t *inst;
    bool exist = true;

    if ( NULL == obj ||
         NULL == data )
    {
        return NBIOT_ERR_BADPARAM;
    }

    inst = (instance_t*)LWM2M_LIST_FIND( obj->instanceList, data->instid );
    if ( NULL == inst )
    {
        inst = (instance_t*)nbiot_malloc( sizeof(instance_t) );
        if ( NULL == inst )
        {
            return NBIOT_ERR_NO_MEMORY;
        }

        exist = false;
        nbiot_memzero( inst, sizeof(instance_t) );
        inst->instid = data->instid;
    }

    res = (resource_t*)LWM2M_LIST_FIND( inst->reslist, data->resid );
    if ( NULL == res )
    {
        res = (resource_t*)nbiot_malloc( sizeof(resource_t) );
        if ( NULL == res )
        {
            if ( !exist )
            {
                nbiot_free( inst );
            }

            return NBIOT_ERR_NO_MEMORY;
        }

        nbiot_memzero( res, sizeof(resource_t) );
        res->resid = data->resid;
        inst->reslist = (resource_t*)LWM2M_LIST_ADD( inst->reslist, res );
    }

    /* setting */
    res->data = data;
    if ( !exist )
    {
        obj->instanceList = LWM2M_LIST_ADD( obj->instanceList, inst );
        obj->readFunc     = prv_resource_read;
        obj->writeFunc    = prv_resource_write;
        obj->executeFunc  = prv_resource_execute;
        obj->discoverFunc = prv_resource_discover;
    }

    return NBIOT_ERR_OK;
}

bool check_resource_object( lwm2m_object_t *obj,
                            uint16_t        instid,
                            uint16_t        resid )
{
    resource_t *res;
    instance_t *inst;

    if ( NULL == obj )
    {
        return false;
    }

    inst = (instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == inst )
    {
        return false;
    }

    res = (resource_t*)LWM2M_LIST_FIND( inst->reslist, resid );
    if ( NULL == res )
    {
        return false;
    }

    return true;
}

void clear_resource_object( lwm2m_object_t *obj )
{
    resource_t *res;
    instance_t *inst;

    if ( NULL == obj )
    {
        return;
    }

    while ( NULL != obj->instanceList )
    {
        inst = (instance_t*)obj->instanceList;
        obj->instanceList = obj->instanceList->next;

        while ( NULL != inst->reslist )
        {
            res = inst->reslist;
            inst->reslist = res->next;
            nbiot_free( res );
        }

        nbiot_free( inst );
    }
}
