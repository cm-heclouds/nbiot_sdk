/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "coap.h"

int coap_init_header( coap_t     *coap,
                      uint8_t     type,
                      uint8_t     code,
                      uint16_t    mid,
                      const void *token,
                      uint8_t     token_len )
{
    if ( COAP_HEADER_SIZE + token_len > coap->size )
    {
        return -1;
    }

    /* header */
    coap->buffer[0]  = COAP_VERSION_MASK & ((COAP_VERSION)<<COAP_VERSION_POSITION);
    coap->buffer[0] |= COAP_TYPE_MASK & (type<<COAP_TYPE_POSITION);
    coap->buffer[0] |= COAP_TOKEN_LEN_MASK & (token_len<<COAP_TOKEN_LEN_POSITION);
    coap->buffer[1]  = code;
    coap->buffer[2]  = (uint8_t)(mid >> 8);
    coap->buffer[3]  = (uint8_t)(mid & 0xff);
    coap->offset     = COAP_HEADER_SIZE + token_len;
    coap->option     = 0;

    /* token */
    if ( token )
    {
        const uint8_t *source = (const uint8_t*)token;
        uint8_t *dest = coap->buffer + COAP_HEADER_SIZE;

        while ( token_len )
        {
            token_len--;
            *dest++ = *source++;
        }
    }

    return 0;
}

int coap_add_option( coap_t     *coap,
                     uint16_t    option,
                     const void *value,
                     uint16_t    length )
{
    uint8_t *flag;
    uint16_t delta = 0;
    uint16_t offset = coap->offset;
    const uint8_t *source = (const uint8_t*)value;

    if ( coap->option <= option )
    {
        delta = option - coap->option;
    }
    else
    {
        return -1;
    }

    /* option header */
    flag = coap->buffer + offset;
    if ( delta < 13 )
    {
        if ( offset + length + 1 <= coap->size )
        {
            coap->buffer[offset++] = (uint8_t)(delta << 4);
        }
        else
        {
            return -1;
        }
    }
    else if ( delta < 269 )
    {
        if ( offset + length + 2 <= coap->size )
        {
            coap->buffer[offset++] = (13 << 4);
            coap->buffer[offset++] = (uint8_t)(delta - 13);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if ( offset + length + 3 <= coap->size )
        {
            coap->buffer[offset++] = (14 << 4);
            coap->buffer[offset++] = (uint8_t)((delta - 269) >> 8);
            coap->buffer[offset++] = (uint8_t)((delta - 269) & 0xf);
        }
        else
        {
            return -1;
        }
    }

    if ( length < 13 )
    {
        *flag |= (uint8_t)length;
    }
    else if ( length < 269 )
    {
        if ( offset + length + 1 <= coap->size )
        {
            *flag                 |= 13;
            coap->buffer[offset++] = (uint8_t)(length - 13);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if ( offset + length + 2 <= coap->size )
        {
            *flag                 |= 14;
            coap->buffer[offset++] = (uint8_t)((length - 269) >> 8);
            coap->buffer[offset++] = (uint8_t)((length - 269) & 0xf);
        }
        else
        {
            return -1;
        }
    }

    /* option value */
    if ( value )
    {
        while ( length )
        {
            length--;
            coap->buffer[offset++] = *source++;
        }
    }

    coap->option = option;
    coap->offset = offset;
    return 0;
}

static int coap_nibble( const uint8_t *buffer,
                        uint16_t      *nibble )
{
    int ret = 0;

    if ( 13 == *nibble )
    {
        ret      = 1;
        *nibble  = buffer[0];
        *nibble += 13;
    }

    if ( 14 == *nibble )
    {
        ret      = 2;
        *nibble  = (uint16_t)buffer[0] << 8;
        *nibble |= buffer[1];
        *nibble += 269;
    }

    return ret;
}

int coap_option( const uint8_t  *buffer,
                 uint16_t        buffer_len,
                 uint16_t        option,
                 const uint8_t **value,
                 uint16_t       *value_len,
                 bool            first )
{
    uint8_t temp;
    uint16_t offset;
    uint16_t last;
    uint16_t nibble;
    const uint8_t *data;

    if ( first )
    {
        last    = 0;
        offset  = COAP_HEADER_SIZE;
        offset += (buffer[0]&COAP_TOKEN_LEN_MASK)>>COAP_TOKEN_LEN_POSITION;
    }
    else
    {
        last   = option;
        offset = 0;
    }

    data = buffer + offset;
    while ( offset < buffer_len && *data != 0xff )
    {
        ++offset;
        temp = *data++;

        /* option delta */
        nibble  = temp >> 4;
        offset += coap_nibble( data, &nibble );
        last   += nibble;
        data    = buffer + offset;

        /* check */
        if ( last > option )
        {
            return 0;
        }

        /* option length */
        nibble  = temp & 0xf;
        offset += coap_nibble( data, &nibble );
        data    = buffer + offset;

        /* check */
        if ( last == option )
        {
            if ( value )
            {
                *value = data;
            }

            if ( value_len )
            {
                *value_len = (uint8_t)nibble;
            }

            return offset + nibble;
        }

        /* next */
        offset += nibble;
        data   += nibble;
    }

    return 0;
}

#ifdef NBIOT_BOOTSTRAP
int coap_payload( const uint8_t  *buffer,
                  uint16_t        buffer_len,
                  const uint8_t **payload,
                  uint16_t       *payload_len )
{
    uint8_t  temp;
    uint16_t offset;
    uint16_t nibble;
    const uint8_t *data;

    offset  = COAP_HEADER_SIZE;
    offset += (buffer[0]&COAP_TOKEN_LEN_MASK)>>COAP_TOKEN_LEN_POSITION;
    data    = buffer + offset;
    while ( offset < buffer_len && *data != 0xff )
    {
        ++offset;
        temp = *data++;

        /* option delta */
        nibble  = temp >> 4;
        offset += coap_nibble( data, &nibble );
        data    = buffer + offset;

        /* option length */
        nibble  = temp & 0xf;
        offset += coap_nibble( data, &nibble );
        offset += nibble;
        data    = buffer + offset;
    }

    if ( offset < buffer_len )
    {
        if ( payload )
        {
            *payload = data + 1;
        }

        if ( payload_len )
        {
            *payload_len = buffer_len - offset - 1;
        }

        return buffer_len;
    }

    return 0;
}
#endif

int coap_add_int_option( coap_t  *coap,
                         uint16_t option,
                         uint32_t value )
{
    uint8_t buffer[4];
    uint8_t length = 0;

    while ( value )
    {
        length++;
        buffer[4-length] = (uint8_t)(value & 0xff);
        value            >>= 8;
    }

    return coap_add_option( coap,
                            option,
                            buffer+4-length,
                            length );
}

int coap_int_option( const uint8_t *buffer,
                     uint16_t       buffer_len,
                     uint16_t       option,
                     uint32_t      *value )
{
    uint16_t val_len;
    const uint8_t *val;

    if ( coap_option(buffer,
                     buffer_len,
                     option,
                     &val,
                     &val_len,
                     true) )
    {
        if ( value )
        {
            *value = 0;
            while ( val_len )
            {
                --val_len;
                *value <<= 8;
                *value  += *val++;
            }
        }

        return 0;
    }

    return -1;
}