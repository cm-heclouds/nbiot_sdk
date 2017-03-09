/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

static void handle_reset( lwm2m_context_t * contextP,
                          void * fromSessionH,
                          coap_packet_t * message )
{
    LOG( "Entering" );
    observe_cancel( contextP, message->mid, fromSessionH );
    contextP->state = STATE_RESET;
}

static coap_status_t handle_request( lwm2m_context_t * contextP,
                                     void * fromSessionH,
                                     coap_packet_t * message,
                                     coap_packet_t * response )
{
    lwm2m_uri_t * uriP;
    coap_status_t result = COAP_IGNORE;

    LOG( "Entering" );

    uriP = uri_decode( NULL, message->uri_path );
    if ( uriP == NULL ) return COAP_400_BAD_REQUEST;

    switch ( uriP->flag & LWM2M_URI_MASK_TYPE )
    {
        case LWM2M_URI_FLAG_DM:
        {
            lwm2m_server_t * serverP;

            serverP = utils_findServer( contextP, fromSessionH );
            if ( serverP != NULL )
            {
                result = dm_handleRequest( contextP, uriP, serverP, message, response );
            }
#ifdef LWM2M_BOOTSTRAP
            else
            {
                serverP = utils_findBootstrapServer( contextP, fromSessionH );
                if ( serverP != NULL )
                {
                    result = bootstrap_handleCommand( contextP, uriP, serverP, message, response );
                }
            }
#endif
        }
        break;

#ifdef LWM2M_BOOTSTRAP
        case LWM2M_URI_FLAG_DELETE_ALL:
        if ( COAP_DELETE != message->code )
        {
            result = COAP_400_BAD_REQUEST;
        }
        else
        {
            result = bootstrap_handleDeleteAll( contextP, fromSessionH );
        }
        break;

        case LWM2M_URI_FLAG_BOOTSTRAP:
        if ( message->code == COAP_POST )
        {
            result = bootstrap_handleFinish( contextP, fromSessionH );
        }
        break;
#endif

        default:
        result = COAP_IGNORE;
        break;
    }

    coap_set_status_code( response, result );

    if ( COAP_IGNORE < result && result < COAP_400_BAD_REQUEST )
    {
        result = NO_ERROR;
    }

    nbiot_free( uriP );
    return result;
}

