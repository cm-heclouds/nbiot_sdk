/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#include "debug.h"

#ifdef DTLS_WITH_LOGS
void dsrv_hexdump_log( const char          *name,
                       const unsigned char *buf,
                       size_t               length,
                       int                  extend )
{
    int n = 0;

    if ( extend )
    {
        nbiot_printf( "%s: (%zu bytes):\n", name, length );

        while ( length-- )
        {
            if ( n % 16 == 0 )
                nbiot_printf( "%08X ", n );

            nbiot_printf( "%02X ", *buf++ );

            n++;
            if ( n % 8 == 0 )
            {
                if ( n % 16 == 0 )
                    nbiot_printf( "\n" );
                else
                    nbiot_printf( " " );
            }
        }
    }
    else
    {
        nbiot_printf( "%s: (%zu bytes): ", name, length );
        while ( length-- )
            nbiot_printf( "%02X", *buf++ );
    }
    nbiot_printf( "\n" );
}
#endif