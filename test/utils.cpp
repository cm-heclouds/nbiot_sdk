/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <gtest/gtest.h>
#include <utils.h>
#include <stdlib.h>

TEST( utils, normal )
{
    nbiot_init_environment();
    {
        const char *str = "test utils";
        EXPECT_EQ( strlen(str), nbiot_strlen(str) );

        char dest[32] = { '\0' };
        EXPECT_STREQ( str, nbiot_strncpy(dest,str,-1) );

        nbiot_strncpy( dest, str, 5 );
        EXPECT_EQ( 0, nbiot_strncmp(dest,str,5) );

        char *_new = nbiot_strdup( str );
        EXPECT_NE( (char*)NULL, _new );
        EXPECT_STREQ( _new, str );
        nbiot_free( _new );
        _new = NULL;

        EXPECT_EQ( strrchr(str,'#'), nbiot_strrchr(str,'#') );
        EXPECT_EQ( strrchr(str,' '), nbiot_strrchr(str,' ') );

        EXPECT_EQ( atoi("2345"), nbiot_atoi("2345") );
        EXPECT_EQ( atoi("-2345"), nbiot_atoi("-2345") );

        char _num[2][32];
        sprintf( _num[0], "%d", 2345 );
        EXPECT_STREQ( _num[0], nbiot_itoa(_num[1],2345) );
        sprintf( _num[0], "%d", -2345 );
        EXPECT_STREQ( _num[0], nbiot_itoa(_num[1],-2345) );

        EXPECT_STREQ( str, (char*)nbiot_memmove(dest,str,strlen(str)+1) );
        EXPECT_EQ( 0, nbiot_memcmp(dest,str,strlen(str)+1) );

        memset( _num[0], 0, 32 );
        nbiot_memzero( _num[1], 32 );
        EXPECT_EQ( 0, nbiot_memcmp(_num[0],_num[1],32) );
    }
    nbiot_clear_environment();
}