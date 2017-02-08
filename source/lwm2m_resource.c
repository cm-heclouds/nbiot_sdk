/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "lwm2m.h"

typedef struct resource_instance_t
{
    struct resource_instance_t   *next;     /* matches lwm2m_list_t::next */
    uint16_t                      resid;    /* matches lwm2m_list_t::id */
    nbiot_resource_t             *resource;
}resource_instance_t;

typedef struct object_instance_t
{
    struct object_instance_t *next;         /* matches lwm2m_list_t::next */
    uint16_t                  instid;       /* matches lwm2m_list_t::id */
    resource_instance_t      *resouces;     /* matches lwm2m_list_t */
}object_instance_t;

static uint8_t prv_get_value( lwm2m_data_t        *data,
                              resource_instance_t *res_inst )
{
    nbiot_resource_t *res;

    res = res_inst->resource;
    switch ( res->type )
    {
        case NBIOT_VALUE_BOOLEAN:
            lwm2m_data_encode_bool( res->value.as_bool, data );
            return COAP_205_CONTENT;

        case NBIOT_VALUE_INTEGER:
            lwm2m_data_encode_int( res->value.as_int, data );
            return COAP_205_CONTENT;

        case NBIOT_VALUE_FLOAT:
            lwm2m_data_encode_float( res->value.as_float, data );
            return COAP_205_CONTENT;

        case NBIOT_VALUE_STRING:
            lwm2m_data_encode_string( res->value.as_str.str, data );
            return COAP_205_CONTENT;

        case NBIOT_VALUE_BINARY:
            lwm2m_data_encode_opaque( res->value.as_bin.bin,
                                      res->value.as_bin.len,
                                      data );
            return COAP_205_CONTENT;

        default:
            return COAP_405_METHOD_NOT_ALLOWED;
    }
}