/* This function is an adaptation of function coap_receive() from Erbium's er-coap-13-engine.c[coap.h,coap.c].
* Erbium is Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
* All rights reserved.
*/
void lwm2m_handle_packet( lwm2m_context_t * contextP,
                          uint8_t * buffer,
                          int length,
                          void * fromSessionH )
{
    coap_status_t coap_error_code = NO_ERROR;
    static coap_packet_t message[1];
    static coap_packet_t response[1];

    LOG( "Entering" );
    coap_error_code = coap_parse_message( message, buffer, (uint16_t)length );
    if ( coap_error_code == NO_ERROR )
    {
        LOG_ARG( "Parsed: ver %u, type %u, tkl %u, code %u.%.2u, mid %u, Content type: %d",
                 message->version, message->type, message->token_len, message->code >> 5, message->code & 0x1F, message->mid, message->content_type );
        LOG_ARG( "Payload: %.*s", message->payload_len, message->payload );
        if ( message->code >= COAP_GET && message->code <= COAP_DELETE )
        {
            uint32_t block_num = 0;
            uint16_t block_size = REST_MAX_CHUNK_SIZE;
            uint32_t block_offset = 0;
            int64_t new_offset = 0;

            /* prepare response */
            if ( message->type == COAP_TYPE_CON )
            {
                /* Reliable CON requests are answered with an ACK. */
                coap_init_message( response, COAP_TYPE_ACK, COAP_205_CONTENT, message->mid );
            }
            else
            {
                /* Unreliable NON requests are answered with a NON as well. */
                coap_init_message( response, COAP_TYPE_NON, COAP_205_CONTENT, contextP->nextMID++ );
            }

            /* mirror token */
            if ( message->token_len )
            {
                coap_set_header_token( response, message->token, message->token_len );
            }

            /* get offset for blockwise transfers */
            if ( coap_get_header_block2( message, &block_num, NULL, &block_size, &block_offset ) )
            {
                LOG_ARG( "Blockwise: block request %u (%u/%u) @ %u bytes", block_num, block_size, REST_MAX_CHUNK_SIZE, block_offset );
                block_size = MIN( block_size, REST_MAX_CHUNK_SIZE );
                new_offset = block_offset;
            }

            /* handle block1 option */
            if ( IS_OPTION( message, COAP_OPTION_BLOCK1 ) )
            {
                /* get server */
                lwm2m_server_t * serverP;
                serverP = utils_findServer( contextP, fromSessionH );
#ifdef LWM2M_BOOTSTRAP
                if ( serverP == NULL )
                {
                    serverP = utils_findBootstrapServer( contextP, fromSessionH );
                }
#endif
                if ( serverP == NULL )
                {
                    coap_error_code = COAP_500_INTERNAL_SERVER_ERROR;
                }
                else
                {
                    uint32_t block1_num;
                    uint8_t  block1_more;
                    uint16_t block1_size;
                    uint8_t * complete_buffer = NULL;
                    size_t complete_buffer_size;

                    /* parse block1 header */
                    coap_get_header_block1( message, &block1_num, &block1_more, &block1_size, NULL );
                    LOG_ARG( "Blockwise: block1 request NUM %u (SZX %u/ SZX Max%u) MORE %u", block1_num, block1_size, REST_MAX_CHUNK_SIZE, block1_more );

                    /* handle block 1 */
                    coap_error_code = coap_block1_handler( &serverP->block1Data, message->mid, message->payload, message->payload_len, block1_size, block1_num, block1_more, &complete_buffer, &complete_buffer_size );

                    /* if payload is complete, replace it in the coap message. */
                    if ( coap_error_code == NO_ERROR )
                    {
                        message->payload = complete_buffer;
                        message->payload_len = complete_buffer_size;
                    }
                    else if ( coap_error_code == COAP_231_CONTINUE )
                    {
                        block1_size = MIN( block1_size, REST_MAX_CHUNK_SIZE );
                        coap_set_header_block1( response, block1_num, block1_more, block1_size );
                    }
                }
            }
            if ( coap_error_code == NO_ERROR )
            {
                coap_error_code = handle_request( contextP, fromSessionH, message, response );
            }
            if ( coap_error_code == NO_ERROR )
            {
                if ( IS_OPTION( message, COAP_OPTION_BLOCK2 ) )
                {
                    /* unchanged new_offset indicates that resource is unaware of blockwise transfer */
                    if ( new_offset == block_offset )
                    {
                        LOG_ARG( "Blockwise: unaware resource with payload length %u/%u", response->payload_len, block_size );
                        if ( block_offset >= response->payload_len )
                        {
                            LOG( "handle_incoming_data(): block_offset >= response->payload_len" );

                            response->code = COAP_402_BAD_OPTION;
                            coap_set_payload( response, "BlockOutOfScope", 15 ); /* a const char str[] and sizeof(str) produces larger code size */
                        }
                        else
                        {
                            coap_set_header_block2( response, block_num, response->payload_len - block_offset > block_size, block_size );
                            coap_set_payload( response, response->payload + block_offset, MIN( response->payload_len - block_offset, block_size ) );
                        } /* if (valid offset) */
                    }
                    else
                    {
                        /* resource provides chunk-wise data */
                        LOG_ARG( "Blockwise: blockwise resource, new offset %d", (int)new_offset );
                        coap_set_header_block2( response, block_num, new_offset != -1 || response->payload_len > block_size, block_size );
                        if ( response->payload_len > block_size ) coap_set_payload( response, response->payload, block_size );
                    } /* if (resource aware of blockwise) */
                }
                else if ( new_offset != 0 )
                {
                    LOG_ARG( "Blockwise: no block option for blockwise resource, using block size %u", REST_MAX_CHUNK_SIZE );

                    coap_set_header_block2( response, 0, new_offset != -1, REST_MAX_CHUNK_SIZE );
                    coap_set_payload( response, response->payload, MIN( response->payload_len, REST_MAX_CHUNK_SIZE ) );
                } /* if (blockwise request) */

                coap_error_code = message_send( contextP, response, fromSessionH );

                nbiot_free( response->payload );
                response->payload = NULL;
                response->payload_len = 0;
            }
            else if ( coap_error_code != COAP_IGNORE )
            {
                if ( 1 == coap_set_status_code( response, coap_error_code ) )
                {
                    coap_error_code = message_send( contextP, response, fromSessionH );
                }
            }
        }
        else
        {
            /* Responses */
            switch ( message->type )
            {
                case COAP_TYPE_NON:
                case COAP_TYPE_CON:
                {
                     bool done = transaction_handleResponse( contextP, fromSessionH, message, response );

                     if ( !done && message->type == COAP_TYPE_CON )
                     {
                         coap_init_message( response, COAP_TYPE_ACK, 0, message->mid );
                         coap_error_code = message_send( contextP, response, fromSessionH );
                     }
                }
                break;

                case COAP_TYPE_RST:
                /* Cancel possible subscriptions. */
                handle_reset( contextP, fromSessionH, message );
                transaction_handleResponse( contextP, fromSessionH, message, NULL );
                break;

                case COAP_TYPE_ACK:
                transaction_handleResponse( contextP, fromSessionH, message, NULL );
                break;

                default:
                break;
            }
        } /* Request or Response */
        coap_free_header( message );
    } /* if (parsed correctly) */
    else
    {
        LOG_ARG( "Message parsing failed %u.%2u", coap_error_code >> 5, coap_error_code & 0x1F );
    }

    if ( coap_error_code != NO_ERROR && coap_error_code != COAP_IGNORE )
    {
        LOG_ARG( "ERROR %u: %s", coap_error_code, coap_error_message );

        /* Set to sendable error code. */
        if ( coap_error_code >= 192 )
        {
            coap_error_code = COAP_500_INTERNAL_SERVER_ERROR;
        }
        /* Reuse input buffer for error message. */
        coap_init_message( message, COAP_TYPE_ACK, coap_error_code, message->mid );
        coap_set_payload( message, coap_error_message, nbiot_strlen( coap_error_message ) );
        message_send( contextP, message, fromSessionH );
    }
}


coap_status_t message_send( lwm2m_context_t * contextP,
                            coap_packet_t * message,
                            void * sessionH )
{
    coap_status_t result = COAP_500_INTERNAL_SERVER_ERROR;
    uint8_t * pktBuffer;
    size_t pktBufferLen = 0;
    size_t allocLen;

    LOG( "Entering" );
    allocLen = coap_serialize_get_size( message );
    LOG_ARG( "Size to allocate: %d", allocLen );
    if ( allocLen == 0 ) return COAP_500_INTERNAL_SERVER_ERROR;

    pktBuffer = (uint8_t *)nbiot_malloc( allocLen );
    if ( pktBuffer != NULL )
    {
        pktBufferLen = coap_serialize_message( message, pktBuffer );
        LOG_ARG( "coap_serialize_message() returned %d", pktBufferLen );
        if ( 0 != pktBufferLen )
        {
            result = lwm2m_buffer_send( sessionH, pktBuffer, pktBufferLen, contextP->userData );
        }
        nbiot_free( pktBuffer );
    }

    return result;
}
