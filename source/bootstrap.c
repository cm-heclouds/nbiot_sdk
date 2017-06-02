/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifdef NBIOT_BOOTSTRAP
#include "internal.h"

int nbiot_bootstrap_start( nbiot_device_t *dev,
                           uint16_t        mid,
                           const uint8_t  *token,
                           uint8_t         token_len,
                           uint8_t        *buffer,
                           size_t          buffer_len )
{
    int len;
    coap_t coap[1];

    /* header */
    coap->buffer = buffer;
    coap->size = buffer_len;
    if ( coap_init_header(coap,
                          COAP_TYPE_CON,
                          COAP_POST,
                          mid,
                          token,
                          token_len) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* uri path */
    if ( coap_add_option(coap,
                         COAP_OPTION_URI_PATH,
                         (const uint8_t*)"bs",
                         2) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    /* uri query - endpoint name */
    if ( nbiot_add_uri_query(coap,
                             "ep=",
                             dev->endpoint_name) )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    len = nbiot_send_buffer( dev->socket,
                             dev->server,
                             coap->buffer,
                             coap->offset );
    if ( len != coap->offset )
    {
        return NBIOT_ERR_SOCKET;
    }

    dev->state = STATE_BS_INITIATED;
    return NBIOT_ERR_OK;
}
#endif