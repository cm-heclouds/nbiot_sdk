/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <gtest/gtest.h>
#include <utils.h>
#include <stdlib.h>
#include <ctype.h>

TEST( utils, normal )
{
     const char *str = "test utils";
     EXPECT_EQ( strlen(str), nbiot_strlen(str) );

     char dest[32] = { '\0' };
     nbiot_strncpy( dest, str, -1 );
     EXPECT_STREQ( str, dest );

     nbiot_strncpy( dest, str, 5 );
     EXPECT_EQ( 0, nbiot_strncmp(dest,str,5) );

     char *_new = nbiot_strdup( str, -1 );
     EXPECT_NE( (char*)NULL, _new );
     EXPECT_STREQ( _new, str );
     nbiot_free( _new );
     _new = NULL;

     EXPECT_EQ( strrchr(str,'#'), nbiot_strrchr(str,-1,'#') );
     EXPECT_EQ( strrchr(str,' '), nbiot_strrchr(str,-1,' ') );

     EXPECT_EQ( atoi("2345"), nbiot_atoi("2345",-1) );
     EXPECT_EQ( atoi("-2345"), nbiot_atoi("-2345",-1) );

     char _num[2][32];
     sprintf( _num[0], "%d", 2345 );
     nbiot_itoa( 2345, _num[1], 32 );
     EXPECT_STREQ( _num[0], _num[1] );
     sprintf( _num[0], "%d", -2345 );
     nbiot_itoa( -2345, _num[1], 32 );
     EXPECT_STREQ( _num[0], _num[1] );

     EXPECT_STREQ( str, (char*)nbiot_memmove(dest,str,strlen(str)+1) );
     EXPECT_EQ( 0, nbiot_memcmp(dest,str,strlen(str)+1) );

     memset( _num[0], 0, 32 );
     nbiot_memzero( _num[1], 32 );
     EXPECT_EQ( 0, nbiot_memcmp(_num[0],_num[1],32) );

#ifdef NBIOT_DEBUG
    EXPECT_EQ( isspace('a'), nbiot_isspace('a') );
    EXPECT_EQ( isspace(' '), nbiot_isspace(' ') );
    EXPECT_EQ( isspace('\r'), nbiot_isspace('\r') );
    EXPECT_EQ( isspace('\t'), nbiot_isspace('\t') );
    EXPECT_EQ( isspace('\n'), nbiot_isspace('\n') );
    EXPECT_EQ( isspace(134), nbiot_isspace(134) );

    EXPECT_EQ( isprint('a'), nbiot_isprint('a') );
    EXPECT_EQ( isprint(' '), nbiot_isprint(' ') );
    EXPECT_EQ( isprint('\r'), nbiot_isprint('\r') );
    EXPECT_EQ( isprint('\t'), nbiot_isprint('\t') );
    EXPECT_EQ( isprint('\n'), nbiot_isprint('\n') );
    EXPECT_EQ( isprint(134), nbiot_isprint(134) );
#endif
}