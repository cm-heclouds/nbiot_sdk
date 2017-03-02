/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>
#include <windows.h>

time_t nbiot_time( void )
{
    return time( NULL );
}

clock_t nbiot_tick( void )
{
    return GetTickCount();
}

void nbiot_sleep( int milliseconds )
{
    Sleep( milliseconds );
}