/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

nbiot_observe_t* nbiot_observe_add( nbiot_device_t    *dev,
                                    const nbiot_uri_t *uri,
                                    const uint8_t     *token,
                                    uint8_t            token_len )
{
    if ( uri->flag & NBIOT_SET_RESID )
    {
        return NULL;
    }

    if ( uri->flag & NBIOT_SET_INSTID )
    {
        nbiot_node_t *obj;
        bool obj_new = false;
        nbiot_observe_t *inst;

        obj = (nbiot_node_t*)NBIOT_LIST_GET( dev->observes, uri->objid );
        if ( !obj )
        {
            obj = (nbiot_node_t*)nbiot_malloc( sizeof(nbiot_node_t) );
            if ( !obj )
            {
                return NULL;
            }

            obj_new = true;
            nbiot_memzero( obj, sizeof(nbiot_node_t) );
            obj->id = uri->objid;
            dev->observes = (nbiot_node_t*)NBIOT_LIST_ADD( dev->observes, obj );
        }

        inst = (nbiot_observe_t*)NBIOT_LIST_GET( obj->data, uri->instid );
        if ( !inst )
        {
            inst = (nbiot_observe_t*)nbiot_malloc( sizeof(nbiot_observe_t) );
            if ( !inst )
            {
                if ( obj_new )
                {
                    dev->observes = (nbiot_node_t*)NBIOT_LIST_DEL( dev->observes, uri->objid, NULL );
                    nbiot_free( obj );
                }

                return NULL;
            }

            nbiot_memzero( inst, sizeof(nbiot_observe_t) );
            inst->id = uri->instid;
            obj->data = NBIOT_LIST_ADD( obj->data, inst );
        }

        inst->active = true;
        inst->token_len = token_len;
        nbiot_memmove( inst->token, token, token_len );

        return inst;
    }

    return NULL;
}

#ifdef NBIOT_CANCEL_OBSERVE
int nbiot_observe_del( nbiot_device_t    *dev,
                       const nbiot_uri_t *uri )
{
    if ( uri->flag & NBIOT_SET_RESID )
    {
        return COAP_METHOD_NOT_ALLOWED_405;
    }

    if ( uri->flag & NBIOT_SET_INSTID )
    {
        nbiot_node_t *obj;
        nbiot_observe_t *inst;

        obj = (nbiot_node_t*)NBIOT_LIST_GET( dev->observes, uri->objid );
        if ( !obj )
        {
            return COAP_NOT_FOUND_404;
        }

        obj->data = NBIOT_LIST_DEL( obj->data, uri->instid, &inst );
        if ( !inst )
        {
            return COAP_NOT_FOUND_404;
        }

        nbiot_free( inst );
        if ( !obj->data )
        {
            dev->observes = (nbiot_node_t*)NBIOT_LIST_DEL( dev->observes, uri->objid, NULL );
            nbiot_free( obj );
        }

        return COAP_CONTENT_205;
    }

    return COAP_BAD_REQUEST_400;
}
#endif

int nbiot_observe_read( nbiot_device_t    *dev,
                        const nbiot_uri_t *uri,
                        nbiot_observe_t   *observe,
                        coap_t            *coap,
                        bool               update )
{
    int len;
    nbiot_node_t *node;
    nbiot_node_t *child;

    node = nbiot_node_find( dev, uri );
    if ( !node )
    {
        return COAP_NOT_FOUND_404;
    }

    /* check */
    if ( update )
    {
        for ( child = (nbiot_node_t*)node->data;
              child != NULL;
              child = child->next )
        {
            if ( ((nbiot_value_t*)child->data)->flag&NBIOT_UPDATED )
            {
                break;
            }
        }

        if ( !child )
        {
            return COAP_IGNORE;
        }
    }

    /* observe */
    if ( coap_add_int_option(coap,
                             COAP_OPTION_OBSERVE,
                             observe->counter++) )
    {
        return COAP_INTERNAL_SERVER_ERROR_500;
    }

    /* content type */
    if ( coap_add_int_option(coap,
                             COAP_OPTION_CONTENT_TYPE,
                             LWM2M_CONTENT_TLV) )
    {
        return COAP_INTERNAL_SERVER_ERROR_500;
    }

    /* payload */
    if ( coap->size > coap->offset )
    {
        coap->buffer[coap->offset++] = 0xff;
    }
    else
    {
        return COAP_INTERNAL_SERVER_ERROR_500;
    }

    /* read */
    len = nbiot_node_read( node,
                           uri->flag,
                           coap->buffer + coap->offset,
                           coap->size - coap->offset,
                           update );
    if ( len )
    {
        coap->offset += len;
    }
    else
    {
        return COAP_INTERNAL_SERVER_ERROR_500;
    }

    return COAP_CONTENT_205;
}

void nbiot_observe_step( nbiot_device_t *dev,
                         time_t          now,
                         uint8_t        *buffer,
                         size_t          buffer_len )
{
    int ret;
    coap_t coap[1];
    nbiot_uri_t uri[1];
    nbiot_node_t *obj;
    nbiot_observe_t *inst;

    for ( obj = dev->observes;
          obj != NULL;
          obj = obj->next )
    {
        uri->objid = obj->id;
        uri->flag = NBIOT_SET_OBJID;

        for ( inst = (nbiot_observe_t*)obj->data;
              inst != NULL;
              inst = inst->next )
        {
            uri->instid = inst->id;
            uri->flag |= NBIOT_SET_INSTID;

            if ( inst->active )
            {
                coap->buffer = buffer;
                coap->size = buffer_len;

                /* header */
                if ( coap_init_header(coap,
                                      COAP_TYPE_NON,
                                      COAP_CONTENT_205,
                                      inst->lastmid = dev->next_mid++,
                                      inst->token,
                                      inst->token_len) )
                {
                    continue;
                }

                ret = nbiot_observe_read( dev,
                                          uri,
                                          inst,
                                          coap,
                                          true );
                if ( ret != COAP_CONTENT_205 )
                {
                    continue;
                }

                nbiot_send_buffer( dev->socket,
                                   dev->server,
                                   coap->buffer,
                                   coap->offset );
            }
        }
    }
}