/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <platform.h>

static bool s_init_env = false;
void nbiot_init_environment( int argc, char *argv[] )
{
    if ( !s_init_env )
    {
        /* todo */
        s_init_env = true;
    }
}

void nbiot_clear_environment( void )
{
    if ( s_init_env )
    {
        /* todo */
        s_init_env = false;
    }
}
