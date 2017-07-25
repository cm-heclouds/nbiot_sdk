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
    if ( uri->flag & NBIOT_SET_OBJID )
    {
        bool obj_new = false;
        nbiot_observe_t *obj = (nbiot_observe_t*)NBIOT_LIST_GET( dev->observes, uri->objid );
        if ( !obj )
        {
            obj = (nbiot_observe_t*)nbiot_malloc( sizeof(nbiot_observe_t) );
            if ( !obj )
            {
                return NULL;
            }

            obj_new = true;
            nbiot_memzero( obj, sizeof( nbiot_observe_t ) );
            obj->id = uri->objid;
            dev->observes = (nbiot_observe_t*)NBIOT_LIST_ADD( dev->observes, obj );
        }

        if ( uri->flag & NBIOT_SET_OBJID )
        {
            bool inst_new = false;
            nbiot_observe_t *inst = (nbiot_observe_t*)NBIOT_LIST_GET( obj->list, uri->instid );
            if ( !inst )
            {
                inst = (nbiot_observe_t*)nbiot_malloc( sizeof(nbiot_observe_t) );
                if ( !inst )
                {
                    if ( obj_new )
                    {
                        dev->observes = (nbiot_observe_t*)NBIOT_LIST_DEL( dev->observes, obj->id, NULL );
                        nbiot_free( obj );
                    }

                    return NULL;
                }

                inst_new = true;
                nbiot_memzero( inst, sizeof(nbiot_observe_t) );
                inst->id = uri->instid;
                obj->list = (nbiot_observe_t*)NBIOT_LIST_ADD( obj->list, inst );
            }

            if ( uri->flag & NBIOT_SET_RESID )
            {
                nbiot_observe_t *res = (nbiot_observe_t*)NBIOT_LIST_GET( inst->list, uri->resid );
                if ( !res )
                {
                    res = (nbiot_observe_t*)nbiot_malloc( sizeof(nbiot_observe_t) );
                    if ( !res )
                    {
                        if ( inst_new )
                        {
                            obj->list = (nbiot_observe_t*)NBIOT_LIST_DEL( obj->list, inst->id, NULL );
                            nbiot_free( inst );
                        }

                        if ( obj_new )
                        {
                            dev->observes = (nbiot_observe_t*)NBIOT_LIST_DEL( dev->observes, obj->id, NULL );
                            nbiot_free( obj );
                        }

                        return NULL;
                    }

                    nbiot_memzero( res, sizeof(nbiot_observe_t) );
                    res->id = uri->resid;
                    inst->list = (nbiot_observe_t*)NBIOT_LIST_ADD( inst->list, res );
                }

                res->active = true;
                res->lasttime = nbiot_time();
                res->token_len = token_len;
                nbiot_memmove( res->token, token, token_len );

                return res;
            }
            else
            {
                inst->active = true;
                inst->lasttime = nbiot_time();
                inst->token_len = token_len;
                nbiot_memmove( inst->token, token, token_len );

                return inst;
            }
        }
        else
        {
            obj->active = true;
            obj->lasttime = nbiot_time();
            obj->token_len = token_len;
            nbiot_memmove( obj->token, token, token_len );

            return obj;
        }
    }

    return NULL;
}

