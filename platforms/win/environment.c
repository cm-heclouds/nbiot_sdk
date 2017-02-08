/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>
#include <winsock2.h>

static bool _nbiot_init_state = false;
void nbiot_init_environment( void )
{
    if ( !_nbiot_init_state )
    {
        WSADATA wsa;

        WSAStartup( MAKEWORD(2,2), &wsa );
        _nbiot_init_state = true;
    }
}

void nbiot_clear_environment( void )
{
    if ( _nbiot_init_state )
    {
        WSACleanup();
        _nbiot_init_state = false;
    }
}