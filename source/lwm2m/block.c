/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

/* the maximum payload transfered by block1 we accumulate per server */
#define MAX_BLOCK1_SIZE 4096

coap_status_t coap_block1_handler( lwm2m_block1_data_t ** pBlock1Data,
                                   uint16_t mid,
                                   uint8_t * buffer,
                                   size_t length,
                                   uint16_t blockSize,
                                   uint32_t blockNum,
                                   bool blockMore,
                                   uint8_t ** outputBuffer,
                                   size_t * outputLength )
{
    lwm2m_block1_data_t * block1Data = *pBlock1Data;;

    /* manage new block1 transfer */
    if ( blockNum == 0 )
    {
        /* we already have block1 data for this server, clear it */
        if ( block1Data != NULL )
        {
            nbiot_free( block1Data->block1buffer );
        }
        else
        {
            block1Data = nbiot_malloc( sizeof(lwm2m_block1_data_t) );
            *pBlock1Data = block1Data;
            if ( NULL == block1Data ) return COAP_500_INTERNAL_SERVER_ERROR;
        }

        block1Data->block1buffer = nbiot_malloc( length );
        block1Data->block1bufferSize = length;

        /* write new block in buffer */
        nbiot_memmove( block1Data->block1buffer, buffer, length );
        block1Data->lastmid = mid;
    }
    /* manage already started block1 transfer */
    else
    {
        if ( block1Data == NULL )
        {
            /* we never receive the first block */
            /* TODO should we clean block1 data for this server ? */
            return COAP_408_REQ_ENTITY_INCOMPLETE;
        }

        /* If this is a retransmission, we already did that. */
        if ( block1Data->lastmid != mid )
        {
            uint8_t * oldBuffer;
            size_t oldSize;

            if ( block1Data->block1bufferSize != blockSize * blockNum )
            {
                /* we don't receive block in right order */
                /* TODO should we clean block1 data for this server ? */
                return COAP_408_REQ_ENTITY_INCOMPLETE;
            }

            /* is it too large? */
            if ( block1Data->block1bufferSize + length >= MAX_BLOCK1_SIZE )
            {
                return COAP_413_ENTITY_TOO_LARGE;
            }

            /* re-alloc new buffer */
            oldBuffer = block1Data->block1buffer;
            oldSize = block1Data->block1bufferSize;
            block1Data->block1bufferSize = oldSize + length;
            block1Data->block1buffer = nbiot_malloc( block1Data->block1bufferSize );
            if ( NULL == block1Data->block1buffer ) return COAP_500_INTERNAL_SERVER_ERROR;
            nbiot_memmove( block1Data->block1buffer, oldBuffer, oldSize );
            nbiot_free( oldBuffer );

            /* write new block in buffer */
            nbiot_memmove( block1Data->block1buffer + oldSize, buffer, length );
            block1Data->lastmid = mid;
        }
    }

    if ( blockMore )
    {
        *outputLength = -1;
        return COAP_231_CONTINUE;
    }
    else
    {
        /* buffer is full, set output parameter */
        /* we don't free it to be able to send retransmission */
        *outputLength = block1Data->block1bufferSize;
        *outputBuffer = block1Data->block1buffer;

        return NO_ERROR;
    }
}

void free_block1_buffer( lwm2m_block1_data_t * block1Data )
{
    if ( block1Data != NULL )
    {
        /* free block1 buffer */
        nbiot_free( block1Data->block1buffer );
        block1Data->block1bufferSize = 0;

        /* free current element */
        nbiot_free( block1Data );
    }
}