int nbiot_observe_del( nbiot_device_t    *dev,
                       const nbiot_uri_t *uri )
{
    if ( uri->flag & NBIOT_SET_OBJID )
    {
        nbiot_observe_t *obj = (nbiot_observe_t*)NBIOT_LIST_GET( dev->observes, uri->objid );
        if ( !obj )
        {
            return COAP_NOT_FOUND_404;
        }

        if ( uri->flag & NBIOT_SET_INSTID )
        {
            nbiot_observe_t *inst = (nbiot_observe_t*)NBIOT_LIST_GET( obj->list, uri->instid );
            if ( !inst )
            {
                return COAP_NOT_FOUND_404;
            }

            if ( uri->flag & NBIOT_SET_RESID )
            {
                nbiot_observe_t *res = (nbiot_observe_t*)NBIOT_LIST_GET( inst->list, uri->resid );
                if ( !res )
                {
                    return COAP_NOT_FOUND_404;
                }

                inst->list = (nbiot_observe_t*)NBIOT_LIST_DEL( inst->list, uri->resid, &res );
                nbiot_free( res );
            }

            if ( !inst->list && (!inst->active || !(uri->flag&NBIOT_SET_RESID)) )
            {
                obj->list = (nbiot_observe_t*)NBIOT_LIST_DEL( obj->list, uri->instid, NULL );
                nbiot_free( inst );
            }
        }

        if ( !obj->list && (!obj->active || !(uri->flag&NBIOT_SET_INSTID)) )
        {
            dev->observes = (nbiot_observe_t*)NBIOT_LIST_DEL( dev->observes, uri->objid, NULL );
            nbiot_free( obj );
        }

        return COAP_CONTENT_205;
    }

    return COAP_BAD_REQUEST_400;
}

#ifdef NOTIFY_ACK
static void do_notify_ack( nbiot_device_t    *dev,
                           const uint8_t     *buffer,
                           size_t             buffer_len,
                           bool               ack,
                           const nbiot_uri_t *uri )
{
    nbiot_node_t *node = nbiot_node_find( dev, uri );
    if ( node )
    {
        if ( uri->flag & NBIOT_SET_RESID )
        {
            dev->notify_ack_func( uri->objid,
                                  uri->instid,
                                  uri->resid,
                                  node->data,
                                  ack );
        }
        else if ( uri->flag & NBIOT_SET_OBJID )
        {
            uint16_t *id;
            nbiot_uri_t tmp[1];

            *tmp = *uri;
            if ( tmp->flag & NBIOT_SET_INSTID )
            {
                id = &tmp->resid;
                tmp->flag |= NBIOT_SET_RESID;
            }
            else
            {
                id = &tmp->instid;
                tmp->flag |= NBIOT_SET_INSTID;
            }

            while ( buffer_len )
            {
                int ret;
                size_t value_len;
                nbiot_node_t *child;
                const uint8_t *value;

                ret = nbiot_tlv_decode( buffer,
                                        buffer_len,
                                        NULL,
                                        id,
                                        &value,
                                        &value_len );
                if ( ret )
                {
                    child = (nbiot_node_t*)NBIOT_LIST_GET( node->data, *id );
                    if ( !child )
                    {
                        break;
                    }

                    buffer += ret;
                    buffer_len -= ret;
                    do_notify_ack( dev,
                                   value,
                                   value_len,
                                   ack,
                                   tmp );
                }
            }
        }
    }
}

static void notify_ack( nbiot_device_t    *dev,
                        const uint8_t     *buffer,
                        size_t             buffer_len,
                        bool               ack,
                        const nbiot_uri_t *uri )
{
    if ( dev->notify_ack_func )
    {
        uint16_t payload_len = 0;
        const uint8_t *payload = NULL;

        if ( coap_payload(buffer,
                          buffer_len,
                          &payload,
                          &payload_len) )
        {
            do_notify_ack( dev,
                           payload,
                           payload_len,
                           ack,
                           uri );
        }
    }
}
#endif

