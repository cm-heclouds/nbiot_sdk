/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <gtest/gtest.h>
#include <platform.h>
#include <error.h>

TEST( platform, normal )
{
    void *ptr = nbiot_malloc( 1 );
    nbiot_free( ptr );

    time_t curr = nbiot_time();
    EXPECT_GE( time(NULL), curr );
    nbiot_sleep( 1000 );
    EXPECT_GE( nbiot_time(), curr + 1 );

    nbiot_socket_t *server = NULL;
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_create(&server) );
    EXPECT_NE( (nbiot_socket_t*)NULL, server );
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_bind(server,"localhost",5638) );

    nbiot_socket_t *client = NULL;
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_create(&client) );
    EXPECT_NE( (nbiot_socket_t*)NULL, client );
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_bind(client,"localhost",56380) );

    nbiot_sockaddr_t *dest_c = NULL;
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_connect(client,"localhost",5638,&dest_c) );
    EXPECT_NE( (nbiot_sockaddr_t*)NULL, dest_c );

    size_t sent_c = 0;
    const char *str_c = "client udp socket";
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_send(client,str_c,strlen(str_c)+1,&sent_c,dest_c) );
    EXPECT_EQ( sent_c, strlen(str_c)+1 );

    size_t read_s = 0;
    char buf_s[32] = { '\0' };
    nbiot_sockaddr_t *src_s = NULL;
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_recv(server,buf_s,sizeof(buf_s),&read_s,&src_s) );
    EXPECT_NE( (nbiot_sockaddr_t*)NULL, src_s );
    EXPECT_STREQ( str_c, buf_s );
    EXPECT_EQ( read_s, sent_c );

    size_t sent_s = 0;
    const char *str_s = "server udp socket";
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_send(server,str_s,strlen(str_s)+1,&sent_s,src_s) );
    EXPECT_EQ( sent_s, strlen(str_s)+1 );

    size_t read_c = 0;
    char buf_c[32] = { '\0' };
    nbiot_sockaddr_t *src_c = NULL;
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_recv(client,buf_c,sizeof(buf_c),&read_c,&src_c) );
    EXPECT_NE( (nbiot_sockaddr_t*)NULL, src_c );
    EXPECT_STREQ( str_s, buf_c );
    EXPECT_EQ( read_c, sent_s );
    EXPECT_TRUE( nbiot_sockaddr_equal(dest_c,src_c) );

    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_close(client) );
    EXPECT_EQ( NBIOT_ERR_OK, nbiot_udp_close(server) );

    nbiot_sockaddr_destroy( dest_c );
    nbiot_sockaddr_destroy( src_c );
    nbiot_sockaddr_destroy( src_s );
}