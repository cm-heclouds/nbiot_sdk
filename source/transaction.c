/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

#ifdef NOTIFY_ACK
int nbiot_transaction_add( nbiot_device_t              *dev,
                           const uint8_t               *buffer,
                           size_t                       buffer_len,
                           nbiot_transaction_callback_t callback,
                           const nbiot_uri_t           *uri )
#else
int nbiot_transaction_add( nbiot_device_t              *dev,
                           const uint8_t               *buffer,
                           size_t                       buffer_len,
                           nbiot_transaction_callback_t callback )
#endif
{
    uint8_t *temp;
    nbiot_transaction_t *transaction;

    transaction = (nbiot_transaction_t*)nbiot_malloc( sizeof(nbiot_transaction_t) );
    if ( !transaction )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    temp = nbiot_malloc( buffer_len );
    if ( !temp )
    {
        nbiot_free( transaction );

        return NBIOT_ERR_NO_MEMORY;
    }

    nbiot_memzero( transaction, sizeof(nbiot_transaction_t) );
    nbiot_memmove( temp, buffer, buffer_len );
    transaction->mid = coap_mid( buffer );
    transaction->buffer = temp;
    transaction->buffer_len = buffer_len;
    transaction->callback = callback;
#ifdef NOTIFY_ACK
    if ( uri )
    {
        *transaction->uri = *uri;
    }
#endif
    dev->transactions = (nbiot_transaction_t*)NBIOT_LIST_ADD( dev->transactions, transaction );

    return NBIOT_ERR_OK;
}

int nbiot_transaction_del( nbiot_device_t *dev,
                           uint16_t        mid,
                           const uint8_t  *buffer,
                           size_t          buffer_len )
{
    nbiot_transaction_t *transaction = NULL;

    dev->transactions = (nbiot_transaction_t*)NBIOT_LIST_DEL( dev->transactions,
                                                              mid,
                                                              &transaction );
    if ( !transaction )
    {
        return NBIOT_ERR_NOT_FOUND;
    }

    if ( buffer &&
         buffer_len &&
         transaction->callback )
    {
#ifdef NOTIFY_ACK
        if ( transaction->uri->flag )
        {
            transaction->callback( dev,
                                   transaction->buffer,
                                   transaction->buffer_len,
                                   transaction->ack,
                                   transaction->uri );
        }
        else
        {
            transaction->callback( dev,
                                   buffer,
                                   buffer_len,
                                   transaction->ack,
                                   transaction->uri );
        }
#else
        transaction->callback( dev,
                               buffer,
                               buffer_len,
                               transaction->ack );
#endif
    }

    nbiot_free( transaction->buffer );
    nbiot_free( transaction );

    return NBIOT_ERR_OK;
}

void nbiot_transaction_step( nbiot_device_t *dev,
                             time_t          now,
                             uint8_t        *buffer,
                             size_t          buffer_len )
{
    nbiot_transaction_t *transaction = dev->transactions;
    while ( transaction )
    {
        nbiot_transaction_t *next = transaction->next;
        if ( transaction->time <= now )
        {
            bool max_counter = false;
            if ( !transaction->ack )
            {
                if ( 0 == transaction->counter )
                {
                    transaction->counter = 1;
                    transaction->time = now + COAP_MAX_RETRANSMIT;
                }
                else if ( COAP_MAX_RETRANSMIT+1 >= transaction->counter )
                {
                    size_t sent = 0;

                    /* retry send */
                    nbiot_udp_send( dev->socket,
                                    transaction->buffer,
                                    transaction->buffer_len,
                                    &sent,
                                    dev->server );
                    transaction->time += COAP_RESPONSE_TIMEOUT << (++transaction->counter);
                }
                else
                {
                    max_counter = true;
                }
            }

            if ( transaction->ack || max_counter )
            {
                nbiot_transaction_del( dev,
                                       transaction->mid,
                                       NULL,
                                       0 );
            }
        }

        transaction = next;
    }
}