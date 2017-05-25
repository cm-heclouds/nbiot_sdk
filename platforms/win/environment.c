/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>
#include <winsock2.h>

static bool s_init_env = false;
void nbiot_init_environment( int argc, char *argv[] )
{
    if ( !s_init_env )
    {
        WSADATA wsa;

        s_init_env = true;
        WSAStartup( MAKEWORD(2,2), &wsa );
    }
}

void nbiot_clear_environment( void )
{
    if ( s_init_env )
    {
        WSACleanup();
        s_init_env = false;
    }
}
