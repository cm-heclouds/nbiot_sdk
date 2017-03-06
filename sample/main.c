/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <nbiot.h>
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void usage( const char *name )
{
    nbiot_printf( "Usage: %s [OPTION]\r\n", name );
    nbiot_printf( "Launch a client.\r\n" );
    nbiot_printf( "Options:\r\n" );
    nbiot_printf( "-i URI\t\tSet the coap uri of the server to connect to. For example: coap://localhost:5683\r\n" );
    nbiot_printf( "-p PORT\t\tSet the port of the client to bind to. Default: 56830\r\n" );
    nbiot_printf( "-t TIME\t\tSet the lifetime of the client. Default: 300\r\n" );
    nbiot_printf( "-n NAME\t\tSet the endpoint name of the client.\r\n" );
    nbiot_printf( "-a AUTHCODE\tSet the authentication code of the client.\r\n" );
    nbiot_printf( "\r\n" );
}

void write_callback( nbiot_resource_t *res )
{
    nbiot_printf( "write /%d/%d/%d\r\n",
                  res->objid,
                  res->instid,
                  res->resid );
}

void execute_callback( nbiot_resource_t *res,
                       const uint8_t    *buffer,
                       int               length )
{
    int i;

    nbiot_printf( "execute /%d/%d/%d\r\n",
                  res->objid,
                  res->instid,
                  res->resid );

    i = 0;
    while ( i < length )
    {
        uint8_t array[16];
        int j;

        nbiot_memmove( array, buffer + i, 16 );
        for ( j = 0; j < 16 && i + j < length; j++ )
        {
            nbiot_printf( "%02X ", array[j] );
            if ( j % 4 == 3 ) nbiot_printf( " " );
        }
        if ( length > 16 )
        {
            while ( j < 16 )
            {
                nbiot_printf( "   " );
                if ( j % 4 == 3 ) nbiot_printf( " " );
                j++;
            }
        }
        nbiot_printf( " " );
        for ( j = 0; j < 16 && i + j < length; j++ )
        {
            if ( isprint( array[j] ) )
                nbiot_printf( "%c", array[j] );
            else
                nbiot_printf( "." );
        }
        nbiot_printf( "\n" );
        i += 16;
    }
}

