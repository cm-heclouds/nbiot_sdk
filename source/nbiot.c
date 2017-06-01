/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

static int device_connect( nbiot_device_t *dev,
                           const char     *uri,
                           int             uri_len )
{
    int ret;
    const char *addr;
    const char *port;

    if ( !nbiot_strncmp(uri,"coaps://",8) )
    {
        addr = uri + 8;
    }
    else if ( !nbiot_strncmp(uri,"coap://",7) )
    {
        addr = uri + 7;
    }
    else
    {
        return NBIOT_ERR_INVALID_URI;
    }

    port = nbiot_strrchr( uri, uri_len, ':' );
    if ( !port )
    {
        return NBIOT_ERR_INVALID_URI;
    }

    addr = nbiot_strdup( addr, port - addr );
    if ( !addr )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    if ( uri_len > 0 )
    {
        uri_len -= (port - uri + 1);
    }

    ret = nbiot_udp_connect( dev->socket,
                             addr,
                             nbiot_atoi(++port,uri_len),
                             &dev->server );
    nbiot_free( (void*)addr );
    if ( ret )
    {
        return ret;
    }

    return NBIOT_ERR_OK;
}

int nbiot_device_create( nbiot_device_t **dev,
                         const char      *endpoint_name,
                         int              life_time,
                         uint16_t         local_port )
{
    int ret;
    nbiot_device_t *tmp;

    tmp = (nbiot_device_t*)nbiot_malloc( sizeof(nbiot_device_t) );
    if ( !tmp )
    {
        return NBIOT_ERR_NO_MEMORY;
    }
    else
    {
        nbiot_memzero( tmp, sizeof(nbiot_device_t) );
    }

    ret = nbiot_udp_create( &tmp->socket );
    if ( ret )
    {
        nbiot_free( tmp );

        return ret;
    }

    ret = nbiot_udp_bind( tmp->socket, NULL, local_port );
    if ( ret )
    {
        nbiot_udp_close( tmp->socket );
        nbiot_free( tmp );

        return ret;
    }

    *dev = tmp;
    tmp->next_mid = nbiot_rand();
    tmp->life_time = life_time;
    tmp->endpoint_name = endpoint_name;

    return NBIOT_ERR_OK;
}

#ifdef NBIOT_LOCATION
static void device_free_locations( nbiot_device_t *dev )
{
    nbiot_location_t *next;
    nbiot_location_t *location = dev->locations;

    while ( location )
    {
        next = location->next;
        nbiot_free( location );
        location = next;
    }
    dev->locations = NULL;
}
#endif

static void device_free_nodes( nbiot_device_t *dev )
{
    nbiot_node_t *obj;
    nbiot_node_t *inst;

    obj = dev->nodes;
    while ( obj )
    {
        inst = (nbiot_node_t*)obj->data;
        while ( inst )
        {
            NBIOT_LIST_FREE( inst->data );
            inst = inst->next;
        }

        NBIOT_LIST_FREE( obj->data );
        obj = obj->next;
    }

    NBIOT_LIST_FREE( dev->nodes );
    dev->nodes = NULL;
}

static void device_free_observes( nbiot_device_t *dev )
{
    nbiot_node_t *obj;

    obj = dev->observes;
    while ( obj )
    {
        NBIOT_LIST_FREE( obj->data );
        obj = obj->next;
    }

    NBIOT_LIST_FREE( dev->observes );
    dev->observes = NULL;
}

void nbiot_device_destroy( nbiot_device_t *dev )
{
    nbiot_sockaddr_destroy( dev->address );
    nbiot_sockaddr_destroy( dev->server );
    nbiot_udp_close( dev->socket );
#ifdef NBIOT_LOCATION
    device_free_locations( dev );
#endif
    device_free_observes( dev );
    device_free_nodes( dev );
    nbiot_free( dev );
}

