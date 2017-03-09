/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <error.h>
#include <platform.h>
#include <utils.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#ifdef NBIOT_DEBUG
#include <stdio.h>
void output_buffer( uint8_t *buffer, int length )
{
    int i;

    if ( length == 0 ) nbiot_printf( "\n" );

    i = 0;
    while ( i < length )
    {
        uint8_t array[16];
        int j;

        nbiot_memmove( array, buffer + i, 16 );
        for ( j = 0; j < 16 && i + j < length; j++ )
        {
            nbiot_printf( "%02X ", array[j] );
            if ( j % 4 == 3 ) nbiot_printf( " " );
        }
        if ( length > 16 )
        {
            while ( j < 16 )
            {
                nbiot_printf( "   " );
                if ( j % 4 == 3 ) nbiot_printf( " " );
                j++;
            }
        }
        nbiot_printf( " " );
        for ( j = 0; j < 16 && i + j < length; j++ )
        {
            if ( isprint( array[j] ) )
                nbiot_printf( "%c", array[j] );
            else
                nbiot_printf( "." );
        }
        nbiot_printf( "\n" );
        i += 16;
    }
}
#endif

#define INVALID_SOCKET (-1)

struct nbiot_socket_t
{
    int sock;
};

struct nbiot_sockaddr_t
{
    struct sockaddr_in addr;
};

int nbiot_udp_create( nbiot_socket_t **sock )
{
    if ( NULL == sock )
    {
        return NBIOT_ERR_BADPARAM;
    }

    *sock = (nbiot_socket_t*)nbiot_malloc( sizeof(nbiot_socket_t) );
    if ( NULL == *sock )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    (*sock)->sock = INVALID_SOCKET;
    return NBIOT_ERR_OK;
}

int nbiot_udp_close( nbiot_socket_t *sock )
{
    if ( NULL == sock )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( INVALID_SOCKET != sock->sock )
    {
        close( sock->sock );
    }

    nbiot_free( sock );
    return NBIOT_ERR_OK;
}

