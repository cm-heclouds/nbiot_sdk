/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

#ifdef NBIOT_BOOTSTRAP
#ifdef NOTIFY_ACK
static void bootstrap_reply( nbiot_device_t    *dev,
                             const uint8_t     *buffer,
                             size_t             buffer_len,
                             bool               ack,
                             const nbiot_uri_t *uri )
#else
static void bootstrap_reply( nbiot_device_t *dev,
                             const uint8_t  *buffer,
                             size_t          buffer_len,
                             bool            ack )
#endif
{
    if ( dev->state == STATE_BS_INITIATED )
    {
        if ( COAP_CHANGED_204 == coap_code(buffer) )
        {
            dev->state = STATE_BS_PENDING;
        }
        else
        {
            dev->state = STATE_BS_FAILED;
        }
    }
}

int nbiot_bootstrap_start( nbiot_device_t *dev,
                           uint8_t        *buffer,
                           size_t          buffer_len )
{
    if ( dev->state == STATE_BS_FAILED ||
         dev->state == STATE_DEREGISTERED )
    {
        int ret;
        coap_t coap[1];
        uint16_t mid = dev->next_mid++;

        /* header */
        coap->buffer = buffer;
        coap->size = buffer_len;
        if ( coap_init_header(coap,
                              COAP_TYPE_CON,
                              COAP_POST,
                              mid,
                              NULL,
                              4) )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        /* token */
        if ( nbiot_init_token(buffer+COAP_HEADER_SIZE,4,mid) )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        /* uri path */
        if ( coap_add_option(coap,
                             COAP_OPTION_URI_PATH,
                             (const uint8_t*)"bs",
                             2) )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        /* uri query - endpoint name */
        if ( nbiot_add_uri_query( coap,
                                  "ep=",
                                  dev->endpoint_name ) )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        ret = nbiot_send_buffer( dev->socket,
                                 dev->server,
                                 coap->buffer,
                                 coap->offset );
        if ( ret <= 0 )
        {
            return COAP_INTERNAL_SERVER_ERROR_500;
        }

        dev->state = STATE_BS_INITIATED;
#ifdef NOTIFY_ACK
        nbiot_transaction_add( dev,
                               coap->buffer,
                               coap->offset,
                               bootstrap_reply,
                               NULL );
#else
        nbiot_transaction_add( dev,
                               coap->buffer,
                               coap->offset,
                               bootstrap_reply );
#endif
    }

    return COAP_NO_ERROR;
}
#endif /* NBIOT_BOOTSTRAP */