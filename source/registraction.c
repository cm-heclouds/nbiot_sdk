/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

#define REGISTER_START  1
#define REGISTER_UPDATE 2
#define DEREGISTER      3
static int nbiot_add_header( nbiot_device_t *dev,
                             uint8_t        *buffer,
                             uint16_t        buffer_len,
                             uint8_t         type,
                             bool            with_objs )
{
    coap_t coap[1];
    uint16_t mid = dev->next_mid++;

    /* header */
    coap->buffer = buffer;
    coap->size = buffer_len;
    if ( coap_init_header(coap,
                          COAP_TYPE_CON,
                          type == DEREGISTER ? COAP_DELETE : COAP_POST,
                          mid,
                          NULL,
                          4) )
    {
        return 0;
    }

    /* token */
    if ( nbiot_init_token(buffer+COAP_HEADER_SIZE,4,mid) )
    {
        return 0;
    }

    /* uri path */
    if ( type == REGISTER_START )
    {
        if ( coap_add_option(coap,
                             COAP_OPTION_URI_PATH,
                             (const uint8_t*)"rd",
                             2) )
        {
            return 0;
        }
    }
    else
    {
#ifdef NBIOT_LOCATION
        nbiot_location_t *tmp = dev->locations;
        while ( tmp )
        {
            if ( coap_add_option(coap,
                                 COAP_OPTION_URI_PATH,
                                 tmp->name,
                                 tmp->size) )
            {
                return 0;
            }
            else
            {
                tmp = tmp->next;
            }
        }
#else
        const char *str;

        /* rd */
        if ( coap_add_option(coap,
                             COAP_OPTION_URI_PATH,
                             (const uint8_t*)"rd",
                             2) )
        {
            return 0;
        }

        /* endpoint name */
        str = nbiot_strrchr( dev->endpoint_name, -1, ';' );
        if ( coap_add_option(coap,
                             COAP_OPTION_URI_PATH,
                             (const uint8_t*)dev->endpoint_name,
                             str ? str-dev->endpoint_name : nbiot_strlen(dev->endpoint_name)) )
        {
            return 0;
        }
#endif
    }

    if ( with_objs && type != DEREGISTER )
    {
        /* content type */
        if ( coap_add_content_type(coap,LWM2M_CONTENT_LINK) )
        {
            return 0;
        }
    }

    if ( type == REGISTER_START )
    {
        /* endpoint name */
        if ( nbiot_add_uri_query( coap,
                                  "ep=",
                                  dev->endpoint_name ) )
        {
            return 0;
        }

        /* binding */
        if ( nbiot_add_uri_query(coap,"b=","U") )
        {
            return 0;
        }

        /* life time */
        if ( dev->life_time > 0 )
        {
            char temp[6];

            nbiot_add_integer( dev->life_time,
                               temp,
                               sizeof( temp ) );
            if ( nbiot_add_uri_query(coap,
                                     "lt=",
                                     temp) )
            {
                return 0;
            }
        }
    }

    return coap->offset;
}

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

static int nbiot_register( nbiot_device_t *dev,
                           uint8_t        *buffer,
                           size_t          buffer_len,
                           uint8_t         type,
                           bool            with_objs )
{
    int rc;
    uint16_t offset = 0;

    rc = nbiot_add_header( dev,
                           buffer,
                           buffer_len - offset,
                           type,
                           with_objs );
    if ( !rc )
    {
        return 0;
    }

    /* payloads */
    offset += rc;
    buffer += rc;
    if ( with_objs )
    {
        nbiot_node_t *obj;
        for ( obj = dev->nodes;
              obj != NULL;
              obj = obj->next )
        {
            nbiot_node_t *inst;
            for ( inst = (nbiot_node_t*)obj->data;
                  inst != NULL;
                  inst = inst->next )
            {
                rc = nbiot_add_resource( buffer,
                                         buffer_len - offset,
                                         obj->id,
                                         inst->id,
                                         inst == dev->nodes->data );
                if ( rc )
                {
                    offset += rc;
                    buffer += rc;
                }
                else
                {
                    return 0;
                }
            }
        }
    }

    return offset;
}

#ifdef NOTIFY_ACK
static void registraction_reply( nbiot_device_t    *dev,
                                 const uint8_t     *buffer,
                                 size_t             buffer_len,
                                 bool               ack,
                                 const nbiot_uri_t *uri )
#else
static void registraction_reply( nbiot_device_t *dev,
                                 const uint8_t  *buffer,
                                 size_t          buffer_len,
                                 bool            ack )
#endif
{
    if ( dev->state == STATE_REG_PENDING )
    {
        dev->registraction = nbiot_time();
        if ( COAP_CREATED_201 == coap_code(buffer) )
        {
#ifdef NBIOT_LOCATION
            bool first = true;
            uint16_t location_path_len;
            const char *location_path = (const char*)buffer;

            do
            {
                int ret;
                nbiot_location_t *tmp;

                ret = coap_location_path( buffer,
                                          buffer_len,
                                          &location_path,
                                          &location_path_len,
                                          first );
                if ( ret )
                {
                    if ( first )
                    {
                        first = false;
                    }

                    tmp = (nbiot_location_t*)nbiot_malloc( LOCATION_SIZE(location_path_len) );
                    tmp->next = NULL;
                    tmp->size = (uint8_t)location_path_len;
                    nbiot_memmove( tmp->name, location_path, location_path_len );

                    if ( dev->locations )
                    {
                        nbiot_location_t *tail = dev->locations;
                        while ( tail->next )
                        {
                            tail = tail->next;
                        }
                        tail->next = tmp;
                    }
                    else
                    {
                        dev->locations = tmp;
                    }

                    buffer += ret;
                    buffer_len -= ret;
                }
                else
                {
                    break;
                }
            } while (1);
#endif

            dev->state = STATE_REGISTERED;
        }
        else
        {
            dev->state = STATE_REG_FAILED;
        }
    }
}

