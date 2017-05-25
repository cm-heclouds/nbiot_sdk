/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <gtest/gtest.h>
#include <platform.h>

int main( int argc, char *argv[] )
{
    int rc;

    nbiot_init_environment( argc, argv );
    {
        testing::InitGoogleTest( &argc, argv );
        rc = RUN_ALL_TESTS();
    }
    nbiot_clear_environment();

    printf( "press any key to exit..." );
    getchar();

    return rc;
}