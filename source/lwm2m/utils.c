/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

int utils_plainTextToInt64( uint8_t * buffer,
                            int length,
                            int64_t * dataP )
{
    uint64_t result = 0;
    int sign = 1;
    int i = 0;

    if ( 0 == length ) return 0;

    if ( buffer[0] == '-' )
    {
        sign = -1;
        i = 1;
    }

    while ( i < length )
    {
        if ( '0' <= buffer[i] && buffer[i] <= '9' )
        {
            if ( result >( UINT64_MAX / 10 ) ) return 0;
            result *= 10;
            result += buffer[i] - '0';
        }
        else
        {
            return 0;
        }
        i++;
    }

    if ( result > INT64_MAX ) return 0;

    if ( sign == -1 )
    {
        *dataP = 0 - result;
    }
    else
    {
        *dataP = result;
    }

    return 1;
}

int utils_plainTextToFloat64( uint8_t * buffer,
                              int length,
                              double * dataP )
{
    double result;
    int sign;
    int i;

    if ( 0 == length ) return 0;

    if ( buffer[0] == '-' )
    {
        sign = -1;
        i = 1;
    }
    else
    {
        sign = 1;
        i = 0;
    }

    result = 0;
    while ( i < length && buffer[i] != '.' )
    {
        if ( '0' <= buffer[i] && buffer[i] <= '9' )
        {
            if ( result >( DBL_MAX / 10 ) ) return 0;
            result *= 10;
            result += (buffer[i] - '0');
        }
        else
        {
            return 0;
        }
        i++;
    }
    if ( buffer[i] == '.' )
    {
        double dec;

        i++;
        if ( i == length ) return 0;

        dec = 0.1;
        while ( i < length )
        {
            if ( '0' <= buffer[i] && buffer[i] <= '9' )
            {
                if ( result >( DBL_MAX - 1 ) ) return 0;
                result += (buffer[i] - '0') * dec;
                dec /= 10;
            }
            else
            {
                return 0;
            }
            i++;
        }
    }

    *dataP = result * sign;
    return 1;
}

size_t utils_intToText( int64_t data,
                        uint8_t * string,
                        size_t length )
{
    int index;
    bool minus;
    size_t result;

    if ( data < 0 )
    {
        minus = true;
        data = 0 - data;
    }
    else
    {
        minus = false;
    }

    index = length - 1;
    do
    {
        string[index] = '0' + data % 10;
        data /= 10;
        index--;
    }
    while ( index >= 0 && data > 0 );

    if ( data > 0 ) return 0;

    if ( minus == true )
    {
        if ( index == 0 ) return 0;
        string[index] = '-';
    }
    else
    {
        index++;
    }

    result = length - index;

    if ( result < length )
    {
        nbiot_memmove( string, string + index, result );
    }

    return result;
}

size_t utils_floatToText( double data,
                          uint8_t * string,
                          size_t length )
{
    size_t intLength;
    size_t decLength;
    int64_t intPart;
    double decPart;

    if ( data <= (double)INT64_MIN || data >= (double)INT64_MAX ) return 0;

    intPart = (int64_t)data;
    decPart = data - intPart;
    if ( decPart < 0 )
    {
        decPart = 1 - decPart;
    }
    else
    {
        decPart = 1 + decPart;
    }

    if ( decPart <= 1 + FLT_EPSILON )
    {
        decPart = 0;
    }

    if ( intPart == 0 && data < 0 )
    {
        /* deal with numbers between -1 and 0 */
        if ( length < 4 ) return 0;   /* "-0.n" */
        string[0] = '-';
        string[1] = '0';
        intLength = 2;
    }
    else
    {
        intLength = utils_intToText( intPart, string, length );
        if ( intLength == 0 ) return 0;
    }
    decLength = 0;
    if ( decPart >= FLT_EPSILON )
    {
        int i;
        double noiseFloor;

        if ( intLength >= length - 1 ) return 0;

        i = 0;
        noiseFloor = FLT_EPSILON;
        do
        {
            decPart *= 10;
            noiseFloor *= 10;
            i++;
        }
        while ( decPart - (int64_t)decPart > noiseFloor );

        decLength = utils_intToText( (int64_t)decPart, string + intLength, length - intLength );
        if ( decLength <= 1 ) return 0;

        /* replace the leading 1 with a dot */
        string[intLength] = '.';
    }

    return intLength + decLength;
}

