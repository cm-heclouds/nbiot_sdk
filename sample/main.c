/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <nbiot.h>
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef NBIOT_DEBUG
#define nbiot_printf(...)        printf(__VA_ARGS__)
#define nbiot_buffer_printf(x,y) printf("size = %d",(y))
#endif

void usage( const char *name )
{
    nbiot_printf( "Usage: %s [OPTION]\r\n", name );
    nbiot_printf( "Launch a client.\r\n" );
    nbiot_printf( "Options:\r\n" );
    nbiot_printf( "-i URI\t\tSet the coap uri of the server to connect to. For example: coap://localhost:5683\r\n" );
    nbiot_printf( "-p PORT\t\tSet the port of the client to bind to. Default: 56830\r\n" );
    nbiot_printf( "-t TIME\t\tSet the lifetime of the client. Default: 300\r\n" );
    nbiot_printf( "-n NAME\t\tSet the endpoint name[imei;imsi] of the client.\r\n" );
    nbiot_printf( "\r\n" );
}

void write_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{
    nbiot_printf( "write /%d/%d/%d\r\n",
                  objid,
                  instid,
                  resid );
}

void execute_callback( uint16_t       objid,
                       uint16_t       instid,
                       uint16_t       resid,
                       nbiot_value_t *data,
                       const void    *buff,
                       size_t         size )
{
    nbiot_printf( "execute /%d/%d/%d\r\n",
                  objid,
                  instid,
                  resid );
    nbiot_buffer_printf( buff, (int)size );
}

int main( int argc, char *argv[] )
{
    int life_time = 300;
    uint16_t port = 56830;
    const char *uri = NULL;
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

                port = nbiot_atoi( argv[opt], -1 );
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

                life_time = nbiot_atoi( argv[opt], -1 );
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

            default:
            {
                usage( argv[0] );
                return 0;
            }
            break;
        }

        ++opt;
    }

    if ( NULL == uri || NULL == endpoint_name )
    {
        usage( argv[0] );
        return 0;
    }

    nbiot_init_environment( argc, argv );
    do
    {
        int ret;
        int i = 0;
        char tmp[16];
        nbiot_value_t dis;  /* ipso digital input - digital input state */
        nbiot_value_t dic;  /* ipso digital input - digital input counter */
        nbiot_value_t aicv; /* ipso analog input - analog input current value */
        nbiot_value_t dicr; /* ipso digital input - digital input counter reset */
        nbiot_value_t at;   /* ipso digital input - application type */
        nbiot_device_t *dev = NULL;

        ret = nbiot_device_create( &dev,
                                   endpoint_name,
                                   life_time,
                                   port );
        if ( ret )
        {
            nbiot_printf( "device create failed, code = %d.\r\n", ret );
            break;
        }

        dis.type = NBIOT_BOOLEAN;
        dis.flag = NBIOT_READABLE;
        dis.value.as_bool = nbiot_rand() % 2 > 0;
        ret = nbiot_resource_add( dev,
                                  3200,
                                  0,
                                  5500,
                                  &dis );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            nbiot_printf( "device add resource(/3200/0/5500) failed, code = %d.\r\n", ret );
            break;
        }

        dic.type = NBIOT_INTEGER;
        dic.flag = NBIOT_READABLE;
        dic.value.as_int = nbiot_rand();
        ret = nbiot_resource_add( dev,
                                  3200,
                                  0,
                                  5501,
                                  &dic );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            nbiot_printf( "device add resource(/3200/0/5501) failed, code = %d.\r\n", ret );
            break;
        }

        aicv.type = NBIOT_FLOAT;
        aicv.flag = NBIOT_READABLE;
        aicv.value.as_float = nbiot_rand() * 0.001f;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  0,
                                  5600,
                                  &aicv );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            nbiot_printf( "device add resource(/3202/0/5600) failed, code = %d.\r\n", ret );
            break;
        }

        dicr.type = NBIOT_STRING;
        dicr.flag = NBIOT_READABLE;
        dicr.value.as_buf.len = nbiot_itoa( nbiot_rand(), tmp, 16 );
        dicr.value.as_buf.val = nbiot_strdup( tmp, dicr.value.as_buf.len );
        ret = nbiot_resource_add( dev,
                                  3200,
                                  0,
                                  5505,
                                  &dicr );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            nbiot_printf( "device add resource(/3200/0/5505) failed, code = %d.\r\n", ret );
            break;
        }

        at.type = NBIOT_BINARY;
        at.flag = NBIOT_READABLE;
        at.value.as_buf.len = nbiot_itoa( nbiot_rand(), tmp, 16 );
        at.value.as_buf.val = nbiot_strdup( tmp, dicr.value.as_buf.len );
        ret = nbiot_resource_add( dev,
                                  3200,
                                  0,
                                  5750,
                                  &at );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            nbiot_printf( "device add resource(/3200/0/5750) failed, code = %d.\r\n", ret );
            break;
        }

        ret = nbiot_device_connect( dev, uri, 10 );
        if ( ret )
        {
            nbiot_device_close( dev, 10 );
            nbiot_device_destroy( dev );
            nbiot_printf( "device connect server failed, code = %d.\r\n", ret );
            break;
        }

        while ( i < life_time )
        {
            i++;

            dis.flag |= NBIOT_UPDATED;
            dis.value.as_bool = nbiot_rand() % 2 > 0;

            dic.flag |= NBIOT_UPDATED;
            dic.value.as_int = nbiot_rand();

            aicv.flag |= NBIOT_UPDATED;
            aicv.value.as_float = nbiot_rand() * 0.001;

            dicr.flag |= NBIOT_UPDATED;
            nbiot_free( dicr.value.as_buf.val );
            dicr.value.as_buf.len = nbiot_itoa( nbiot_rand(), tmp, 16 );
            dicr.value.as_buf.val = nbiot_strdup( tmp, dicr.value.as_buf.len );

            at.flag |= NBIOT_UPDATED;
            nbiot_free( at.value.as_buf.val );
            at.value.as_buf.len = nbiot_itoa( nbiot_rand(), tmp, 16 );
            at.value.as_buf.val = nbiot_strdup( tmp, dicr.value.as_buf.len );

            ret = nbiot_device_step( dev, 1 );
            if ( ret )
            {
                nbiot_printf( "device step error, code = %d.\r\n", ret );
                break;
            }
        }

        nbiot_device_close( dev, 10 );
        nbiot_device_destroy( dev );
    } while(0);
    nbiot_clear_environment();

    nbiot_printf( "press enter key to exit..." );
    getchar();

    return 0;
}