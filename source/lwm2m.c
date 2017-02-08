/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "lwm2m.h"
#include "struct.h"

void* lwm2m_malloc( size_t size )
{
    return nbiot_malloc( size );
}

void lwm2m_free( void *ptr )
{
    nbiot_free( ptr );
}

char* lwm2m_strdup( const char *str )
{
    return nbiot_strdup( str );
}

int lwm2m_strncmp( const char *str1,
                   const char *str2,
                   size_t      size )
{
    return nbiot_strncmp( str1, str2, size );
}

time_t lwm2m_gettime( void )
{
    return nbiot_time();
}

#ifdef LWM2M_WITH_LOGS
void (*lwm2m_printf)(const char * format, ...) = nbiot_printf;
#endif

uint8_t lwm2m_buffer_send( void    *session,
                           uint8_t *buffer,
                           size_t   length,
                           void     *userdata )
{
    int ret;
    size_t sent;
    size_t offset;
    connection_t *conn;
    nbiot_device_t *data;

    if ( NULL == session ||
         NULL == buffer ||
         NULL == userdata )
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    offset = 0;
    conn = (connection_t*)session;
    data = (nbiot_device_t*)userdata;
    while ( offset < length )
    {
        ret = nbiot_udp_send( data->sock,
                              buffer,
                              length,
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
    if ( !lwm2m_strncmp(uri,"coaps://",8) )
    {
        addr = uri + 8;
    }
    else if ( !lwm2m_strncmp(uri,"coap://",7) )
    {
        addr = uri + 7;
    }
    else
    {
        lwm2m_free( uri );

        return NULL;
    }

    port = nbiot_strrchr( addr, ':' );
    if ( NULL == port )
    {
        lwm2m_free( uri );

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
        dev->connlist = conn;
    }

    lwm2m_free( uri );
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

    conn = (connection_t*)lwm2m_malloc( sizeof(connection_t) );
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
        lwm2m_free( conn );

        return connlist;
    }

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

    nbiot_sockaddr_destroy( conn->addr );
    lwm2m_free( conn );

    return connlist;
}

void connection_destroy( connection_t *connlist )
{
    while ( NULL != connlist )
    {
        connection_t *conn;

        conn = connlist;
        connlist = conn->next;
        nbiot_sockaddr_destroy( conn->addr );
        lwm2m_free( conn );
    }
}