/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

time_t nbiot_time( void )
{
    return time( NULL );
}

#ifdef HAVE_DTLS
clock_t nbiot_tick( void )
{
    static clock_t clock_offset = 0;
    struct timeval tv;

    if ( 0 == clock_offset )
    {
        clock_offset = time(NULL);
    }
    gettimeofday( &tv, NULL );

    return (tv.tv_sec-clock_offset)*CLOCK_PER_SECOND + tv.tv_usec*CLOCK_PER_SECOND/1000;
}
#endif

void nbiot_sleep( int milliseconds )
{
    usleep( milliseconds*1000 );
}