static uint8_t prv_set_value( lwm2m_data_t        *data,
                              resource_instance_t *res_inst )
{
    nbiot_resource_t *res;

    res = res_inst->resource;
    switch ( res->type )
    {
        case NBIOT_VALUE_BOOLEAN:
        {
            bool val;

            if ( LWM2M_TYPE_MULTIPLE_RESOURCE == data->type )
            {
                if ( NULL == data->value.asChildren.array )
                {
                    return COAP_400_BAD_REQUEST;
                }

                data = data->value.asChildren.array;
            }

            if ( 1 != lwm2m_data_decode_bool(data,&val) )
            {
                return COAP_400_BAD_REQUEST;
            }

            res->value.as_bool = val;
            return COAP_204_CHANGED;
        }
        break;

        case NBIOT_VALUE_INTEGER:
        {
            int64_t val;

            if ( LWM2M_TYPE_MULTIPLE_RESOURCE == data->type )
            {
                if ( NULL == data->value.asChildren.array )
                {
                    return COAP_400_BAD_REQUEST;
                }

                data = data->value.asChildren.array;
            }

            if ( 1 != lwm2m_data_decode_int(data,&val) )
            {
                return COAP_400_BAD_REQUEST;
            }

            res->value.as_int = (int)val;
            return COAP_204_CHANGED;
        }
        break;

        case NBIOT_VALUE_FLOAT:
        {
            double val;

            if ( LWM2M_TYPE_MULTIPLE_RESOURCE == data->type )
            {
                if ( NULL == data->value.asChildren.array )
                {
                    return COAP_400_BAD_REQUEST;
                }

                data = data->value.asChildren.array;
            }

            if ( 1 != lwm2m_data_decode_float(data,&val) )
            {
                return COAP_400_BAD_REQUEST;
            }

            res->value.as_float = (float)val;
            return COAP_204_CHANGED;
        }
        break;

        case NBIOT_VALUE_STRING:
        {
            char *str;

            if ( LWM2M_TYPE_MULTIPLE_RESOURCE == data->type )
            {
                if ( NULL == data->value.asChildren.array )
                {
                    return COAP_400_BAD_REQUEST;
                }

                data = data->value.asChildren.array;
            }

            if ( LWM2M_TYPE_STRING != data->type ||
                 NULL == data->value.asBuffer.buffer )
            {
                return COAP_400_BAD_REQUEST;
            }

            str = lwm2m_strdup( (char*)data->value.asBuffer.buffer );
            if ( NULL == str )
            {
                return COAP_400_BAD_REQUEST;
            }

            lwm2m_free( res->value.as_str.str );
            res->value.as_str.str = str;
            res->value.as_str.len = nbiot_strlen( str ) + 1;

            return COAP_204_CHANGED;
        }
        break;


        case NBIOT_VALUE_BINARY:
        {
            uint8_t *bin;

            if ( LWM2M_TYPE_MULTIPLE_RESOURCE == data->type )
            {
                if ( NULL == data->value.asChildren.array )
                {
                    return COAP_400_BAD_REQUEST;
                }

                data = data->value.asChildren.array;
            }

            if ( (LWM2M_TYPE_STRING != data->type &&
                  LWM2M_TYPE_OPAQUE != data->type) ||
                  NULL == data->value.asBuffer.buffer )
            {
                return COAP_400_BAD_REQUEST;
            }

            bin = lwm2m_malloc( data->value.asBuffer.length );
            if ( NULL == bin )
            {
                return COAP_400_BAD_REQUEST;
            }

            nbiot_memmove( bin,
                           data->value.asBuffer.buffer,
                           data->value.asBuffer.length );
            lwm2m_free( res->value.as_bin.bin );
            res->value.as_bin.bin = bin;
            res->value.as_bin.len = data->value.asBuffer.length;

            return COAP_204_CHANGED;
        }

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
    object_instance_t *obj_inst;
    resource_instance_t *res_inst;

    obj_inst = (object_instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == obj_inst )
    {
        return COAP_404_NOT_FOUND;
    }

    /* is the server asking for the full instance ? */
    if ( 0 == *num )
    {
        i = 0;
        res_inst = obj_inst->resouces;
        while ( NULL != res_inst )
        {
            ++i;
            res_inst = res_inst->next;
        }

        *data = lwm2m_data_new( i );
        if ( NULL == *data )
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        *num = i;

        i = 0;
        res_inst = obj_inst->resouces;
        while ( NULL != res_inst )
        {
            (*data)[i++].id = res_inst->resid;
            res_inst = res_inst->next;
        }
    }

    i = 0;
    do
    {
        res_inst = (resource_instance_t*)LWM2M_LIST_FIND( obj_inst->resouces, (*data)[i].id );
        if ( NULL == res_inst )
        {
            ret = COAP_404_NOT_FOUND;
        }
        else
        {
            ret = prv_get_value( (*data)+i, res_inst );
        }

        ++i;
    } while ( i < *num && COAP_205_CONTENT == ret );

    return ret;
}

static uint8_t prv_resource_write( uint16_t        instid,
                                   int             num,
                                   lwm2m_data_t   *data,
                                   lwm2m_object_t *obj )
{
    int i;
    uint8_t ret;
    nbiot_resource_t *res;
    object_instance_t *obj_inst;
    resource_instance_t *res_inst;

    obj_inst = (object_instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == obj_inst )
    {
        return COAP_404_NOT_FOUND;
    }

    if ( num <= 0 )
    {
        return COAP_404_NOT_FOUND;
    }

    for ( i = 0; i < num; ++i )
    {
        res_inst = (resource_instance_t*)LWM2M_LIST_FIND( obj_inst->resouces, data[i].id );
        if ( NULL == res_inst )
        {
            ret = COAP_404_NOT_FOUND;
            break;
        }

        res = res_inst->resource;
        if ( res->flag & NBIOT_VALUE_WRITABLE )
        {
            ret = prv_set_value( data+i, res_inst );
            if ( COAP_204_CHANGED != ret )
            {
                break;
            }

            if ( NULL != res->write )
            {
                (*res->write)( res );
            }
        }
        else
        {
            ret = COAP_405_METHOD_NOT_ALLOWED;
            break;
        }
    }

    return ret;
}

static uint8_t prv_resource_execute( uint16_t        instid,
                                     uint16_t        resid,
                                     uint8_t        *buffer,
                                     int             length,
                                     lwm2m_object_t *obj )
{
    nbiot_resource_t *res;
    object_instance_t *obj_inst;
    resource_instance_t *res_inst;

    obj_inst = (object_instance_t*)LWM2M_LIST_FIND( obj->instanceList, instid );
    if ( NULL == obj_inst )
    {
        return COAP_404_NOT_FOUND;
    }

    res_inst = (resource_instance_t*)LWM2M_LIST_FIND( obj_inst->resouces, resid );
    if ( NULL == res_inst )
    {
        return COAP_404_NOT_FOUND;
    }

    res = res_inst->resource;
    if ( res->flag & NBIOT_VALUE_EXECUTABLE )
    {
        if ( NULL != res->execute )
        {
            (*res->execute)( res, buffer, length );
        }
    }
    else
    {
        return COAP_405_METHOD_NOT_ALLOWED;
    }

    return COAP_204_CHANGED;
}

int create_resource_object( lwm2m_object_t   *res_obj,
                            nbiot_resource_t *resource )
{
    bool obj_inst_exist = true;
    object_instance_t *obj_inst;
    resource_instance_t *res_inst;

    if ( NULL == res_obj ||
         NULL == resource )
    {
        return NBIOT_ERR_BADPARAM;
    }

    obj_inst = (object_instance_t*)LWM2M_LIST_FIND( res_obj->instanceList, resource->instid );
    if ( NULL == obj_inst )
    {
        obj_inst = (object_instance_t*)lwm2m_malloc( sizeof(object_instance_t) );
        if ( NULL == obj_inst )
        {
            return NBIOT_ERR_NO_MEMORY;
        }

        obj_inst_exist = false;
        nbiot_memzero( obj_inst, sizeof(object_instance_t) );
        obj_inst->instid = resource->instid;
    }

    res_inst = (resource_instance_t*)LWM2M_LIST_FIND( obj_inst->resouces, resource->resid );
    if ( NULL == res_inst )
    {
        res_inst = (resource_instance_t*)lwm2m_malloc( sizeof(resource_instance_t) );
        if ( NULL == res_inst )
        {
            if ( !obj_inst_exist )
            {
                nbiot_free( obj_inst );
            }

            return NBIOT_ERR_NO_MEMORY;
        }

        nbiot_memzero( res_inst, sizeof(resource_instance_t) );
        res_inst->resid = resource->resid;
        obj_inst->resouces = (resource_instance_t*)LWM2M_LIST_ADD( obj_inst->resouces, res_inst );
    }

    /* setting */
    res_inst->resource = resource;
    if ( !obj_inst_exist )
    {
        res_obj->instanceList = LWM2M_LIST_ADD( res_obj->instanceList, obj_inst );
        res_obj->readFunc = prv_resource_read;
        res_obj->writeFunc = prv_resource_write;
        res_obj->executeFunc = prv_resource_execute;
    }

    return NBIOT_ERR_OK;
}

bool check_resource_object( lwm2m_object_t *sec_obj,
                            uint16_t        instid,
                            uint16_t        resid )
{
    object_instance_t *obj_inst;
    resource_instance_t *res_inst;

    if ( NULL == sec_obj )
    {
        return false;
    }

    obj_inst = (object_instance_t*)LWM2M_LIST_FIND( sec_obj->instanceList, instid );
    if ( NULL == obj_inst )
    {
        return false;
    }

    res_inst = (resource_instance_t*)LWM2M_LIST_FIND( obj_inst->resouces, resid );
    if ( NULL == res_inst )
    {
        return false;
    }

    return true;
}

void clear_resource_object( lwm2m_object_t *sec_obj )
{
    object_instance_t *obj_inst;
    resource_instance_t *res_inst;

    if ( NULL == sec_obj )
    {
        return;
    }

    while ( NULL != sec_obj->instanceList )
    {
        obj_inst = (object_instance_t*)sec_obj->instanceList;
        sec_obj->instanceList = sec_obj->instanceList->next;

        while ( NULL != obj_inst->resouces )
        {
            res_inst = obj_inst->resouces;
            obj_inst->resouces = res_inst->next;
            nbiot_free( res_inst );
        }

        nbiot_free( obj_inst );
    }
}