int nbiot_register_start( nbiot_device_t *dev,
                          uint8_t        *buffer,
                          size_t          buffer_len )
{
#ifdef NBIOT_BOOTSTRAP
    if ( dev->state == STATE_BS_FINISHED )
#else
    if ( dev->state == STATE_REG_FAILED ||
         dev->state == STATE_DEREGISTERED )
#endif
    {
        int ret;

        buffer_len = nbiot_register( dev,
                                     buffer,
                                     buffer_len,
                                     REGISTER_START,
                                     true );
        if ( !buffer_len )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        ret = nbiot_send_buffer( dev->socket,
                                 dev->server,
                                 buffer,
                                 buffer_len );
        if ( ret <= 0 )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        dev->state = STATE_REG_PENDING;
#ifdef NOTIFY_ACK
        nbiot_transaction_add( dev,
                               buffer,
                               buffer_len,
                               registraction_reply,
                               NULL );
#else
        nbiot_transaction_add( dev,
                               buffer,
                               buffer_len,
                               registraction_reply );
#endif
    }

    return COAP_NO_ERROR;
}

#ifdef NOTIFY_ACK
static void registraction_update_reply( nbiot_device_t    *dev,
                                        const uint8_t     *buffer,
                                        size_t             buffer_len,
                                        bool               ack,
                                        const nbiot_uri_t *uri )
#else
static void registraction_update_reply( nbiot_device_t *dev,
                                        const uint8_t  *buffer,
                                        size_t          buffer_len,
                                        bool            ack )
#endif
{
    if ( dev->state == STATE_REG_UPDATE_PENDING )
    {
        dev->registraction = nbiot_time();
        if ( COAP_CHANGED_204 == coap_code(buffer) )
        {
            dev->state = STATE_REGISTERED;
        }
        else
        {
            dev->state = STATE_REG_FAILED;
        }
    }
}

int nbiot_register_update( nbiot_device_t *dev,
                           uint8_t        *buffer,
                           size_t          buffer_len,
                           bool            with_objs )
{
    if ( dev->state == STATE_REGISTERED ||
         dev->state == STATE_REG_UPDATE_NEEDED )
    {
        int ret;

        buffer_len = nbiot_register( dev,
                                     buffer,
                                     buffer_len,
                                     REGISTER_UPDATE,
                                     with_objs );
        if ( !buffer_len )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        ret = nbiot_send_buffer( dev->socket,
                                 dev->server,
                                 buffer,
                                 buffer_len );
        if ( ret <= 0 )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

#ifdef NOTIFY_ACK
        nbiot_transaction_add( dev,
                               buffer,
                               buffer_len,
                               registraction_update_reply,
                               NULL );
#else
        nbiot_transaction_add( dev,
                               buffer,
                               buffer_len,
                               registraction_update_reply );
#endif
        dev->state = STATE_REG_UPDATE_PENDING;
    }

    return COAP_NO_ERROR;
}

#ifdef NOTIFY_ACK
static void deregister_reply( nbiot_device_t    *dev,
                              const uint8_t     *buffer,
                              size_t             buffer_len,
                              bool               ack,
                              const nbiot_uri_t *uri )
#else
static void deregister_reply( nbiot_device_t *dev,
                              const uint8_t  *buffer,
                              size_t          buffer_len,
                              bool            ack )
#endif
{
    if ( dev->state == STATE_DEREG_PENDING )
    {
        dev->state = STATE_DEREGISTERED;
    }
}

int nbiot_deregister( nbiot_device_t *dev,
                      uint8_t        *buffer,
                      size_t          buffer_len )
{
    if ( dev->state == STATE_REGISTERED ||
         dev->state == STATE_REG_UPDATE_NEEDED ||
         dev->state == STATE_REG_UPDATE_PENDING )
    {
        int ret;

        buffer_len = nbiot_add_header( dev,
                                       buffer,
                                       buffer_len,
                                       DEREGISTER,
                                       false );
        if ( !buffer_len )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        ret = nbiot_send_buffer( dev->socket,
                                 dev->server,
                                 buffer,
                                 buffer_len );
        if ( ret <= 0 )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

#ifdef NOTIFY_ACK
        nbiot_transaction_add( dev,
                               buffer,
                               buffer_len,
                               deregister_reply,
                               NULL );
#else
        nbiot_transaction_add( dev,
                               buffer,
                               buffer_len,
                               deregister_reply );
#endif
        dev->state = STATE_DEREG_PENDING;
    }

    return COAP_NO_ERROR;
}

void nbiot_register_step( nbiot_device_t *dev,
                          time_t          now,
                          uint8_t        *buffer,
                          size_t          buffer_len )
{
    if ( dev->state == STATE_REGISTERED )
    {
        int next_update = dev->life_time;
        if ( COAP_MAX_TRANSMIT_WAIT < next_update )
        {
            next_update -= (int)COAP_MAX_TRANSMIT_WAIT;
        }
        else
        {
            next_update = next_update >> 1;
        }

        if ( dev->registraction + next_update <= now )
        {
            nbiot_register_update( dev,
                                   buffer,
                                   buffer_len,
                                   false );
        }
    }

    if ( dev->state == STATE_REG_UPDATE_NEEDED )
    {
        nbiot_register_update( dev,
                               buffer,
                               buffer_len,
                               true );
    }
}