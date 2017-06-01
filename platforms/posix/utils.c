/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

int nbiot_strlen( const char *str )
{
    const char *eos;

    eos = str;
    while ( *eos )
    {
        ++eos;
    }

    return (eos - str);
}

int nbiot_strncpy( char       *dest,
                   const char *src,
                   int         size )
{
    char *eos;

    eos = dest;
    while ( size && *src )
    {
        *eos = *src;
        ++eos;
        ++src;
        --size;
    }
    *eos = '\0';

    return (eos - dest);
}

int nbiot_strncmp( const char *str1,
                   const char *str2,
                   int         size )
{
    if ( str1 && str2 )
    {
        while ( size &&
                *str1 &&
                *str2 &&
                *str1 == *str2 )
        {
            ++str1;
            ++str2;
            --size;
        }

        if ( size )
        {
            return (*str1 - *str2);
        }

        return 0;
    }

    if ( str1 )
    {
        return 1;
    }

    if ( str2 )
    {
        return -1;
    }

    return 0;
}

char* nbiot_strdup( const char *str,
                    int         size )
{
    char *dest = NULL;

    if ( str )
    {
        if ( size < 0 )
        {
            size = nbiot_strlen( str );
        }

        dest = (char*)nbiot_malloc( size + 1 );
        if ( dest )
        {
            dest[size] = '\0';
            nbiot_memmove( dest, str, size );
        }
    }

    return dest;
}

const char* nbiot_strrchr( const char *str,
                           int         size,
                           char        ch )
{
    const char *eos = str;
    const char *dst = NULL;

    while ( size && *eos )
    {
        while ( size &&
                *eos &&
                *eos != ch )
        {
            ++eos;
            --size;
        }

        if ( size && *eos == ch )
        {
            --size;
            dst = eos++;
        }
    }

    return dst;
}

int nbiot_atoi( const char *str,
                int         size )
{
    int ret = 0;
    int pst = 1;

    if ( str )
    {
        while ( size &&
                *str &&
                (' '  == *str ||
                 '\r' == *str ||
                 '\t' == *str ||
                 '\n' == *str ||
                 '-'  == *str ||
                 '+'  == *str) )
        {
            pst = *str - '-';
            ++str;
            --size;
        }

        while ( size &&
                *str &&
                *str >= '0' &&
                *str <= '9' )
        {
            ret *= 10;
            ret += *str - '0';
            ++str;
            --size;
        }
    }

    return (pst ? ret : -ret);
}

int nbiot_itoa( int   val,
                char *str,
                int   size )
{
    if ( str )
    {
        char ch;
        char tmp[11];
        char *dst = tmp;
        char *eos = tmp;

        if ( val < 0 )
        {
            dst++;
            val = -val;
            *eos++ = '-';
        }

        do
        {
            *eos++ = val % 10 + '0';
            val = val / 10;
        } while( val );

        /* reverse */
        *eos-- = '\0';
        while ( dst < eos )
        {
            ch = *dst;
            *dst = *eos;
            *eos = ch;

            ++dst;
            --eos;
        }

        /* copy */
        return nbiot_strncpy( str, tmp, size );
    }

    return 0;
}

void *nbiot_memmove( void       *dst,
                     const void *src,
                     size_t      size )
{
    if ( dst && src && size )
    {
        char *_dst;
        const char *_src;

        _dst = (char*)dst;
        _src = (const char*)src;
        while ( size )
        {
            *_dst = *_src;
            ++_dst;
            ++_src;
            --size;
        }
    }

    return dst;
}

int nbiot_memcmp( const void *mem1,
                  const void *mem2,
                  size_t      size )
{
    if ( mem1 && mem2 )
    {
        const char *str1;
        const char *str2;

        str1 = (const char*)mem1;
        str2 = (const char*)mem2;
        while ( size &&
                *str1 == *str2 )
        {
            ++str1;
            ++str2;
            --size;
        }

        if ( size )
        {
            return (*str1 - *str2);
        }

        return 0;
    }

    if ( mem1 )
    {
        return 1;
    }

    if ( mem2 )
    {
        return -1;
    }

    return 0;

}

void nbiot_memzero( void  *mem,
                    size_t size )
{
    if ( mem && size )
    {
        char *dst;

        dst = (char*)mem;
        while ( size )
        {
            *dst = '\0';
            ++dst;
            --size;
        }
    }
}

int nbiot_rand( void )
{
    static bool s_rand = false;

    if ( !s_rand )
    {
        s_rand = true;
        srand( (unsigned int)time(NULL) );
    }

    return rand();
}

#ifdef NBIOT_DEBUG
int nbiot_isspace( int ch )
{
    return isspace( ch );
}

int nbiot_isprint( int ch )
{
    return isprint( ch );
}

void nbiot_printf( const char *format, ... )
{
    va_list args;

    va_start( args, format );
    vfprintf( stderr, format, args );
    va_end( args );
}

void nbiot_buffer_printf( const void *buf,
                          size_t      len )
{
    if ( len )
    {
        size_t i = 0;
        const uint8_t *tmp = (const uint8_t*)buf;

        while ( i < len )
        {
            size_t j;
            uint8_t arr[16];

            nbiot_memmove( arr, tmp + i, 16 );
            for ( j = 0;
                  j < 16 && i + j < len;
                  ++j )
            {
                nbiot_printf( "%02X ", arr[j] );
                if ( j % 4 == 3 )
                {
                    nbiot_printf( " " );
                }
            }

            while ( j < 16 )
            {
                nbiot_printf( "   " );
                if ( j % 4 == 3 )
                {
                    nbiot_printf( " " );
                }
                ++j;
            }

            nbiot_printf( " " );
            for ( j = 0;
                  j < 16 && i + j < len;
                  ++j )
            {
                if ( nbiot_isprint(arr[j]) )
                {
                    nbiot_printf( "%c", arr[j] );
                }
                else
                {
                    nbiot_printf( "." );
                }
            }
            nbiot_printf( "\n" );
            i += 16;
        }
    }
    else
    {
        nbiot_printf( "\n" );
    }
}
#endif