int main( int argc, char *argv[] )
{
    int life_time = 300;
    uint16_t port = 56830;
    const char *uri = NULL;
    const char *auth_code = NULL;
    const char *endpoint_name = NULL;

    int opt = 1;
    while ( opt < argc )
    {
        if ( NULL == argv[opt] ||
             '-'  != argv[opt][0] ||
             '\0' != argv[opt][2] )
        {
            usage( argv[0] );
            return 0;
        }

        switch ( argv[opt][1] )
        {
            case 'i':
            {
                ++opt;
                if ( opt >= argc )
                {
                    usage( argv[0] );
                    return 0;
                }

                uri = argv[opt];
            }
            break;

            case 'p':
            {
                ++opt;
                if ( opt >= argc )
                {
                    usage( argv[0] );
                    return 0;
                }

                port = nbiot_atoi( argv[opt] );
            }
            break;

            case 't':
            {
                ++opt;
                if ( opt >= argc )
                {
                    usage( argv[0] );
                    return 0;
                }

                life_time = nbiot_atoi( argv[opt] );
            }
            break;

            case 'n':
            {
                ++opt;
                if ( opt >= argc )
                {
                    usage( argv[0] );
                    return 0;
                }

                endpoint_name = argv[opt];
            }
            break;

            case 'a':
            {
                ++opt;
                if ( opt >= argc )
                {
                    usage( argv[0] );
                    return 0;
                }

                auth_code = argv[opt];
            }
            break;

            default:
            {
                usage( argv[0] );
                return 0;
            }
            break;
        }

        ++opt;
    }

    if ( NULL == uri ||
         NULL == auth_code ||
         NULL == endpoint_name )
    {
        usage( argv[0] );
        return 0;
    }

    nbiot_init_environment();
    do
    {
        int i;
        int ret;
        char tmp[16];
        nbiot_device_t *dev = NULL;

        nbiot_resource_t dis;  /* ipso digital input - digital input state */
        nbiot_resource_t dic;  /* ipso digital input - digital input counter */
        nbiot_resource_t dicr; /* ipso digital input - digital input counter reset */
        nbiot_resource_t at;   /* ipso digital input - application type */
        nbiot_resource_t aicv; /* ipso analog input - analog input current value */
        nbiot_resource_t *res[5];

        res[0] = &dis;
        res[1] = &dic;
        res[2] = &dicr;
        res[3] = &at;
        res[4] = &aicv;

        /* ipso digital input - digital input state */
        dis.objid         = 3200;
        dis.instid        = 0;
        dis.resid         = 5500;
        dis.type          = NBIOT_VALUE_BOOLEAN;
        dis.value.as_bool = rand()%2 > 0;
        dis.flag          = NBIOT_RESOURCE_READABLE;
        dis.write         = NULL;
        dis.execute       = NULL;

        /* ipso digital input - digital input counter */
        dic.objid        = 3200;
        dic.instid       = 0;
        dic.resid        = 5501;
        dic.type         = NBIOT_VALUE_INTEGER;
        dic.value.as_int = rand();
        dic.flag         = NBIOT_RESOURCE_READABLE;
        dic.write        = NULL;
        dic.execute      = NULL;

        /* ipso digital input - digital input counter reset */
        dicr.objid            = 3200;
        dicr.instid           = 0;
        dicr.resid            = 5505;
        dicr.type             = NBIOT_VALUE_BINARY;
        dicr.value.as_bin.bin = (uint8_t*)nbiot_strdup( nbiot_itoa(tmp,rand()) );
        dicr.value.as_bin.len = nbiot_strlen( (char*)dicr.value.as_bin.bin ) + 1;
        dicr.flag             = NBIOT_RESOURCE_READABLE;
        dicr.write            = NULL;
        dicr.execute          = NULL;

        /* ipso digital input - application type */
        at.objid            = 3200;
        at.instid           = 0;
        at.resid            = 5750;
        at.type             = NBIOT_VALUE_STRING;
        at.value.as_str.str = nbiot_strdup( nbiot_itoa(tmp,rand()) );
        at.value.as_str.len = nbiot_strlen( (char*)dicr.value.as_str.str ) + 1;
        at.flag             = NBIOT_RESOURCE_READABLE | NBIOT_RESOURCE_WRITABLE;
        at.write            = write_callback;
        at.execute          = NULL;

        /* ipso analog input - analog input current value */
        aicv.objid          = 3202;
        aicv.instid         = 0;
        aicv.resid          = 5600;
        aicv.type           = NBIOT_VALUE_FLOAT;
        aicv.value.as_float = rand() * 0.001f;
        aicv.flag           = NBIOT_RESOURCE_READABLE;
        aicv.write          = NULL;
        aicv.execute        = NULL;

        ret = nbiot_device_create( &dev, port );
        if ( ret )
        {
            nbiot_printf( "create device instance failed.\r\n" );
            break;
        }

        ret = nbiot_device_connect( dev,
                                    uri,
                                    life_time );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            nbiot_printf( "connect server failed.\r\n" );
            break;
        }

        ret = nbiot_device_configure( dev,
                                      endpoint_name,
                                      auth_code,
                                      res,
                                      sizeof(res) / sizeof(res[0]) );
        if ( ret )
        {
            nbiot_device_close( dev );
            nbiot_device_destroy( dev );
            nbiot_printf( "configure client failed.\r\n" );
            break;
        }

        i = 0;
        while ( i < life_time/10 )
        {
            ret = nbiot_device_step( dev, 1 );
            if ( ret )
            {
                nbiot_printf( "device step error, code = %d.\r\n", ret );
                break;
            }
            else
            {
                ++i;
                nbiot_sleep( 1000 );
            }
        }

        if ( !ret )
        {
            while ( i < life_time )
            {
                /* ipso digital input - digital input state */
                dis.value.as_bool = rand()%2 > 0;
                nbiot_device_notify( dev,
                                     dis.objid,
                                     dis.instid,
                                     dis.resid );

                /* ipso digital input - digital input counter */
                dic.value.as_int = rand();
                nbiot_device_notify( dev,
                                     dic.objid,
                                     dic.instid,
                                     dic.resid );

                /* ipso digital input - digital input counter reset */
                nbiot_free( dicr.value.as_bin.bin );
                dicr.value.as_bin.bin = (uint8_t*)nbiot_strdup( nbiot_itoa( tmp, rand() ) );
                dicr.value.as_bin.len = nbiot_strlen( (char*)dicr.value.as_bin.bin ) + 1;
                nbiot_device_notify( dev,
                                     dicr.objid,
                                     dicr.instid,
                                     dicr.resid );

                /* ipso digital input - application type */
                nbiot_free( at.value.as_str.str );
                at.value.as_str.str = nbiot_strdup( nbiot_itoa( tmp, rand() ) );
                at.value.as_str.len = nbiot_strlen( (char*)dicr.value.as_str.str ) + 1;
                nbiot_device_notify( dev,
                                     at.objid,
                                     at.instid,
                                     at.resid );

                /* ipso analog input - analog input current value */
                aicv.value.as_float = rand() * 0.001f;
                nbiot_device_notify( dev,
                                     aicv.objid,
                                     aicv.instid,
                                     aicv.resid );

                ret = nbiot_device_step( dev, 1 );
                if ( ret )
                {
                    nbiot_printf( "device step error, code = %d.\r\n", ret );
                    break;
                }
                else
                {
                    ++i;
                    nbiot_sleep( 1000 );
                }
            }
        }

        nbiot_device_close( dev );
        nbiot_device_destroy( dev );
        nbiot_free( dicr.value.as_bin.bin );
        nbiot_free( at.value.as_str.str );
    } while(0);
    nbiot_clear_environment();

    getchar();
    nbiot_printf( "press enter key to exit..." );

    return 0;
}