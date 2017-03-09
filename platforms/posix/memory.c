/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>
#include <stdlib.h>

#ifdef NBIOT_DEBUG
#include <stdio.h>

static size_t _last = 100;
static size_t _total = 0;
#endif

void *nbiot_malloc( size_t size )
{
#ifdef NBIOT_DEBUG
    size_t *ptr;

    ptr = (size_t*)malloc( sizeof(size_t) + size );
    *ptr = size;
    _total += size;
    if ( _total > _last )
    {
        printf( "nbiot_malloc() %dbytes memories.\n", (int)_total );
        _last += 100;
    }

    return (ptr + 1);
#else
    return malloc( size );
#endif
}

void nbiot_free( void *ptr )
{
#ifdef NBIOT_DEBUG
    if ( NULL != ptr )
    {
        size_t *tmp;

        tmp = (size_t*)ptr;
        --tmp;
        _total -= *tmp;
        free( tmp );
    }
#else
    free( ptr );
#endif
}