int nbiot_udp_bind( nbiot_socket_t *sock,
                    const char     *addr,
                    uint16_t        port )
{
    char temp[16];
    unsigned long flag;
    struct addrinfo *p;
    struct addrinfo *res;
    struct addrinfo hints;

    if ( NULL == sock )
    {
        return NBIOT_ERR_BADPARAM;
    }

    nbiot_itoa( temp, port );
    nbiot_memzero( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    sock->sock = INVALID_SOCKET;

    if ( getaddrinfo(addr,temp,&hints,&res) )
    {
        return NBIOT_ERR_INTERNAL;
    }

    if ( NULL == res )
    {
        return NBIOT_ERR_INTERNAL;
    }

    for ( p = res;
          NULL != p && INVALID_SOCKET == sock->sock;
          p = p->ai_next )
    {
        sock->sock = socket( p->ai_family,
                             p->ai_socktype,
                             p->ai_protocol );
        if ( INVALID_SOCKET != sock->sock )
        {
            if ( bind(sock->sock,p->ai_addr,p->ai_addrlen) )
            {
                close( sock->sock );
                sock->sock = INVALID_SOCKET;
            }
        }
    }

    freeaddrinfo( res );
    if ( INVALID_SOCKET == sock->sock )
    {
        return NBIOT_ERR_INTERNAL;
    }

    flag = 1;
    if ( ioctl(sock->sock,FIONBIO,&flag) )
    {
        close(sock->sock);
        sock->sock = INVALID_SOCKET;

        return NBIOT_ERR_INTERNAL;
    }

    return NBIOT_ERR_OK;
}

int nbiot_udp_connect( nbiot_socket_t    *sock,
                       const char        *addr,
                       uint16_t           port,
                       nbiot_sockaddr_t **dest )
{
    int s;
    char temp[16];
    struct addrinfo *p;
    struct addrinfo *res;
    struct addrinfo hints;

    if ( NULL == sock ||
         NULL == dest )
    {
        return NBIOT_ERR_BADPARAM;
    }

    nbiot_itoa( temp, port );
    nbiot_memzero( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ( getaddrinfo(addr,temp,&hints,&res) )
    {
        return NBIOT_ERR_INTERNAL;
    }

    if ( NULL == res )
    {
        return NBIOT_ERR_INTERNAL;
    }

    s = INVALID_SOCKET;
    for ( p = res;
          NULL != p && INVALID_SOCKET == s;
          p = p->ai_next )
    {
        s = socket( p->ai_family,
                    p->ai_socktype,
                    p->ai_protocol );
        if ( INVALID_SOCKET != s )
        {
            if ( connect(s,p->ai_addr,p->ai_addrlen) )
            {
                close( s );
                s = INVALID_SOCKET;
            }
            else
            {
                close( s );
                if ( NULL == *dest )
                {
                    *dest = (nbiot_sockaddr_t*)nbiot_malloc( sizeof(nbiot_sockaddr_t) );
                    if ( NULL == *dest )
                    {
                        freeaddrinfo( res );

                        return NBIOT_ERR_NO_MEMORY;
                    }
                }

                nbiot_memmove( &(*dest)->addr,
                               p->ai_addr,
                               p->ai_addrlen );
                break;
            }
        }
    }

    freeaddrinfo( res );
    return (INVALID_SOCKET == s ? NBIOT_ERR_INTERNAL : NBIOT_ERR_OK);
}

int nbiot_udp_send( nbiot_socket_t         *sock,
                    const void             *buff,
                    size_t                  size,
                    size_t                 *sent,
                    const nbiot_sockaddr_t *dest )
{
    int ret;

    if ( NULL == sock ||
         NULL == buff ||
         NULL == sent ||
         NULL == dest )
    {
        return NBIOT_ERR_BADPARAM;
    }

    *sent = 0;
    ret = sendto( sock->sock,
                  (const char*)buff,
                  (int)size,
                  0,
                  (const struct sockaddr*)&dest->addr,
                  sizeof(dest->addr) );
    if ( ret < 0 )
    {
        if ( EAGAIN != errno &&
             EWOULDBLOCK == errno )
        {
            return NBIOT_ERR_SOCKET;
        }
    }
    else
    {
        *sent = ret;
#ifdef NBIOT_DEBUG
        nbiot_printf( "sendto(len = %d)\n", ret );
        output_buffer( (uint8_t*)buff, ret );
#endif
    }

    return NBIOT_ERR_OK;
}

int nbiot_udp_recv( nbiot_socket_t    *sock,
                    void              *buff,
                    size_t             size,
                    size_t            *read,
                    nbiot_sockaddr_t **src )
{
    int ret;
    socklen_t len;
    struct sockaddr_in addr;

    if ( NULL == sock ||
         NULL == buff ||
         NULL == read ||
         NULL == src )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( NULL == *src )
    {
        *src = (nbiot_sockaddr_t*)nbiot_malloc( sizeof(nbiot_sockaddr_t) );
        if ( NULL == *src )
        {
            return NBIOT_ERR_NO_MEMORY;
        }
        else
        {
            nbiot_memzero( *src, sizeof(nbiot_sockaddr_t) );
        }
    }

    len = sizeof(addr);
    *read = 0;
    ret = recvfrom( sock->sock,
                    (char*)buff,
                    (int)size,
                    0,
                    (struct sockaddr*)&addr,
                    &len );
    if ( ret < 0 )
    {
        if ( EAGAIN != errno &&
             EWOULDBLOCK == errno )
        {
            return NBIOT_ERR_SOCKET;
        }
    }
    else
    {
        *read = ret;
        nbiot_memmove( &(*src)->addr, &addr, len );
#ifdef NBIOT_DEBUG
        nbiot_printf( "recvfrom(len = %d)\n", ret );
        output_buffer( (uint8_t*)buff, ret );
#endif
    }

    return NBIOT_ERR_OK;
}

size_t nbiot_sockaddr_size( const nbiot_sockaddr_t *addr )
{
    return sizeof(addr->addr);
}

bool nbiot_sockaddr_equal( const nbiot_sockaddr_t *addr1,
                           const nbiot_sockaddr_t *addr2 )
{
    if ( NULL == addr1 ||
         NULL == addr2 )
    {
        return false;
    }

    return !nbiot_memcmp( addr1,
                          addr2,
                          sizeof(nbiot_sockaddr_t) );
}

void nbiot_sockaddr_destroy( nbiot_sockaddr_t *s )
{
    if ( NULL != s )
    {
        nbiot_free( s );
    }
}