/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "m2m.h"

typedef struct device_instance_t
{
    struct server_instance_t *next;         /* matches lwm2m_list_t::next */
    uint16_t                  instid;       /* matches lwm2m_list_t::id */
    char                     *serial_number;
}device_instance_t;

/* resouce id */
#define LWM2M_DEVICE_MANUFACTURER     0
#define LWM2M_DEVICE_MODEL_NUMBER     1
#define LWM2M_DEVICE_SERIAL_NUMBER    2
#define LWM2M_DEVICE_FIRMWARE_VERSION 3

static uint8_t prv_get_value( lwm2m_data_t      *data,
                              device_instance_t *dev )
{
    switch ( data->id )
    {
        case LWM2M_DEVICE_MANUFACTURER:
            lwm2m_data_encode_string( NBIOT_DEVICE_MANUFACTURER, data );
            return COAP_205_CONTENT;

        case LWM2M_DEVICE_MODEL_NUMBER:
            lwm2m_data_encode_string( NBIOT_DEVICE_MODEL_NUMBER, data );
            return COAP_205_CONTENT;

        case LWM2M_DEVICE_SERIAL_NUMBER:
            lwm2m_data_encode_string( dev->serial_number, data );
            return COAP_205_CONTENT;

        case LWM2M_DEVICE_FIRMWARE_VERSION:
            lwm2m_data_encode_string( NBIOT_DEVICE_FIRMWARE_VERSION, data );
            return COAP_205_CONTENT;

        default:
            return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_device_read( uint16_t        instid,
                                int            *num,
                                lwm2m_data_t  **data,
                                lwm2m_object_t *obj )
{
    int i;
    uint8_t ret;
    device_instance_t *dev;

    dev = (device_instance_t*)LWM2M_LIST_FIND(obj->instanceList,instid);
    if ( NULL == dev )
    {
        return COAP_404_NOT_FOUND;
    }

    /* is the server asking for the full instance ? */
    if ( 0 == *num )
    {
        uint16_t res[] = {
            LWM2M_DEVICE_MANUFACTURER,
            LWM2M_DEVICE_MODEL_NUMBER,
            LWM2M_DEVICE_SERIAL_NUMBER,
            LWM2M_DEVICE_FIRMWARE_VERSION
        };

        i = sizeof(res) / sizeof(uint16_t);
        *data = lwm2m_data_new( i );
        if ( NULL == *data )
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        *num = i;
        for ( i = 0; i < *num; ++i )
        {
            (*data)[i].id = res[i];
        }
    }

    i = 0;
    do
    {
        ret = prv_get_value( (*data) + i, dev );
        ++i;
    }
    while ( i < *num && COAP_205_CONTENT == ret );

    return ret;
}

lwm2m_object_t*
create_device_object( const char *serial_number )
{
    char *str;
    lwm2m_object_t *obj;
    device_instance_t *dev;

    if ( NULL == serial_number )
    {
        return NULL;
    }

    obj = (lwm2m_object_t*)nbiot_malloc( sizeof(lwm2m_object_t) );
    if ( NULL == obj )
    {
        return NULL;
    }

    nbiot_memzero( obj, sizeof(lwm2m_object_t) );
    obj->objID = LWM2M_DEVICE_OBJECT_ID;

    dev = (device_instance_t*)nbiot_malloc( sizeof(device_instance_t) );
    if ( NULL == dev )
    {
        nbiot_free( obj );

        return NULL;
    }

    str = nbiot_strdup( serial_number );
    if ( NULL == str )
    {
        nbiot_free( dev );
        nbiot_free( obj );

        return NULL;
    }

    nbiot_memzero( dev, sizeof(device_instance_t) );
    dev->instid = 0;
    dev->serial_number = str;
    obj->instanceList = LWM2M_LIST_ADD(obj->instanceList,dev);
    obj->readFunc = prv_device_read;

    return obj;
}

void clear_device_object( lwm2m_object_t *dev_obj )
{
    while ( NULL != dev_obj->instanceList )
    {
        device_instance_t *dev;

        dev = (device_instance_t*)dev_obj->instanceList;
        dev_obj->instanceList = dev_obj->instanceList->next;

        if ( NULL != dev->serial_number )
        {
            nbiot_free( dev->serial_number );
        }
        nbiot_free( dev );
    }
}