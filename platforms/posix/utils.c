/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <utils.h>
#include <stdio.h>
#include <stdarg.h>

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

char* nbiot_strncpy( char       *dest,
                     const char *src,
                     int         size )
{
    char *tmp;

    tmp = dest;
    if ( size < 0 )
    {
        while ( *src )
        {
            *tmp = *src;
            ++tmp;
            ++src;
        }
    }
    else
    {
        while ( *src && size )
        {
            *tmp = *src;
            ++tmp;
            ++src;
            --size;
        }
    }
    *tmp = '\0';

    return dest;
}

int nbiot_strncmp( const char *str1,
                   const char *str2,
                   int         size )
{
    if ( str1 && str2 )
    {
        if ( size < 0 )
        {
            while ( *str1 &&
                    *str2 &&
                    *str1 == *str2 )
            {
                ++str1;
                ++str2;
            }

            return (*str1 - *str2);
        }
        else
        {
            while ( *str1 &&
                    *str2 &&
                    *str1 == *str2 &&
                    size > 0 )
            {
                ++str1;
                ++str2;
                --size;
            }

            if ( size > 0 )
            {
                return (*str1 - *str2);
            }
            else
            {
                return 0;
            }
        }
    }
    else if ( str1 )
    {
        return 1;
    }
    else if ( str2 )
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

char* nbiot_strdup( const char *str )
{
    int len;
    char *dst;

    if ( NULL == str )
    {
        return NULL;
    }

    len = nbiot_strlen(str) + 1;
    dst = (char*)nbiot_malloc( len );
    if ( NULL != dst )
    {
        nbiot_strncpy( dst, str, len );
    }

    return dst;
}

char* nbiot_strrchr( const char *str,
                     char        ch )
{
    const char *eos;

    eos = str;
    while ( *eos )
    {
        ++eos;
    }

    while ( eos != str && *eos != ch )
    {
        --eos;
    }

    if ( *eos == ch )
    {
        return (char*)eos;
    }

    return NULL;
}

int nbiot_atoi( const char *str )
{
    int ret;
    int neg;

    ret = 0;
    if ( str )
    {
        while ( ' '  == *str ||
                '\r' == *str ||
                '\t' == *str ||
                '\n' == *str )
        {
            ++str;
        }

        if ( '-' == *str )
        {
            neg = 1;
        }
        else
        {
            neg = 0;
        }

        while ( '-'  == *str ||
                '+'  == *str ||
                ' '  == *str ||
                '\r' == *str ||
                '\t' == *str ||
                '\n' == *str )
        {
            ++str;
        }

        while ( *str )
        {
            if ( *str < '0' || *str > '9' )
            {
                break;
            }
            else
            {
                ret *= 10;
                ret += (*str - '0');
            }

            ++ str;
        }
    }

    return (neg ? -ret : ret);
}

char* nbiot_itoa( char *str,
                  int   val )
{
    if ( str )
    {
        char *dst = str;
        char *eos = str;

        if ( val < 0 )
        {
            val = -val;
            *dst = '-';
            ++dst;
            ++eos;
        }

        do
        {
            int rem;

            rem = val % 10;
            val = val / 10;

            *eos = '0' + rem;
            ++eos;
        } while ( val );
        *eos = '\0';

        /* reverse */
        --eos;
        while ( dst < eos )
        {
            char ch;

            ch = *dst;
            *dst = *eos;
            *eos = ch;

            ++dst;
            --eos;
        }
    }

    return str;
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
        while ( *str1 == *str2 && size )
        {
            ++str1;
            ++str2;
            --size;
        }

        if ( size )
        {
            return (*str1 - *str2);
        }
        else
        {
            return 0;
        }
    }
    else if ( mem1 )
    {
        return 1;
    }
    else if ( mem2 )
    {
        return -1;
    }
    else
    {
        return 0;
    }
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

void nbiot_printf( const char *format, ... )
{
    va_list args;

    va_start( args, format );
    vfprintf( stderr, format, args );
    va_end( args );
}