static void handle_observe( nbiot_device_t    *dev,
                            const nbiot_uri_t *uri,
                            coap_t            *coap,
                            const uint8_t     *token,
                            uint8_t            token_len )
{
    do
    {
        int ret;
        nbiot_node_t *node;
        nbiot_observe_t *tmp;

        node = nbiot_node_find( dev, uri );
        if ( !node )
        {
            coap_set_code( coap, COAP_NOT_FOUND_404 );
            break;
        }

        tmp = nbiot_observe_add( dev,
                                 uri,
                                 token,
                                 token_len );
        if ( !tmp )
        {
            coap_set_code( coap, COAP_BAD_REQUEST_400 );
            break;
        }

        ret = nbiot_observe_read( dev,
                                  uri,
                                  tmp,
                                  coap,
                                  false );
        coap_set_code( coap, ret );
    } while (0);
}

#ifdef NBIOT_BOOTSTRAP
static void handle_bootstrap( nbiot_device_t    *dev,
                              const nbiot_uri_t *uri,
                              coap_t            *coap,
                              const uint8_t     *payload,
                              size_t             payload_len,
                              const char       **svr_uri,
                              uint16_t          *svr_uri_len )
{
    do
    {
        if ( uri->objid || !(uri->flag&NBIOT_SET_INSTID) )
        {
            coap_set_code( coap, COAP_METHOD_NOT_ALLOWED_405 );
            break;
        }

        if ( uri->flag & NBIOT_SET_RESID )
        {
            if ( uri->resid )
            {
                coap_set_code( coap, COAP_METHOD_NOT_ALLOWED_405 );
                break;
            }

            *svr_uri = (const char*)payload;
            *svr_uri_len = payload_len;
            dev->state = STATE_BS_FINISHED;
            coap_set_code( coap, COAP_CHANGED_204 );
        }
        else
        {
            coap_set_code( coap, COAP_BAD_REQUEST_400 );
            while ( payload_len )
            {
                int ret;
                uint16_t id;
                size_t value_len;
                const uint8_t *value;

                ret = nbiot_tlv_decode( payload,
                                        payload_len,
                                        NULL,
                                        &id,
                                        &value,
                                        &value_len );
                if ( ret )
                {
                    payload += ret;
                    payload_len -= ret;
                    if ( id )
                    {
                        coap_set_code( coap, COAP_METHOD_NOT_ALLOWED_405 );
                    }
                    else
                    {
                        *svr_uri = (const char*)value;
                        *svr_uri_len = value_len;
                        dev->state = STATE_BS_FINISHED;
                        coap_set_code( coap, COAP_CHANGED_204 );
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    } while (0);
}
#endif

static void handle_request( nbiot_device_t *dev,
                            uint16_t        code,
                            uint8_t        *buffer,
                            size_t          buffer_len,
                            size_t          max_buffer_len )
{
    int ret;
    coap_t coap[1];
    nbiot_uri_t uri[1];
    const uint8_t *token = NULL;
    uint32_t observe = UINT32_MAX;
    uint8_t type = coap_type( buffer );
    uint8_t token_len = coap_token( buffer, &token );
#ifdef NBIOT_BOOTSTRAP
    const uint8_t *payload = NULL;
    const char *svr_uri = NULL;
    uint16_t payload_len = 0;
    uint16_t svr_uri_len = 0;

    /* payload */
    coap_payload( buffer,
                  buffer_len,
                  &payload,
                  &payload_len );
#endif
    /* observe */
    coap_int_option( buffer,
                     buffer_len,
                     COAP_OPTION_OBSERVE,
                     &observe );

    /* response */
    coap->option = 0;
    coap->buffer = buffer;
    coap->size = max_buffer_len;
    coap->offset = COAP_HEADER_SIZE + token_len;
    if ( COAP_TYPE_CON == type )
    {
        coap_set_type( coap, COAP_TYPE_ACK );
    }
    else
    {
        coap_set_type( coap, COAP_TYPE_NON );
        coap_set_mid( coap, dev->next_mid++ );
    }

    do
    {
        ret = nbiot_uri_decode( uri,
                                buffer,
                                buffer_len );
        if ( ret )
        {
            coap_set_code( coap, COAP_BAD_REQUEST_400 );
            break;
        }

        if ( COAP_GET == code )
        {
            /* observe */
            if ( observe == 0 )
            {
                handle_observe( dev,
                                uri,
                                coap,
                                token,
                                token_len );
                break;
            }

#ifdef NBIOT_CANCEL_OBSERVE
            if ( observe == 1 )
            {
                /* cancel observe */
                coap_set_code( coap, nbiot_observe_del(dev,uri) );
                break;
            }
#endif
        }
#ifdef NBIOT_BOOTSTRAP
        if ( COAP_PUT == code )
        {
            if ( dev->state == STATE_BS_PENDING )
            {
                handle_bootstrap( dev,
                                  uri,
                                  coap,
                                  payload,
                                  payload_len,
                                  &svr_uri,
                                  &svr_uri_len );
                break;
            }
        }
#endif

        /* discover */
        /* read */
        /* execute */
        /* write - partial update */
        /* create */
        /* attributes */
        /* write - replace */
        /* delete */
        coap_set_code( coap, COAP_METHOD_NOT_ALLOWED_405 );
        break;
    } while (0);

    nbiot_send_buffer( dev->socket,
                       dev->server,
                       coap->buffer,
                       coap->offset );

#ifdef NBIOT_BOOTSTRAP
    /* try to connect server */
    if ( svr_uri && dev->state == STATE_BS_FINISHED )
    {
        device_connect( dev, svr_uri, svr_uri_len );
    }
#endif
}

int nbiot_device_connect( nbiot_device_t *dev,
                          const char     *server_uri,
                          int             timeout )
{
    int ret;
    uint8_t buffer[NBIOT_SOCK_BUF_SIZE];

    ret = device_connect( dev,
                          server_uri,
                          -1 );
    if ( ret )
    {
        return ret;
    }

    if ( dev->state == STATE_REG_FAILED ||
         dev->state == STATE_DEREGISTERED )
    {
        int len;
        int times;
        time_t last;
        time_t curr;
        uint8_t token[4];
        uint16_t mid = dev->next_mid++;

        times = 0;
        last = nbiot_time();
        nbiot_init_token( token, sizeof(token), mid );

        do
        {
            times++; /* retry 3 times */
#ifdef NBIOT_BOOTSTRAP
            if ( dev->state == STATE_REG_PENDING )
            {
                ret = nbiot_register_start( dev,
                                            mid,
                                            token,
                                            sizeof(token),
                                            buffer,
                                            sizeof(buffer) );
            }
            else
            {
                ret = nbiot_bootstrap_start( dev,
                                             mid,
                                             token,
                                             sizeof(token),
                                             buffer,
                                             sizeof(buffer) );
            }
#else
            ret = nbiot_register_start( dev,
                                        mid,
                                        token,
                                        sizeof(token),
                                        buffer,
                                        sizeof(buffer) );
#endif
            if ( ret )
            {
                return ret;
            }

            do
            {
                curr = nbiot_time();
                len = nbiot_recv_buffer( dev->socket,
                                        &dev->address,
                                         buffer,
                                         sizeof(buffer) );
                if ( len > 0 &&
                     nbiot_sockaddr_equal(dev->address,dev->server) )
                {
                    uint16_t code = coap_code( buffer );
#ifdef NBIOT_BOOTSTRAP
                    if ( COAP_GET <= code && COAP_DELETE >= code )
                    {
                        handle_request( dev,
                                        code,
                                        buffer,
                                        len,
                                        sizeof(buffer) );
                    }
                    else
#endif
                    {
                        const uint8_t *src_token;
                        uint16_t src_token_len = coap_token( buffer, &src_token );
                        if ( src_token_len == sizeof(token) &&
                             nbiot_memcmp( token, src_token, src_token_len ) == 0 )
                        {
                            uint8_t type = coap_type( buffer );
#ifdef NBIOT_BOOTSTRAP
                            if ( dev->state == STATE_BS_INITIATED )
                            {
                                if ( type == COAP_TYPE_ACK &&
                                     code == COAP_CHANGED_204 )
                                {
                                    dev->state = STATE_BS_PENDING;
                                }
                                else
                                {
                                   dev->state = STATE_BS_FAILED;
                                }
                            }
                            else
#endif
                            if ( type == COAP_TYPE_ACK &&
                                 code == COAP_CREATED_201 )
                            {
#ifdef NBIOT_LOCATION
                                bool first = true;
                                uint16_t location_path_len;
                                const char *location_path = buffer;

                                do
                                {
                                    int ret;
                                    nbiot_location_t *tmp;

                                    ret = coap_option( location_path,
                                                       len,
                                                       COAP_OPTION_LOCATION_PATH,
                                                       (const uint8_t**)&location_path,
                                                       &location_path_len,
                                                       first );
                                    if ( ret )
                                    {
                                        if ( first )
                                        {
                                            first = false;
                                        }

                                        tmp = (nbiot_location_t*)nbiot_malloc( LOCATION_SIZE( location_path_len ) );
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

                                        len -= ret;
                                        location_path += location_path_len;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                } while ( 1 );
#endif
                                dev->state = STATE_REGISTERED;
                            }
                            else
                            {
                                dev->state = STATE_REG_FAILED;
                            }
                        }
                    }
                }

#ifdef NBIOT_BOOTSTRAP
                if ( dev->state == STATE_BS_FINISHED )
                {
                    /* registraction */
                    mid = dev->next_mid++;
                    nbiot_init_token( token, sizeof(token), mid );
                    ret = nbiot_register_start( dev,
                                                mid,
                                                token,
                                                sizeof(token),
                                                buffer,
                                                sizeof(buffer) );
                    if ( ret )
                    {
                        return NBIOT_ERR_REG_FAILED;
                    }
                }

                if ( dev->state == STATE_BS_FAILED )
                {
                    return NBIOT_ERR_BS_FAILED;
                }
#endif

                /* ok */
                if ( STATE_REGISTERED == dev->state )
                {
                    return NBIOT_ERR_OK;
                }

                /* failed */
                if ( STATE_REG_FAILED == dev->state )
                {
                    return NBIOT_ERR_REG_FAILED;
                }

                nbiot_sleep( 100 );
            } while ( curr <= last + timeout*times / 3 );
        } while ( curr <= last + timeout );

        return NBIOT_ERR_TIMEOUT;
    }

    return STATE_ERROR( dev );
}

void nbiot_device_close( nbiot_device_t *dev,
                         int             timeout )
{
    if ( dev->state == STATE_REGISTERED ||
         dev->state == STATE_REG_UPDATE_NEEDED ||
         dev->state == STATE_REG_UPDATE_PENDING )
    {
        int ret;
        int len;
        int times;
        time_t last;
        time_t curr;
        uint8_t token[4];
        uint16_t mid = dev->next_mid++;
        uint8_t buffer[NBIOT_SOCK_BUF_SIZE];

        times = 0;
        last = nbiot_time();
        nbiot_init_token( token, sizeof(token), mid );

        do
        {
            times++; /* retry 3 times */
            ret = nbiot_deregister( dev,
                                    mid,
                                    token,
                                    sizeof(token),
                                    buffer,
                                    sizeof(buffer) );
            if ( ret )
            {
                break;
            }

            do
            {
                curr = nbiot_time();
                len = nbiot_recv_buffer( dev->socket,
                                        &dev->address,
                                         buffer,
                                         sizeof( buffer ) );
                if ( len > 0 &&
                     nbiot_sockaddr_equal(dev->address,dev->server) )
                {
                    const uint8_t *src_token;
                    uint16_t src_token_len = coap_token( buffer, &src_token );

                    if ( src_token_len == sizeof(token) &&
                         nbiot_memcmp(token,src_token,src_token_len) == 0 )
                    {
                        dev->state = STATE_DEREGISTERED;
                    }
                }

                /* ok */
                if ( dev->state == STATE_REG_FAILED ||
                     dev->state == STATE_DEREGISTERED ||
                     dev->state == STATE_SERVER_RESET )
                {
                    break;
                }

                /* sleep */
                nbiot_sleep( 100 );
            } while ( curr - last <= timeout*times/3 );
        } while ( curr - last <= timeout );
    }
}

int nbiot_device_step( nbiot_device_t *dev,
                       int             timeout )
{
    if ( dev->state == STATE_REGISTERED ||
         dev->state == STATE_REG_UPDATE_NEEDED ||
         dev->state == STATE_REG_UPDATE_PENDING )
    {
        int len;
        time_t last;
        time_t curr;
        uint8_t buffer[NBIOT_SOCK_BUF_SIZE];

        last = nbiot_time();
        do
        {
            len = nbiot_recv_buffer( dev->socket,
                                    &dev->address,
                                     buffer,
                                     sizeof(buffer) );
            if ( len > 0 &&
                 nbiot_sockaddr_equal(dev->address,dev->server) )
            {
                uint16_t code = coap_code( buffer );
                if ( COAP_GET <= code && COAP_DELETE >= code )
                {
                    handle_request( dev,
                                    code,
                                    buffer,
                                    len,
                                    sizeof(buffer) );
                }
            }

            curr = nbiot_time();
            nbiot_observe_step( dev,
                                curr,
                                buffer,
                                sizeof(buffer) );
            nbiot_sleep( 100 );
        } while ( curr <= last + timeout );
    }

    return STATE_ERROR( dev );
}

int nbiot_resource_add( nbiot_device_t *dev,
                        uint16_t        objid,
                        uint16_t        instid,
                        uint16_t        resid,
                        nbiot_value_t  *data )
{
    if ( dev->state == STATE_REG_FAILED ||
         dev->state == STATE_DEREGISTERED )
    {
        nbiot_node_t *node;
        nbiot_node_t *parent;

        node = (nbiot_node_t*)NBIOT_LIST_GET( dev->nodes, objid );
        if ( !node )
        {
            node = (nbiot_node_t*)nbiot_malloc( sizeof(nbiot_node_t) );
            if ( !node )
            {
                return NBIOT_ERR_NO_MEMORY;
            }

            nbiot_memzero( node, sizeof(nbiot_node_t) );
            node->id = objid;
            dev->nodes = (nbiot_node_t*)NBIOT_LIST_ADD( dev->nodes, node );
        }

        parent = node;
        node = (nbiot_node_t*)NBIOT_LIST_GET( parent->data, instid );
        if ( !node )
        {
            node = (nbiot_node_t*)nbiot_malloc( sizeof(nbiot_node_t) );
            if ( !node )
            {
                return NBIOT_ERR_NO_MEMORY;
            }

            nbiot_memzero( node, sizeof(nbiot_node_t) );
            node->id = instid;
            parent->data = NBIOT_LIST_ADD( parent->data, node );
        }

        parent = node;
        node = (nbiot_node_t*)NBIOT_LIST_GET( parent->data, instid );
        if ( !node )
        {
            node = (nbiot_node_t*)nbiot_malloc( sizeof(nbiot_node_t) );
            if ( !node )
            {
                return NBIOT_ERR_NO_MEMORY;
            }

            nbiot_memzero( node, sizeof(nbiot_node_t) );
            node->id = resid;
            parent->data = NBIOT_LIST_ADD( parent->data, node );
        }

        /* not free */
        node->data = data;
        return NBIOT_ERR_OK;
    }

    return NBIOT_ERR_REGISTERED;
}

int nbiot_resource_del( nbiot_device_t *dev,
                        uint16_t        objid,
                        uint16_t        instid,
                        uint16_t        resid )
{
    return NBIOT_ERR_NOT_SUPPORT;
}

void nbiot_device_sync( nbiot_device_t *dev )
{
    /* nothing */
}