size_t utils_int64ToPlainText( int64_t data,
                               uint8_t ** bufferP )
{
#define _PRV_STR_LENGTH 32
    uint8_t string[_PRV_STR_LENGTH];
    size_t length;

    length = utils_intToText( data, string, _PRV_STR_LENGTH );
    if ( length == 0 ) return 0;

    *bufferP = (uint8_t *)nbiot_malloc( length );
    if ( NULL == *bufferP ) return 0;

    nbiot_memmove( *bufferP, string, length );

    return length;
}


size_t utils_float64ToPlainText( double data,
                                 uint8_t ** bufferP )
{
    uint8_t string[_PRV_STR_LENGTH * 2];
    size_t length;

    length = utils_floatToText( data, string, _PRV_STR_LENGTH * 2 );
    if ( length == 0 ) return 0;

    *bufferP = (uint8_t *)nbiot_malloc( length );
    if ( NULL == *bufferP ) return 0;

    nbiot_memmove( *bufferP, string, length );

    return length;
}


size_t utils_boolToPlainText( bool data,
                              uint8_t ** bufferP )
{
    return utils_int64ToPlainText( (int64_t)(data ? 1 : 0), bufferP );
}

lwm2m_media_type_t utils_convertMediaType( coap_content_type_t type )
{
    /* Here we just check the content type is a valid value for LWM2M */
    switch ( (uint16_t)type )
    {
        case TEXT_PLAIN:
        return LWM2M_CONTENT_TEXT;
        case APPLICATION_OCTET_STREAM:
        return LWM2M_CONTENT_OPAQUE;
        case LWM2M_CONTENT_TLV:
        case LWM2M_CONTENT_TLV_OLD:
        return LWM2M_CONTENT_TLV;
        case LWM2M_CONTENT_JSON:
        case LWM2M_CONTENT_JSON_OLD:
        return LWM2M_CONTENT_JSON;
        case APPLICATION_LINK_FORMAT:
        return LWM2M_CONTENT_LINK;

        default:
        return LWM2M_CONTENT_TEXT;
    }
}

lwm2m_server_t * utils_findServer( lwm2m_context_t * contextP,
                                   void * fromSessionH )
{
    lwm2m_server_t * targetP;

    targetP = contextP->serverList;
    while ( targetP != NULL
            && false == lwm2m_session_is_equal( targetP->sessionH, fromSessionH, contextP->userData ) )
    {
        targetP = targetP->next;
    }

    return targetP;
}

lwm2m_server_t * utils_findBootstrapServer( lwm2m_context_t * contextP,
                                            void * fromSessionH )
{
    lwm2m_server_t * targetP;

    targetP = contextP->bootstrapServerList;
    while ( targetP != NULL
            && false == lwm2m_session_is_equal( targetP->sessionH, fromSessionH, contextP->userData ) )
    {
        targetP = targetP->next;
    }

    return targetP;
}

/* copy a string in a buffer. */
/* return the number of copied bytes or -1 if the buffer is not large enough */
int utils_stringCopy( char * buffer,
                      size_t length,
                      const char * str )
{
    size_t i;

    for ( i = 0; i < length && str[i] != 0; i++ )
    {
        buffer[i] = str[i];
    }

    if ( i == length ) return -1;

    buffer[i] = 0;

    return (int)i;
}

