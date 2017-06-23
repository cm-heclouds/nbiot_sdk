/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

#define TLV_TYPE_MASK 0xc0

int nbiot_tlv_length( uint16_t id, size_t len )
{
    size_t ret = len + 2;

    if ( id & 0xff00 )
    {
        ++ret;
    }

    if ( len > 0xffff )
    {
        ret += 3;
    }
    else if ( len > 0xff )
    {
        ret += 2;
    }
    else if ( len > 0x7 )
    {
        ret += 1;
    }

    return ret;
}

static inline size_t nbiot_tlv_header( uint8_t header )
{
    int ret = 2;

    if ( header & 0x20 )
    {
        ++ret;
    }

    header &= 0x18;
    if ( 0x08 == header )
    {
        return (ret + 1);
    }

    if ( 0x10 == header )
    {
        return (ret + 2);
    }

    if ( 0x18 == header )
    {
        return (ret + 3);
    }

    return ret;
}

int nbiot_tlv_decode( const uint8_t  *buffer,
                      size_t          buffer_len,
                      uint8_t        *type,
                      uint16_t       *id,
                      const uint8_t **value,
                      size_t         *value_len )
{
    size_t len;
    size_t offset;
    size_t header;

    if ( !buffer && !buffer_len )
    {
        return 0;
    }

    header = nbiot_tlv_header( buffer[0] );
    if ( header > buffer_len )
    {
        return 0;
    }

    if ( type )
    {
        *type = buffer[0] & TLV_TYPE_MASK;
    }

    if ( buffer[0] & 0x20 )
    {
        offset = 3;
        if ( id )
        {
            *id  = ((uint16_t)buffer[1]) << 8;
            *id |= buffer[2];
        }
    }
    else
    {
        offset = 2;
        if ( id )
        {
            *id = buffer[1];
        }
    }

    switch ( buffer[0] & 0x18 )
    {
        case 0x00:
        {
            len = buffer[0] & 0x07;
        }
        break;

        case 0x08:
        {
            len = buffer[offset];
        }
        break;

        case 0x10:
        {
            len  = ((uint16_t)buffer[offset]) << 8;
            len |= buffer[offset+1];
        }
        break;

        case 0x18:
        {
            len  = ((uint32_t)buffer[offset]) << 16;
            len |= ((uint16_t)buffer[offset+1]) << 8;
            len |= buffer[offset+2];
        }
        break;

        default:
        {
            /* can't happen */
            return 0;
        }
        break;
    }

    if ( header + len > buffer_len )
    {
        return 0;
    }

    if ( value )
    {
        *value = buffer + header;
    }

    if ( value_len )
    {
        *value_len = len;
    }

    return header + len;
}

int nbiot_tlv_encode( uint8_t       *buffer,
                      size_t         buffer_len,
                      uint8_t        type,
                      uint16_t       id,
                      const uint8_t *value,
                      size_t         value_len )
{
    size_t offset;

    if ( !buffer || !buffer_len )
    {
        return 0;
    }

    if ( nbiot_tlv_length(id,value_len) > (int)buffer_len )
    {
        return 0;
    }

    buffer[0] = type & TLV_TYPE_MASK;
    if ( id & 0xff00 )
    {
        buffer[0] |= 0x20;
        buffer[1]  = (uint8_t)(id >> 8);
        buffer[2]  = (uint8_t)(id & 0xff);
        offset     = 3;
    }
    else
    {
        buffer[1] = (uint8_t)id;
        offset    = 2;
    }

    if ( value_len <= 7 )
    {
        buffer[0] |= value_len;
    }
    else if ( value_len <= 0xff )
    {
        buffer[offset] = value_len;
        buffer[0]     |= 0x08;
        offset        += 1;
    }
    else if ( value_len <= 0xffff )
    {
        buffer[offset]   = (uint8_t)((value_len>>8) & 0xff);
        buffer[offset+1] = (uint8_t)(value_len & 0xff);
        buffer[0]       |= 0x10;
        offset          += 2;
    }
    else if ( value_len <= 0xffffff )
    {
        buffer[offset]   = (uint8_t)((value_len>>16) & 0xff);
        buffer[offset+1] = (uint8_t)((value_len>>8) & 0xff);
        buffer[offset+2] = (uint8_t)(value_len & 0xff);
        buffer[0]       |= 0x18;
        offset          += 3;
    }
    else
    {
        return 0;
    }

    if ( value )
    {
        while ( value_len > 0 )
        {
            --value_len;
            buffer[offset++] = *value++;
        }
    }

    return offset;
}