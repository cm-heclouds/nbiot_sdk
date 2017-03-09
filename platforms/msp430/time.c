/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>
#include <time.h>

time_t nbiot_time( void )
{
    return time( NULL );
}

#ifdef HAVE_DTLS
clock_t nbiot_tick( void )
{
    return clock();
}
#endif

void nbiot_sleep( int milliseconds )
{
    /* todo */
}
