/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <error.h>
#include <platform.h>
#include <utils.h>
#include <winsock2.h>
#include <ws2tcpip.h>

struct nbiot_socket_t
{
    SOCKET sock;
};

struct nbiot_sockaddr_t
{
    struct sockaddr_in addr;
};

int nbiot_udp_create( nbiot_socket_t **sock )
{
    if ( !sock )
    {
        return NBIOT_ERR_BADPARAM;
    }

    *sock = (nbiot_socket_t*)nbiot_malloc( sizeof(nbiot_socket_t) );
    if ( !(*sock) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    (*sock)->sock = INVALID_SOCKET;
    return NBIOT_ERR_OK;
}

int nbiot_udp_close( nbiot_socket_t *sock )
{
    if ( !sock )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( INVALID_SOCKET != sock->sock )
    {
        closesocket( sock->sock );
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

    if ( !sock )
    {
        return NBIOT_ERR_BADPARAM;
    }

    nbiot_itoa( port, temp, 16 );
    nbiot_memzero( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    sock->sock = INVALID_SOCKET;

    if ( getaddrinfo(addr,temp,&hints,&res) )
    {
        return NBIOT_ERR_INTERNAL;
    }

    if ( !res )
    {
        return NBIOT_ERR_INTERNAL;
    }

    for ( p = res;
          p && INVALID_SOCKET == sock->sock;
          p = p->ai_next )
    {
        sock->sock = socket( p->ai_family,
                             p->ai_socktype,
                             p->ai_protocol );
        if ( INVALID_SOCKET != sock->sock )
        {
            if ( bind(sock->sock,p->ai_addr,p->ai_addrlen) )
            {
                closesocket( sock->sock );
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
    if ( ioctlsocket(sock->sock,FIONBIO,&flag) )
    {
        closesocket(sock->sock);
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
    char temp[16];
    struct addrinfo *p;
    struct addrinfo *res;
    struct addrinfo hints;
    SOCKET s;

    if ( !sock || !dest )
    {
        return NBIOT_ERR_BADPARAM;
    }

    nbiot_itoa( port, temp, 16 );
    nbiot_memzero( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ( getaddrinfo(addr,temp,&hints,&res) )
    {
        return NBIOT_ERR_INTERNAL;
    }

    if ( !res )
    {
        return NBIOT_ERR_INTERNAL;
    }

    s = INVALID_SOCKET;
    for ( p = res;
          p && INVALID_SOCKET == s;
          p = p->ai_next )
    {
        s = socket( p->ai_family,
                    p->ai_socktype,
                    p->ai_protocol );
        if ( INVALID_SOCKET != s )
        {
            if ( connect(s,p->ai_addr,p->ai_addrlen) )
            {
                closesocket( s );
                s = INVALID_SOCKET;
            }
            else
            {
                closesocket( s );
                if ( !(*dest) )
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

    if ( !sock || !buff || !sent || !dest )
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
        if ( WSAEWOULDBLOCK != WSAGetLastError() )
        {
            return NBIOT_ERR_SOCKET;
        }
    }
    else
    {
        *sent = ret;

#ifdef NBIOT_DEBUG
        nbiot_printf( "sendto(%s:%d len = %d)\n",
                      inet_ntoa( dest->addr.sin_addr ),
                      ntohs( dest->addr.sin_port ),
                      ret );
        nbiot_buffer_printf( buff, ret );
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
    int len;
    struct sockaddr_in addr;

    if ( !sock || !buff || !read || !src )
    {
        return NBIOT_ERR_BADPARAM;
    }

    if ( !(*src) )
    {
        *src = (nbiot_sockaddr_t*)nbiot_malloc( sizeof(nbiot_sockaddr_t) );
        if ( !(*src) )
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
        if ( WSAEWOULDBLOCK != WSAGetLastError() )
        {
            return NBIOT_ERR_SOCKET;
        }
    }
    else
    {
        *read = ret;
        nbiot_memmove( &(*src)->addr, &addr, len );

#ifdef NBIOT_DEBUG
        nbiot_printf( "recvfrom(%s:%d len = %d)\n",
                      inet_ntoa( addr.sin_addr ),
                      ntohs( addr.sin_port ),
                      ret );
        nbiot_buffer_printf( buff, ret );
#endif
    }

    return NBIOT_ERR_OK;
}

bool nbiot_sockaddr_equal( const nbiot_sockaddr_t *addr1,
                           const nbiot_sockaddr_t *addr2 )
{
    if ( !addr1 || !addr2 )
    {
        return false;
    }

    return !nbiot_memcmp( addr1,
                          addr2,
                          sizeof(nbiot_sockaddr_t) );
}

void nbiot_sockaddr_destroy( nbiot_sockaddr_t *s )
{
    if ( s )
    {
        nbiot_free( s );
    }
}
