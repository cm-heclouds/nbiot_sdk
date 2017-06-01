/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

static int nbiot_add_resource( uint8_t *buffer,
                               uint16_t buffer_len,
                               uint16_t objid,
                               uint16_t instid,
                               bool     first )
{
    int rc;
    uint16_t offset = 0;

    if ( first )
    {
        offset++;
        *buffer++ = 0xff;
        rc = nbiot_add_string( "</>;rt=\"oma.lwm2m\",</",
                               (char*)buffer,
                               buffer_len - offset );
        if ( !rc )
        {
            return 0;
        }
    }
    else
    {
        rc = nbiot_add_string( ",</",
                               (char*)buffer,
                               buffer_len - offset );
        if ( !rc )
        {
            return 0;
        }
    }

    /* object id */
    offset += rc;
    buffer += rc;
    rc = nbiot_add_integer( objid,
                            (char*)buffer,
                            buffer_len - offset );
    if ( !rc )
    {
        return 0;
    }

    /* "/" */
    offset += rc;
    buffer += rc;
    if ( offset + 1 > buffer_len )
    {
        return 0;
    }
    else
    {
        offset++;
        *buffer++ = '/';
    }

    /* instance id */
    rc = nbiot_add_integer( instid,
                            (char*)buffer,
                            buffer_len - offset );
    if ( !rc )
    {
        return 0;
    }

    /* ">" */
    offset += rc;
    buffer += rc;
    if ( offset + 1 > buffer_len )
    {
        return 0;
    }
    else
    {
        offset++;
        *buffer++ = '>';
    }

    return offset;
}

static int nbiot_add_objects( nbiot_device_t *dev,
                              coap_t         *coap )
{
    nbiot_node_t *obj, *inst;

    for ( obj = dev->nodes;
          obj != NULL;
          obj = obj->next )
    {
        for ( inst = (nbiot_node_t*)obj->data;
              inst != NULL;
              inst = inst->next )
        {
            int len = nbiot_add_resource( coap->buffer + coap->offset,
                                          coap->size - coap->offset,
                                          obj->id,
                                          inst->id,
                                          inst == dev->nodes->data );
            if ( len )
            {
                coap->offset += len;
            }
            else
            {
                return NBIOT_ERR_NO_MEMORY;
            }
        }
    }

    return NBIOT_ERR_OK;
}

int nbiot_register_start( nbiot_device_t *dev,
                          uint16_t        mid,
                          const uint8_t  *token,
                          uint8_t         token_len,
                          uint8_t        *buffer,
                          size_t          buffer_len )
{
    int len;
    coap_t coap[1];

    /* header */
    coap->buffer = buffer;
    coap->size = buffer_len;
    if ( coap_init_header(coap,
                          COAP_TYPE_CON,
                          COAP_POST,
                          mid,
                          token,
                          token_len) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* uri path */
    if ( coap_add_option(coap,
                         COAP_OPTION_URI_PATH,
                         "rd",
                         2) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* content type */
    if ( coap_add_int_option(coap,
                             COAP_OPTION_CONTENT_TYPE,
                             LWM2M_CONTENT_LINK) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* endpoint name */
    if ( nbiot_add_uri_query( coap,
                              "ep=",
                              dev->endpoint_name) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* binding */
    if ( nbiot_add_uri_query(coap,"b=","U"))
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* life time */
    if ( dev->life_time > 0 )
    {
        char str[6];
        nbiot_itoa( dev->life_time, str, sizeof(str) );
        if ( nbiot_add_uri_query(coap,"lt=",str) )
        {
            return NBIOT_ERR_NO_MEMORY;
        }
    }

    if ( nbiot_add_objects(dev,coap) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    len = nbiot_send_buffer( dev->socket,
                             dev->server,
                             coap->buffer,
                             coap->offset );
    if ( len != coap->offset )
    {
        return NBIOT_ERR_SOCKET;
    }

    dev->state = STATE_REG_PENDING;
    return NBIOT_ERR_OK;
}

static int nbiot_add_location( nbiot_device_t *dev,
                               coap_t         *coap )
{
#ifndef NBIOT_LOCATION
    const char *str;

    /* rd */
    if ( coap_add_option(coap,
                         COAP_OPTION_URI_PATH,
                         (const uint8_t*)"rd",
                         2) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* endpoint name */
    str = nbiot_strrchr( dev->endpoint_name, -1, ';' );
    if ( coap_add_option(coap,
                         COAP_OPTION_URI_PATH,
                         (const uint8_t*)dev->endpoint_name,
                         str ? str-dev->endpoint_name : nbiot_strlen(dev->endpoint_name)) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }
#else
    nbiot_location_t *location;

    location = dev->locations;
    while ( location )
    {
        if ( coap_add_option(coap,
                             COAP_OPTION_URI_PATH,
                             location->name,
                             location->size) )
        {
            return NBIOT_ERR_NO_MEMORY;
        }
        else
        {
            location = location->next;
        }
    }
#endif

    return NBIOT_ERR_OK;
}

int nbiot_deregister( nbiot_device_t *dev,
                      uint16_t        mid,
                      const uint8_t  *token,
                      uint8_t         token_len,
                      uint8_t        *buffer,
                      size_t          buffer_len )
{
    int len;
    coap_t coap[1];

    /* header */
    coap->buffer = buffer;
    coap->size = buffer_len;
    if ( coap_init_header(coap,
                          COAP_TYPE_CON,
                          COAP_DELETE,
                          mid,
                          token,
                          token_len) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    if ( nbiot_add_location(dev,coap) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    len = nbiot_send_buffer( dev->socket,
                             dev->server,
                             coap->buffer,
                             coap->offset );
    if ( len != coap->offset )
    {
        return NBIOT_ERR_SOCKET;
    }

    dev->state = STATE_DEREG_PENDING;
    return NBIOT_ERR_OK;
}
