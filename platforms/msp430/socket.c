/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <error.h>
#include <platform.h>
#include <utils.h>
#include <ctype.h>
#include <errno.h>

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
	/* unique identification */
};

struct nbiot_sockaddr_t
{
	/*  unique identification */
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

    /* todo */
    return NBIOT_ERR_OK;
}

int nbiot_udp_close( nbiot_socket_t *sock )
{
    if ( NULL == sock )
    {
        return NBIOT_ERR_BADPARAM;
    }

    /* todo */

    nbiot_free( sock );
    return NBIOT_ERR_OK;
}

int nbiot_udp_bind( nbiot_socket_t *sock,
                    const char     *addr,
                    uint16_t        port )
{
	/* todo */

    return NBIOT_ERR_OK;
}

int nbiot_udp_connect( nbiot_socket_t    *sock,
                       const char        *addr,
                       uint16_t           port,
                       nbiot_sockaddr_t **dest )
{
	/* todo */

    return NBIOT_ERR_OK;
}

int nbiot_udp_send( nbiot_socket_t         *sock,
                    const void             *buff,
                    size_t                  size,
                    size_t                 *sent,
                    const nbiot_sockaddr_t *dest )
{
	int ret = 0;
	/* todo */

#ifdef NBIOT_DEBUG
    nbiot_printf( "sendto(len = %d)\n", ret );
    output_buffer( (uint8_t*)buff, ret );
#endif

    return NBIOT_ERR_OK;
}

int nbiot_udp_recv( nbiot_socket_t    *sock,
                    void              *buff,
                    size_t             size,
                    size_t            *read,
                    nbiot_sockaddr_t **src )
{
	int ret = 0;
	/* todo */

#ifdef NBIOT_DEBUG
    nbiot_printf( "recvfrom(len = %d)\n", ret );
    output_buffer( (uint8_t*)buff, ret );
#endif

    return NBIOT_ERR_OK;
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
