/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "m2m.h"
#include "struct.h"

uint8_t lwm2m_buffer_send( void    *session,
                           uint8_t *buffer,
                           size_t   length,
                           void     *userdata )
{
#ifdef HAVE_DTLS
    int ret;
    connection_t *conn;
#else
    int ret;
    size_t sent;
    size_t offset;
    connection_t *conn;
    nbiot_device_t *data;
#endif

    if ( NULL == session ||
         NULL == buffer ||
         NULL == userdata )
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

#ifdef HAVE_DTLS
    conn = (connection_t*)session;
    ret = dtls_write( conn->dtls,
                      conn->addr,
                      buffer,
                      length );
    if ( -1 == ret )
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }
#else
    offset = 0;
    conn = (connection_t*)session;
    data = (nbiot_device_t*)userdata;
    while ( offset < length )
    {
        ret = nbiot_udp_send( data->sock,
                              buffer + offset,
                              length - offset,
                              &sent,
                              conn->addr );
        if ( ret < 0 )
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        else
        {
            offset += sent;
        }
    }
#endif

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal( void *session1,
                             void *session2,
                             void *userdata )
{
    return (session1 == session2);
}

void* lwm2m_connect_server( uint16_t sec_instid,
                            void    *userdata )
{
    char *uri;
    char *addr;
    char *port;
    connection_t *conn;
    nbiot_device_t *dev;
    lwm2m_object_t *sec_obj;

    dev = (nbiot_device_t*)userdata;
    if ( NULL == dev )
    {
        return NULL;
    }

    sec_obj = nbiot_object_find( dev, LWM2M_SECURITY_OBJECT_ID );
    if ( NULL == sec_obj )
    {
        return NULL;
    }

    uri = get_server_uri( sec_obj, sec_instid );
    if ( NULL == uri )
    {
        return NULL;
    }

    /* parse uri in the form "coaps://[host]:[port]" */
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
        nbiot_free( uri );

        return NULL;
    }

    port = nbiot_strrchr( addr, ':' );
    if ( NULL == port )
    {
        nbiot_free( uri );

        return NULL;
    }

    /* split string */
    *port = '\0';
    ++port;

    conn = connection_create( dev->connlist,
                              dev->sock,
                              addr,
                              nbiot_atoi(port) );
    if ( NULL != conn )
    {
#ifdef HAVE_DTLS
        conn->dev = dev;
        conn->lwm2m = dev->lwm2m;
#endif
        dev->connlist = conn;
    }

    nbiot_free( uri );
    return conn;
}

void lwm2m_close_connection( void *session,
                             void *userdata )
{
    connection_t *conn;
    nbiot_device_t *dev;

    if ( NULL == session ||
         NULL == userdata )
    {
        return;
    }

    conn = (connection_t*)session;
    dev = (nbiot_device_t*)userdata;
    dev->connlist = connection_remove( dev->connlist, conn );
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

    conn = (connection_t*)ctx->app;
    if ( NULL == conn ||
         NULL == conn->dev )
    {
        return -1;
    }

    offset = 0;
    while ( offset < len )
    {
        ret = nbiot_udp_send( conn->dev->sock,
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

    conn = (connection_t*)ctx->app;
    if ( NULL == conn ||
         NULL == conn->lwm2m )
    {
        return -1;
    }
    else
    {
        lwm2m_handle_packet( conn->lwm2m,
                             data,
                             len,
                             conn );
    }

    return 0;
}

static int get_ecdsa_key( dtls_context_t *ctx,
                          const session_t *session,
                          const dtls_ecdsa_key_t **result )
{
    *result = NULL;
    return 0;
}

static int verify_ecdsa_key( dtls_context_t *ctx,
                             const session_t *session,
                             const unsigned char *other_pub_x,
                             const unsigned char *other_pub_y,
                             size_t key_size )
{
    return 0;
}

static dtls_handler_t dtls_cb =
{
    .write = send_to_peer,
    .read = read_from_peer,
    .event = NULL,
    .get_ecdsa_key = get_ecdsa_key,
    .verify_ecdsa_key = verify_ecdsa_key
};
#endif

connection_t*
connection_create( connection_t   *connlist,
                   nbiot_socket_t *sock,
                   const char     *addr,
                   uint16_t        port )
{
    int ret;
    connection_t *conn;

    if ( NULL == sock )
    {
        return connlist;
    }

    conn = (connection_t*)nbiot_malloc( sizeof(connection_t) );
    if ( NULL == conn )
    {
        return connlist;
    }

    conn->addr = NULL;
    ret = nbiot_udp_connect( sock,
                             addr,
                             port,
                             &conn->addr );
    if ( ret )
    {
        nbiot_free( conn );

        return connlist;
    }

#ifdef HAVE_DTLS
    dtls_init();
    conn->dtls = dtls_new_context( conn );
    if ( NULL == conn->dtls )
    {
        nbiot_sockaddr_destroy( conn->addr );
        nbiot_free( conn );

        return connlist;
    }
    else
    {
        dtls_set_handler( conn->dtls, &dtls_cb );
    }
    conn->lwm2m = NULL;
#endif

    conn->next = connlist;
    connlist = conn;

    return connlist;
}

connection_t*
connection_find( connection_t           *connlist,
                 const nbiot_sockaddr_t *addr )
{
    connection_t *conn;

    if ( NULL == connlist ||
         NULL == addr )
    {
        return NULL;
    }

    conn = connlist;
    while ( NULL != conn )
    {
        if ( nbiot_sockaddr_equal(conn->addr,addr) )
        {
            return conn;
        }
        else
        {
            conn = conn->next;
        }
    }

    return NULL;
}

connection_t*
connection_remove( connection_t *connlist,
                   connection_t *conn )
{
    connection_t *prev;

    if ( NULL == connlist ||
         NULL == conn )
    {
        return connlist;
    }

    if ( conn == connlist )
    {
        connlist = conn->next;
    }
    else
    {
        prev = connlist;
        while ( NULL != prev &&
                prev->next != conn )
        {
            prev = prev->next;
        }

        if ( NULL != prev )
        {
            prev->next = conn->next;
        }
    }

#ifdef HAVE_DTLS
    dtls_close( conn->dtls, conn->addr );
#endif
    nbiot_sockaddr_destroy( conn->addr );
    nbiot_free( conn );

    return connlist;
}

void connection_destroy( connection_t *connlist )
{
    while ( NULL != connlist )
    {
        connection_t *conn;

        conn = connlist;
        connlist = conn->next;
#ifdef HAVE_DTLS
        dtls_close( conn->dtls, conn->addr );
#endif
        nbiot_sockaddr_destroy( conn->addr );
        nbiot_free( conn );
    }
}