int utils_intCopy( char * buffer,
                   size_t length,
                   int32_t value )
{
#define _PRV_INT32_MAX_STR_LEN 11
    uint8_t str[_PRV_INT32_MAX_STR_LEN];
    size_t len;

    len = utils_intToText( value, str, _PRV_INT32_MAX_STR_LEN );
    if ( len == 0 ) return -1;
    if ( len > length + 1 ) return -1;

    nbiot_memmove( buffer, str, len );
    buffer[len] = 0;

    return len;
}

void utils_copyValue( void * dst,
                      const void * src,
                      size_t len )
{
#ifdef BIG_ENDIAN
    nbiot_memmove( dst, src, len );
#else
#ifdef LITTLE_ENDIAN
    size_t i;

    for ( i = 0; i < len; i++ )
    {
        ((uint8_t *)dst)[i] = ((uint8_t *)src)[len - 1 - i];
    }
#endif
#endif
}

int utils_opaqueToInt( const uint8_t * buffer,
                       size_t buffer_len,
                       int64_t * dataP )
{
    *dataP = 0;

    switch ( buffer_len )
    {
        case 1:
        {
            *dataP = (int8_t)buffer[0];

            break;
        }

        case 2:
        {
            int16_t value;

            utils_copyValue( &value, buffer, buffer_len );

            *dataP = value;
            break;
        }

        case 4:
        {
            int32_t value;

            utils_copyValue( &value, buffer, buffer_len );

            *dataP = value;
            break;
        }

        case 8:
        utils_copyValue( dataP, buffer, buffer_len );
        return buffer_len;

        default:
        return 0;
    }

    return buffer_len;
}

int utils_opaqueToFloat( const uint8_t * buffer,
                         size_t buffer_len,
                         double * dataP )
{
    switch ( buffer_len )
    {
        case 4:
        {
            float temp;

            utils_copyValue( &temp, buffer, buffer_len );

            *dataP = temp;
        }
        return 4;

        case 8:
        utils_copyValue( dataP, buffer, buffer_len );
        return 8;

        default:
        return 0;
    }
}

/**
* Encode an integer value to a byte representation.
* Returns the length of the result. For values < 0xff length is 1,
* for values < 0xffff length is 2 and so on.
* @param data        Input value
* @param data_buffer Result in data_buffer is in big endian encoding
*                    Negative values are represented in two's complement as of
*                    OMA-TS-LightweightM2M-V1_0-20160308-D, Appendix C
*/
size_t utils_encodeInt( int64_t data,
                        uint8_t data_buffer[_PRV_64BIT_BUFFER_SIZE] )
{
    size_t length = 0;

    nbiot_memzero( data_buffer, _PRV_64BIT_BUFFER_SIZE );

    if ( data >= INT8_MIN && data <= INT8_MAX )
    {
        length = 1;
        data_buffer[0] = (uint8_t)data;
    }
    else if ( data >= INT16_MIN && data <= INT16_MAX )
    {
        int16_t value;

        value = (int16_t)data;
        length = 2;
        data_buffer[0] = (value >> 8) & 0xFF;
        data_buffer[1] = value & 0xFF;
    }
    else if ( data >= INT32_MIN && data <= INT32_MAX )
    {
        int32_t value;

        value = (int32_t)data;
        length = 4;
        utils_copyValue( data_buffer, &value, length );
    }
    else if ( data >= INT64_MIN && data <= INT64_MAX )
    {
        length = 8;
        utils_copyValue( data_buffer, &data, length );
    }

    return length;
}

size_t utils_encodeFloat( double data,
                          uint8_t data_buffer[_PRV_64BIT_BUFFER_SIZE] )
{
    size_t length = 0;

    nbiot_memzero( data_buffer, _PRV_64BIT_BUFFER_SIZE );

    if ( (data < 0.0 - (double)FLT_MAX) || (data >( double )FLT_MAX) )
    {
        length = 8;
        utils_copyValue( data_buffer, &data, 8 );
    }
    else
    {
        float value;

        length = 4;
        value = (float)data;
        utils_copyValue( data_buffer, &value, 4 );
    }

    return length;
}
