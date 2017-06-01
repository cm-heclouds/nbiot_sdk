/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

int nbiot_init_token( uint8_t *token,
                      uint8_t  token_len,
                      uint16_t mid )
{
    int val;
    time_t sec;
    uint8_t arr[8];

    if ( !token || token_len > 8 )
    {
        return -1;
    }

    val = nbiot_rand();
    sec = nbiot_time();
    arr[0] = (uint8_t)mid;
    arr[1] = (uint8_t)(mid >> 8);
    arr[2] = (uint8_t)sec;
    arr[3] = (uint8_t)(sec >> 8);
    arr[4] = (uint8_t)(sec >> 16);
    arr[5] = (uint8_t)(sec >> 24);
    arr[6] = (uint8_t)val;
    arr[7] = (uint8_t)(val >> 8);
    nbiot_memmove( token, arr, token_len );

    return 0;
}

int nbiot_add_string( const char *src,
                      char       *dest,
                      size_t      size )
{
    size_t use = 0;

    while ( *src && use < size )
    {
        ++use;
        *dest++ = *src++;
    }

    if ( *src )
    {
        return 0;
    }

    if ( use < size )
    {
        *dest = '\0';
    }

    return use;
}

int nbiot_add_integer( int64_t src,
                       char   *dest,
                       size_t  size )
{
    size_t use = 0;

    if ( src < 0 && use < size )
    {
        ++use;
        src = -src;
        *dest++ = '-';
    }

    if ( use < size )
    {
        int tmp;
        char *eos = dest;

        do
        {
            ++use;
            *eos++ = src % 10 + '0';
            src = src / 10;
        } while ( src && use < size );

        if ( src )
        {
            return 0;
        }

        if ( use < size )
        {
            *eos = '\0';
        }

        /* reverse */
        --eos;
        while ( dest < eos )
        {
            tmp = *dest;
            *dest = *eos;
            *eos = tmp;

            ++dest;
            --eos;
        }
    }

    return use;
}

int nbiot_send_buffer( nbiot_socket_t         *socket,
                       const nbiot_sockaddr_t *address,
                       const uint8_t          *buffer,
                       size_t                  buffer_len )
{
    int ret;
    size_t sent = 0;

    ret = nbiot_udp_send( socket,
                          buffer,
                          buffer_len,
                          &sent,
                          address );
    if ( ret )
    {
        return ret;
    }

    return sent;
}

int nbiot_recv_buffer( nbiot_socket_t    *socket,
                       nbiot_sockaddr_t **address,
                       uint8_t           *buffer,
                       size_t             buffer_len )
{
    int ret;
    size_t recv = 0;

    ret = nbiot_udp_recv( socket,
                          buffer,
                          buffer_len,
                          &recv,
                          address );
    if ( ret )
    {
        return ret;
    }

    return recv;
}

int nbiot_token( const uint8_t *buffer,
                 uint8_t       *token )
{
    const uint8_t *tmp;
    uint8_t token_len;

    token_len = coap_token( buffer, &tmp );
    if ( token )
    {
        nbiot_memmove( token, tmp, token_len );
    }

    return token_len;
}

int nbiot_payload( const uint8_t *buffer,
                   size_t         buffer_len,
                   uint8_t      **payload,
                   uint16_t      *payload_len )
{
    int ret;
    const uint8_t *source;

    ret = coap_payload( buffer,
                        buffer_len,
                        &source,
                        payload_len );
    if ( ret )
    {
        *payload = (uint8_t*)nbiot_strdup( (char*)source, *payload_len );
    }

    return ret;
}

int nbiot_uri_query( const uint8_t *buffer,
                     size_t         buffer_len,
                     char         **uri_query,
                     uint16_t      *uri_query_len,
                     bool           first )
{
    int ret;
    const char *str;

    ret = coap_uri_query( buffer,
                          buffer_len,
                          &str,
                          uri_query_len,
                          first );
    if ( ret )
    {
        *uri_query = nbiot_strdup( str, *uri_query_len );
    }

    return ret;
}

int nbiot_add_uri_query( coap_t     *coap,
                         const char *key,
                         const char *value )
{
    int ret;
    int use;
    uint8_t *buffer;
    uint16_t offset;

    offset = coap->offset + 5;
    buffer = coap->buffer + offset;
    ret = nbiot_add_string( key,
                            (char*)buffer,
                            coap->size - offset );
    if ( !ret )
    {
        return -1;
    }

    use     = ret;
    buffer += ret;
    ret = nbiot_add_string( value,
                            (char*)buffer,
                            coap->size - offset - use );
    if ( !ret )
    {
        return -1;
    }

    use += ret;
    buffer = coap->buffer + offset;
    if ( coap_add_option(coap,
                         COAP_OPTION_URI_QUERY,
                         buffer,
                         use) )
    {
        return -1;
    }

    return 0;
}