static void observe_read( nbiot_device_t    *dev,
                          nbiot_observe_t   *observe,
                          nbiot_node_t      *node,
                          const nbiot_uri_t *uri,
                          uint8_t           *buffer,
                          size_t             buffer_len )
{
    do
    {
        int ret;
        coap_t coap[1];

        coap->buffer = buffer;
        coap->size = buffer_len;
        observe->lasttime = nbiot_time();
        observe->lastmid = dev->next_mid++;

#ifdef NOTIFY_ACK
        ret = coap_init_header( coap,
                                COAP_TYPE_CON,
                                COAP_CONTENT_205,
                                observe->lastmid,
                                observe->token,
                                observe->token_len );
#else
        ret = coap_init_header( coap,
                                COAP_TYPE_NON,
                                COAP_CONTENT_205,
                                observe->lastmid,
                                observe->token,
                                observe->token_len );
#endif
        if ( ret )
        {
            break;
        }

        ret = coap_add_observe( coap, observe->counter++ );
        if ( ret )
        {
            break;
        }

        ret = coap_add_content_type( coap, LWM2M_CONTENT_TLV );
        if ( ret )
        {
            break;
        }

        if ( coap->size > coap->offset )
        {
            coap->buffer[coap->offset++] = 0xff;
        }
        else
        {
            break;
        }

        ret = nbiot_node_read( node,
                               uri->flag,
                               uri->flag,
                               coap->buffer + coap->offset,
                               coap->size - coap->offset,
                               true );
        if ( ret > 0 )
        {
            coap->offset += ret;
        }
        else
        {
            break;
        }

        /* send */
        nbiot_send_buffer( dev->socket,
                           dev->server,
                           coap->buffer,
                           coap->offset );
#ifdef NOTIFY_ACK
        nbiot_transaction_add( dev,
                               coap->buffer,
                               coap->offset,
                               notify_ack,
                               uri );
#endif
    } while(0);
}

static bool observe_check( nbiot_node_t *node, uint8_t flag )
{
    if ( flag & NBIOT_SET_RESID )
    {
        nbiot_value_t *data = node->data;

        return (data->flag & NBIOT_UPDATED);
    }
    else if ( flag & NBIOT_SET_OBJID )
    {
        if ( flag & NBIOT_SET_INSTID )
        {
            flag |= NBIOT_SET_RESID;
        }
        else
        {
            flag |= NBIOT_SET_INSTID;
        }

        for ( node = (nbiot_node_t*)node->data;
              node != NULL;
              node = node->next )
        {
            if ( observe_check(node,flag) )
            {
                return true;
            }
        }
    }

    return false;
}

void nbiot_observe_step( nbiot_device_t *dev,
                         time_t          now,
                         uint8_t        *buffer,
                         size_t          buffer_len )
{
    nbiot_uri_t uri[1];
    nbiot_node_t *node;
    nbiot_observe_t *obj;
    nbiot_observe_t *inst;
    nbiot_observe_t *res;

    for ( obj = dev->observes;
          obj != NULL;
          obj = obj->next )
    {
        uri->objid = obj->id;
        uri->flag = NBIOT_SET_OBJID;

        for ( inst = obj->list;
              inst != NULL;
              inst = inst->next )
        {
            uri->instid = inst->id;
            uri->flag |= NBIOT_SET_INSTID;

            for ( res = inst->list;
                  res != NULL;
                  res = res->next )
            {
                if ( res->active )
                {
                    uri->resid = res->id;
                    uri->flag |= NBIOT_SET_RESID;
                    node = nbiot_node_find( dev, uri );
                    if ( node &&
                         observe_check(node,uri->flag) )
                    {
                        observe_read( dev,
                                      res,
                                      node,
                                      uri,
                                      buffer,
                                      buffer_len );
                    }
                }
            }

            if ( inst->active )
            {
                uri->flag &= ~NBIOT_SET_RESID;
                node = nbiot_node_find( dev, uri );
                if ( node &&
                     observe_check(node,uri->flag))
                {
                    observe_read( dev,
                                  inst,
                                  node,
                                  uri,
                                  buffer,
                                  buffer_len );
                }
            }
        }

        if ( obj->active )
        {
            uri->flag = NBIOT_SET_OBJID;
            node = nbiot_node_find( dev, uri );
            if ( node &&
                 observe_check(node,uri->flag))
            {
                observe_read( dev,
                              obj,
                              node,
                              uri,
                              buffer,
                              buffer_len );
            }
        }
    }
}