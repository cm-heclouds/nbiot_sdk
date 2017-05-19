/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "coap.h"
#include <utils.h>

#ifdef COAP_WITH_LOGS
#define PRINTF(...) nbiot_printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Option format serialization*/
#define COAP_SERIALIZE_INT_OPTION(number, field, text) \
if ( IS_OPTION( coap_pkt, number ) ) \
{ \
    PRINTF( text" [%u]\n", coap_pkt->field ); \
    option += coap_serialize_int_option( number, \
                                         current_number, \
                                         option, \
                                         coap_pkt->field ); \
    current_number = number; \
}
#define COAP_SERIALIZE_BYTE_OPTION(number, field, text) \
if ( IS_OPTION( coap_pkt, number ) ) \
{ \
    /*FIXME always prints 8 bytes */ \
    PRINTF( text" %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", \
            coap_pkt->field##_len, \
            coap_pkt->field[0], \
            coap_pkt->field[1], \
            coap_pkt->field[2], \
            coap_pkt->field[3], \
            coap_pkt->field[4], \
            coap_pkt->field[5], \
            coap_pkt->field[6], \
            coap_pkt->field[7] ); \
    option += coap_serialize_array_option( number, \
                                           current_number, \
                                           option, \
                                           coap_pkt->field, \
                                           coap_pkt->field##_len, \
                                           '\0' ); \
    current_number = number; \
}
#define COAP_SERIALIZE_STRING_OPTION(number, field, splitter, text) \
if ( IS_OPTION( coap_pkt, number ) ) \
{ \
    PRINTF( text" [%.*s]\n", \
            coap_pkt->field##_len, \
            coap_pkt->field ); \
    option += coap_serialize_array_option( number, \
                                           current_number, \
                                           option, \
                                           (uint8_t *)coap_pkt->field, \
                                           coap_pkt->field##_len, \
                                           splitter ); \
    current_number = number; \
}
#define COAP_SERIALIZE_MULTI_OPTION(number, field, text) \
if ( IS_OPTION( coap_pkt, number ) ) \
{ \
        PRINTF( text ); \
        option += coap_serialize_multi_option( number, \
                                               current_number, \
                                               option, \
                                               coap_pkt->field ); \
        current_number = number; \
}
#define COAP_SERIALIZE_ACCEPT_OPTION(number, field, text) \
if ( IS_OPTION( coap_pkt, number ) ) \
{ \
    int i; \
    for ( i = 0; i < coap_pkt->field##_num; ++i ) \
    { \
        PRINTF( text" [%u]\n", coap_pkt->field[i] ); \
        option += coap_serialize_int_option( number, \
                                             current_number, \
                                             option, \
                                             coap_pkt->field[i] ); \
        current_number = number; \
    } \
}
#define COAP_SERIALIZE_BLOCK_OPTION(number, field, text) \
if ( IS_OPTION( coap_pkt, number ) ) \
{ \
    uint32_t block; \
    PRINTF( text" [%lu%s (%u B/blk)]\n", \
            coap_pkt->field##_num, \
            coap_pkt->field##_more ? "+" : "", \
            coap_pkt->field##_size ); \
    block = coap_pkt->field##_num << 4; \
    if ( coap_pkt->field##_more ) \
    { \
        block |= 0x8; \
    } \
    block |= 0xF & coap_log_2( coap_pkt->field##_size / 16 ); \
    PRINTF( text" encoded: 0x%lX\n", block ); \
    option += coap_serialize_int_option( number, \
                                         current_number, \
                                         option, \
                                         block ); \
    current_number = number; \
}

typedef enum
{
    COAP_OPTION_IF_MATCH       = 1,  /* 0-8 B */
    COAP_OPTION_URI_HOST       = 3,  /* 1-255 B */
    COAP_OPTION_ETAG           = 4,  /* 1-8 B */
    COAP_OPTION_IF_NONE_MATCH  = 5,  /* 0 B */
    COAP_OPTION_URI_PORT       = 7,  /* 0-2 B */
    COAP_OPTION_MAX_AGE        = 14, /* 0-4 B */
    COAP_OPTION_LOCATION_QUERY = 20, /* 1-270 B */
    COAP_OPTION_SIZE           = 28, /* 0-4 B */
    COAP_OPTION_PROXY_URI      = 35  /* 1-270 B */
} coap_option_ignore_t;

const char *coap_error_message = "";
coap_status_t coap_error_code = NO_ERROR;

uint16_t coap_log_2( uint16_t value )
{
    uint16_t result = 0;
    do
    {
        value = value >> 1;
        result++;
    }
    while ( value );

    return result ? result - 1 : result;
}

static uint32_t coap_parse_int_option( uint8_t *bytes,
                                       size_t   length )
{
    uint32_t var = 0;
    size_t i = 0;
    while ( i < length )
    {
        var <<= 8;
        var |= bytes[i++];
    }
    return var;
}

static uint8_t coap_option_nibble( unsigned int value )
{
    if ( value < 13 )
    {
        return value;
    }
    else if ( value <= 0xFF + 13 )
    {
        return 13;
    }
    else
    {
        return 14;
    }
}

static size_t coap_set_option_header( unsigned int delta,
                                      size_t       length,
                                      uint8_t     *buffer )
{
    size_t written = 0;
    unsigned int *x;

    buffer[0] = coap_option_nibble( delta ) << 4 | coap_option_nibble( length );

    /* avoids code duplication without function overhead */
    x = &delta;
    do
    {
        if ( *x > 268 )
        {
            buffer[++written] = (*x - 269) >> 8;
            buffer[++written] = (*x - 269);
        }
        else if ( *x > 12 )
        {
            buffer[++written] = (*x - 13);
        }
    }
    while ( x != (unsigned int *)&length && (x = (unsigned int *)&length) );

    PRINTF( "WRITTEN %u B opt header\n", written );

    return ++written;
}

static size_t coap_serialize_int_option( unsigned int number,
                                         unsigned int current_number,
                                         uint8_t     *buffer,
                                         uint32_t     value )
{
    size_t i = 0;

    if ( 0xFF000000 & value ) ++i;
    if ( 0xFFFF0000 & value ) ++i;
    if ( 0xFFFFFF00 & value ) ++i;
    if ( 0xFFFFFFFF & value ) ++i;

    PRINTF( "OPTION %u (delta %u, len %u)\n", number, number - current_number, i );

    i = coap_set_option_header( number - current_number, i, buffer );

    if ( 0xFF000000 & value ) buffer[i++] = (uint8_t)(value >> 24);
    if ( 0xFFFF0000 & value ) buffer[i++] = (uint8_t)(value >> 16);
    if ( 0xFFFFFF00 & value ) buffer[i++] = (uint8_t)(value >> 8);
    if ( 0xFFFFFFFF & value ) buffer[i++] = (uint8_t)(value);

    return i;
}

static size_t coap_serialize_array_option( unsigned int number,
                                           unsigned int current_number,
                                           uint8_t     *buffer,
                                           uint8_t     *array,
                                           size_t       length,
                                           char         split_char )
{
    size_t i = 0;

    if ( split_char != '\0' )
    {
        size_t j;
        uint8_t *part_start = array;
        uint8_t *part_end = NULL;
        size_t temp_length;

        for ( j = 0; j <= length; ++j )
        {
            if ( array[j] == split_char || j == length )
            {
                part_end = array + j;
                temp_length = part_end - part_start;

                i += coap_set_option_header( number - current_number, temp_length, &buffer[i] );
                nbiot_memmove( &buffer[i], part_start, temp_length );
                i += temp_length;

                PRINTF( "OPTION type %u, delta %u, len %u, part [%.*s]\n", number, number - current_number, i, temp_length, part_start );

                ++j; /* skip the splitter */
                current_number = number;
                part_start = array + j;
            }
        } /* for */
    }
    else
    {
        i += coap_set_option_header( number - current_number, length, &buffer[i] );
        nbiot_memmove( &buffer[i], array, length );
        i += length;

        PRINTF( "OPTION type %u, delta %u, len %u\n", number, number - current_number, length );
    }

    return i;
}

static  size_t coap_serialize_multi_option( unsigned int    number,
                                            unsigned int    current_number,
                                            uint8_t        *buffer,
                                            multi_option_t *array )
{
    size_t i = 0;
    multi_option_t * j;

    for ( j = array; j != NULL; j = j->next )
    {
        i += coap_set_option_header( number - current_number, j->len, &buffer[i] );
        current_number = number;
        nbiot_memmove( &buffer[i], j->data, j->len );
        i += j->len;
    } /* for */

    return i;
}

char* coap_get_multi_option_as_string( multi_option_t * option )
{
    size_t len = 0;
    multi_option_t * opt;
    char * output;

    for ( opt = option; opt != NULL; opt = opt->next )
    {
        len += opt->len + 1;     /* for separator */
    }

    output = (char*)nbiot_malloc( len + 1 ); /* for String terminator */
    if ( output != NULL )
    {
        size_t i = 0;

        for ( opt = option; opt != NULL; opt = opt->next )
        {
            output[i] = '/';
            i += 1;

            nbiot_memmove( output + i, opt->data, opt->len );
            i += opt->len;
        }
        output[i] = 0;
    }

    return output;
}

static void coap_add_multi_option( multi_option_t **dst,
                                   uint8_t         *option,
                                   size_t           option_len,
                                   uint8_t          is_static )
{
    multi_option_t *opt = (multi_option_t *)nbiot_malloc( sizeof(multi_option_t) );

    if ( opt )
    {
        opt->next = NULL;
        opt->len = (uint8_t)option_len;
        if ( is_static )
        {
            opt->data = option;
            opt->is_static = 1;
        }
        else
        {
            opt->is_static = 0;
            opt->data = (uint8_t *)nbiot_malloc( option_len );
            if ( opt->data == NULL )
            {
                nbiot_free( opt );
                return;
            }
            nbiot_memmove( opt->data, option, option_len );
        }

        if ( *dst )
        {
            multi_option_t * i = *dst;
            while ( i->next )
            {
                i = i->next;
            }
            i->next = opt;
        }
        else
        {
            *dst = opt;
        }
    }
}

static void free_multi_option( multi_option_t *dst )
{
    if ( dst )
    {
        multi_option_t *n = dst->next;
        dst->next = NULL;
        if ( dst->is_static == 0 )
        {
            nbiot_free( dst->data );
        }
        nbiot_free( dst );
        free_multi_option( n );
    }
}

void coap_init_message( void               *packet,
                        coap_message_type_t type,
                        uint8_t             code,
                        uint16_t            mid )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    /* Important thing */
    nbiot_memzero( coap_pkt, sizeof(coap_packet_t) );

    coap_pkt->type = type;
    coap_pkt->code = code;
    coap_pkt->mid = mid;
}

size_t coap_serialize_get_size( void *packet )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;
    size_t length = 0;

    length = COAP_HEADER_LEN + coap_pkt->payload_len + coap_pkt->token_len;

    if ( IS_OPTION( coap_pkt, COAP_OPTION_OBSERVE ) )
    {
        /* can be stored in extended fields */
        length += COAP_MAX_OPTION_HEADER_LEN;
    }
    if ( IS_OPTION( coap_pkt, COAP_OPTION_LOCATION_PATH ) )
    {
        multi_option_t * optP;

        for ( optP = coap_pkt->location_path; optP != NULL; optP = optP->next )
        {
            length += COAP_MAX_OPTION_HEADER_LEN + optP->len;
        }
    }
    if ( IS_OPTION( coap_pkt, COAP_OPTION_URI_PATH ) )
    {
        multi_option_t * optP;

        for ( optP = coap_pkt->uri_path; optP != NULL; optP = optP->next )
        {
            length += COAP_MAX_OPTION_HEADER_LEN + optP->len;
        }
    }
    if ( IS_OPTION( coap_pkt, COAP_OPTION_CONTENT_TYPE ) )
    {
        /* can be stored in extended fields */
        length += COAP_MAX_OPTION_HEADER_LEN;
    }
    if ( IS_OPTION( coap_pkt, COAP_OPTION_URI_QUERY ) )
    {
        multi_option_t * optP;

        for ( optP = coap_pkt->uri_query; optP != NULL; optP = optP->next )
        {
            length += COAP_MAX_OPTION_HEADER_LEN + optP->len;
        }
    }
    if ( IS_OPTION( coap_pkt, COAP_OPTION_BLOCK2 ) )
    {
        /* can be stored in extended fields */
        length += COAP_MAX_OPTION_HEADER_LEN;
    }
    if ( IS_OPTION( coap_pkt, COAP_OPTION_BLOCK1 ) )
    {
        /* can be stored in extended fields */
        length += COAP_MAX_OPTION_HEADER_LEN;
    }
    if ( IS_OPTION( coap_pkt, COAP_OPTION_SIZE ) )
    {
        /* can be stored in extended fields */
        length += COAP_MAX_OPTION_HEADER_LEN;
    }

    return length;
}

size_t coap_serialize_message( void    *packet,
                               uint8_t *buffer )
{
  coap_packet_t *const coap_pkt = (coap_packet_t *) packet;
  uint8_t *option;
  unsigned int current_number = 0;

  /* Initialize */
  coap_pkt->buffer = buffer;
  coap_pkt->version = 1;

  PRINTF("-Serializing MID %u to %p, ", coap_pkt->mid, coap_pkt->buffer);

  /* set header fields */
  coap_pkt->buffer[0]  = 0x00;
  coap_pkt->buffer[0] |= COAP_HEADER_VERSION_MASK & (coap_pkt->version)<<COAP_HEADER_VERSION_POSITION;
  coap_pkt->buffer[0] |= COAP_HEADER_TYPE_MASK & (coap_pkt->type)<<COAP_HEADER_TYPE_POSITION;
  coap_pkt->buffer[0] |= COAP_HEADER_TOKEN_LEN_MASK & (coap_pkt->token_len)<<COAP_HEADER_TOKEN_LEN_POSITION;
  coap_pkt->buffer[1] = coap_pkt->code;
  coap_pkt->buffer[2] = (uint8_t) ((coap_pkt->mid)>>8);
  coap_pkt->buffer[3] = (uint8_t) (coap_pkt->mid);

  /* set Token */
  PRINTF("Token (len %u)", coap_pkt->token_len);
  option = coap_pkt->buffer + COAP_HEADER_LEN;
  for (current_number=0; current_number<coap_pkt->token_len; ++current_number)
  {
    PRINTF(" %02X", coap_pkt->token[current_number]);
    *option = coap_pkt->token[current_number];
    ++option;
  }
  PRINTF("-\n");

  /* Serialize options */
  current_number = 0;
  PRINTF("-Serializing options at %p-\n", option);

  /* The options must be serialized in the order of their number */
  COAP_SERIALIZE_INT_OPTION(    COAP_OPTION_OBSERVE,       observe,       "Observe" );
  COAP_SERIALIZE_MULTI_OPTION(  COAP_OPTION_LOCATION_PATH, location_path, "Location-Path" );
  COAP_SERIALIZE_MULTI_OPTION(  COAP_OPTION_URI_PATH,      uri_path,      "Uri-Path" )
  COAP_SERIALIZE_INT_OPTION(    COAP_OPTION_CONTENT_TYPE,  content_type,  "Content-Format" );
  COAP_SERIALIZE_MULTI_OPTION(  COAP_OPTION_URI_QUERY,     uri_query,     "Uri-Query" );
  COAP_SERIALIZE_BLOCK_OPTION(  COAP_OPTION_BLOCK2,        block2,        "Block2" );
  COAP_SERIALIZE_BLOCK_OPTION(  COAP_OPTION_BLOCK1,        block1,        "Block1" );

  PRINTF("-Done serializing at %p----\n", option);
  /* Free allocated header fields */
  coap_free_header(packet);

  /* Pack payload */
  /* Payload marker */
  if (coap_pkt->payload_len)
  {
    *option = 0xFF;
    ++option;
  }

  nbiot_memmove(option, coap_pkt->payload, coap_pkt->payload_len);

  PRINTF("-Done %u B (header len %u, payload len %u)-\n", coap_pkt->payload_len + option - buffer, option - buffer, coap_pkt->payload_len);

  PRINTF("Dump [0x%02X %02X %02X %02X  %02X %02X %02X %02X]\n",
      coap_pkt->buffer[0],
      coap_pkt->buffer[1],
      coap_pkt->buffer[2],
      coap_pkt->buffer[3],
      coap_pkt->buffer[4],
      coap_pkt->buffer[5],
      coap_pkt->buffer[6],
      coap_pkt->buffer[7]
    );

  return (option - buffer) + coap_pkt->payload_len; /* packet length */
}

coap_status_t coap_parse_message( void    *request,
                                  uint8_t *data,
                                  uint16_t data_len )
{
  uint8_t *current_option;
  unsigned int option_number;
  unsigned int option_delta;
  size_t option_length;
  coap_packet_t *const coap_pkt = (coap_packet_t *)request;

  /* Initialize packet */
  nbiot_memzero(coap_pkt, sizeof(coap_packet_t));

  /* pointer to packet bytes */
  coap_pkt->buffer = data;

  /* parse header fields */
  coap_pkt->version = (COAP_HEADER_VERSION_MASK & coap_pkt->buffer[0])>>COAP_HEADER_VERSION_POSITION;
  coap_pkt->type = (COAP_HEADER_TYPE_MASK & coap_pkt->buffer[0])>>COAP_HEADER_TYPE_POSITION;
  coap_pkt->token_len = MIN(COAP_TOKEN_LEN, (COAP_HEADER_TOKEN_LEN_MASK & coap_pkt->buffer[0])>>COAP_HEADER_TOKEN_LEN_POSITION);
  coap_pkt->code = coap_pkt->buffer[1];
  coap_pkt->mid = coap_pkt->buffer[2]<<8 | coap_pkt->buffer[3];

  if (coap_pkt->version != 1)
  {
    coap_error_message = "CoAP version must be 1";
    return BAD_REQUEST_4_00;
  }

  current_option = data + COAP_HEADER_LEN;

  if (coap_pkt->token_len != 0)
  {
      nbiot_memmove(coap_pkt->token, current_option, coap_pkt->token_len);
      SET_OPTION(coap_pkt, COAP_OPTION_TOKEN);

      PRINTF("Token (len %u) [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", coap_pkt->token_len,
        coap_pkt->token[0],
        coap_pkt->token[1],
        coap_pkt->token[2],
        coap_pkt->token[3],
        coap_pkt->token[4],
        coap_pkt->token[5],
        coap_pkt->token[6],
        coap_pkt->token[7]
      ); /*FIXME always prints 8 bytes */
  }

  /* parse options */
  current_option += coap_pkt->token_len;

  option_number = 0;
  option_delta = 0;
  option_length = 0;

  while (current_option < data+data_len)
  {
    unsigned int *x;

    /* Payload marker 0xFF, currently only checking for 0xF* because rest is reserved */
    if ((current_option[0] & 0xF0)==0xF0)
    {
      coap_pkt->payload = ++current_option;
      coap_pkt->payload_len = data_len - (coap_pkt->payload - data);

      break;
    }

    option_delta = current_option[0]>>4;
    option_length = current_option[0] & 0x0F;
    ++current_option;

    /* avoids code duplication without function overhead */
    x = &option_delta;
    do
    {
      if (*x==13)
      {
        *x += current_option[0];
        ++current_option;
      }
      else if (*x==14)
      {
        *x += 255;
        *x += current_option[0]<<8;
        ++current_option;
        *x += current_option[0];
        ++current_option;
      }
    }
    while (x!=(unsigned int *)&option_length && (x=(unsigned int *)&option_length));

    option_number += option_delta;

    PRINTF("OPTION %u (delta %u, len %u): ", option_number, option_delta, option_length);

    SET_OPTION(coap_pkt, option_number);

    switch ( option_number )
    {
        case COAP_OPTION_IF_MATCH:
        case COAP_OPTION_URI_HOST:
        case COAP_OPTION_ETAG:
        case COAP_OPTION_IF_NONE_MATCH:
        case COAP_OPTION_URI_PORT:
        case COAP_OPTION_MAX_AGE:
        case COAP_OPTION_LOCATION_QUERY:
        case COAP_OPTION_PROXY_URI:
        case COAP_OPTION_SIZE:
        case COAP_OPTION_TOKEN:
            /* nothing */
        break;

        case COAP_OPTION_CONTENT_TYPE:
            coap_pkt->content_type = coap_parse_int_option( current_option, option_length );
            PRINTF( "Content-Format [%u]\n", coap_pkt->content_type );
        break;

        case COAP_OPTION_ACCEPT:
            if ( coap_pkt->accept_num < COAP_MAX_ACCEPT_NUM )
            {
                coap_pkt->accept[coap_pkt->accept_num] = coap_parse_int_option( current_option, option_length );
                coap_pkt->accept_num += 1;
                PRINTF( "Accept [%u]\n", coap_pkt->content_type );
                PRINTF( "Accept [%u]\n", coap_pkt->accept[coap_pkt->accept_num-1] );
            }
        break;

        case COAP_OPTION_URI_PATH:
            /* coap_merge_multi_option() operates in-place on the IPBUF, but final packet field should be const string -> cast to string */
            /* coap_merge_multi_option( (char **) &(coap_pkt->uri_path), &(coap_pkt->uri_path_len), current_option, option_length, 0); */
            coap_add_multi_option( &(coap_pkt->uri_path), current_option, option_length, 1 );
            PRINTF( "Uri-Path [%.*s]\n", option_length, current_option );
        break;

        case COAP_OPTION_URI_QUERY:
            /* coap_merge_multi_option() operates in-place on the IPBUF, but final packet field should be const string -> cast to string */
            /* coap_merge_multi_option( (char **) &(coap_pkt->uri_query), &(coap_pkt->uri_query_len), current_option, option_length, '&'); */
            coap_add_multi_option( &(coap_pkt->uri_query), current_option, option_length, 1 );
            PRINTF( "Uri-Query [%.*s]\n", option_length, current_option );
        break;

        case COAP_OPTION_LOCATION_PATH:
            coap_add_multi_option( &(coap_pkt->location_path), current_option, option_length, 1 );
        break;

        case COAP_OPTION_OBSERVE:
            coap_pkt->observe = coap_parse_int_option( current_option, option_length );
            PRINTF( "Observe [%lu]\n", coap_pkt->observe );
        break;

        case COAP_OPTION_BLOCK2:
            coap_pkt->block2_num = coap_parse_int_option( current_option, option_length );
            coap_pkt->block2_more = (coap_pkt->block2_num & 0x08) >> 3;
            coap_pkt->block2_size = 16 << (coap_pkt->block2_num & 0x07);
            coap_pkt->block2_offset = (coap_pkt->block2_num & ~0x0000000F) << (coap_pkt->block2_num & 0x07);
            coap_pkt->block2_num >>= 4;
            PRINTF( "Block2 [%lu%s (%u B/blk)]\n", coap_pkt->block2_num, coap_pkt->block2_more ? "+" : "", coap_pkt->block2_size );
        break;

        case COAP_OPTION_BLOCK1:
            coap_pkt->block1_num = coap_parse_int_option( current_option, option_length );
            coap_pkt->block1_more = (coap_pkt->block1_num & 0x08) >> 3;
            coap_pkt->block1_size = 16 << (coap_pkt->block1_num & 0x07);
            coap_pkt->block1_offset = (coap_pkt->block1_num & ~0x0000000F) << (coap_pkt->block1_num & 0x07);
            coap_pkt->block1_num >>= 4;
            PRINTF( "Block1 [%lu%s (%u B/blk)]\n", coap_pkt->block1_num, coap_pkt->block1_more ? "+" : "", coap_pkt->block1_size );
        break;

        default:
            PRINTF( "unknown (%u)\n", option_number );
            /* Check if critical (odd) */
            if ( option_number & 1 )
            {
                coap_error_message = "Unsupported critical option";
                return BAD_OPTION_4_02;
            }
        break;
    }

    current_option += option_length;
  } /* for */
  PRINTF( "-Done parsing-------\n" );

  return NO_ERROR;
}

void coap_free_header( void *packet )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    free_multi_option( coap_pkt->uri_path );
    free_multi_option( coap_pkt->uri_query );
    free_multi_option( coap_pkt->location_path );
    coap_pkt->uri_path = NULL;
    coap_pkt->uri_query = NULL;
    coap_pkt->location_path = NULL;
}

int coap_set_status_code( void        *packet,
                          unsigned int code )
{
    if ( code <= 0xFF )
    {
        ((coap_packet_t *)packet)->code = (uint8_t)code;
        return 1;
    }
    else
    {
        return 0;
    }
}

int coap_set_header_content_type( void        *packet,
                                  unsigned int content_type )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    coap_pkt->content_type = (coap_content_type_t)content_type;
    SET_OPTION( coap_pkt, COAP_OPTION_CONTENT_TYPE );
    return 1;
}

int coap_get_header_token( void           *packet,
                           const uint8_t **token )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    if ( !IS_OPTION( coap_pkt, COAP_OPTION_TOKEN ) ) return 0;

    *token = coap_pkt->token;
    return coap_pkt->token_len;
}

int coap_set_header_token( void          *packet,
                           const uint8_t *token,
                           size_t         token_len )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    coap_pkt->token_len = (uint8_t)(MIN( COAP_TOKEN_LEN, token_len ));
    nbiot_memmove( coap_pkt->token, token, coap_pkt->token_len );

    SET_OPTION( coap_pkt, COAP_OPTION_TOKEN );
    return coap_pkt->token_len;
}

int coap_set_header_uri_path( void       *packet,
                              const char *path )
{
    coap_packet_t *coap_pkt = (coap_packet_t *)packet;
    int length = 0;

    free_multi_option( coap_pkt->uri_path );
    coap_pkt->uri_path = NULL;

    if ( path[0] == '/' ) ++path;

    do
    {
        int i = 0;

        while ( path[i] != 0 && path[i] != '/' ) i++;
        coap_add_multi_option( &(coap_pkt->uri_path), (uint8_t *)path, i, 0 );

        if ( path[i] == '/' ) i++;
        path += i;
        length += i;
    }
    while ( path[0] != 0 );

    SET_OPTION( coap_pkt, COAP_OPTION_URI_PATH );
    return length;
}

int coap_set_header_uri_path_segment( void       *packet,
                                      const char *segment )
{
    coap_packet_t *coap_pkt = (coap_packet_t *)packet;
    int length;

    if ( segment == NULL || segment[0] == 0 )
    {
        coap_add_multi_option( &(coap_pkt->uri_path), NULL, 0, 1 );
        length = 0;
    }
    else
    {
        length = nbiot_strlen( segment );
        coap_add_multi_option( &(coap_pkt->uri_path), (uint8_t *)segment, length, 0 );
    }

    SET_OPTION( coap_pkt, COAP_OPTION_URI_PATH );
    return length;
}

int coap_set_header_uri_query( void       *packet,
                               const char *query )
{
    int length = 0;
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    free_multi_option( coap_pkt->uri_query );
    coap_pkt->uri_query = NULL;

    if ( query[0] == '?' ) ++query;

    do
    {
        int i = 0;

        while ( query[i] != 0 && query[i] != '&' ) i++;
        coap_add_multi_option( &(coap_pkt->uri_query), (uint8_t *)query, i, 0 );

        if ( query[i] == '&' ) i++;
        query += i;
        length += i;
    }
    while ( query[0] != 0 );

    SET_OPTION( coap_pkt, COAP_OPTION_URI_QUERY );
    return length;
}

int coap_set_header_location_path( void       *packet,
                                   const char *path )
{
    coap_packet_t *coap_pkt = (coap_packet_t *)packet;
    int length = 0;

    free_multi_option( coap_pkt->location_path );
    coap_pkt->location_path = NULL;

    if ( path[0] == '/' ) ++path;

    do
    {
        int i = 0;

        while ( path[i] != 0 && path[i] != '/' ) i++;
        coap_add_multi_option( &(coap_pkt->location_path), (uint8_t *)path, i, 0 );

        if ( path[i] == '/' ) i++;
        path += i;
        length += i;
    }
    while ( path[0] != 0 );

    SET_OPTION( coap_pkt, COAP_OPTION_LOCATION_PATH );
    return length;
}

int coap_get_header_observe( void     *packet,
                             uint32_t *observe )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    if ( !IS_OPTION( coap_pkt, COAP_OPTION_OBSERVE ) ) return 0;

    *observe = coap_pkt->observe;
    return 1;
}

int coap_set_header_observe( void    *packet,
                             uint32_t observe )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    coap_pkt->observe = 0x00FFFFFF & observe;
    SET_OPTION( coap_pkt, COAP_OPTION_OBSERVE );
    return 1;
}

int coap_get_header_block2( void     *packet,
                            uint32_t *num,
                            uint8_t  *more,
                            uint16_t *size,
                            uint32_t *offset )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    if ( !IS_OPTION( coap_pkt, COAP_OPTION_BLOCK2 ) ) return 0;

    /* pointers may be NULL to get only specific block parameters */
    if ( num != NULL ) *num = coap_pkt->block2_num;
    if ( more != NULL ) *more = coap_pkt->block2_more;
    if ( size != NULL ) *size = coap_pkt->block2_size;
    if ( offset != NULL ) *offset = coap_pkt->block2_offset;

    return 1;
}

int coap_set_header_block2( void    *packet,
                            uint32_t num,
                            uint8_t  more,
                            uint16_t size )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    if ( size<16 ) return 0;
    if ( size>2048 ) return 0;
    if ( num > 0x0FFFFF ) return 0;

    coap_pkt->block2_num = num;
    coap_pkt->block2_more = more ? 1 : 0;
    coap_pkt->block2_size = size;

    SET_OPTION( coap_pkt, COAP_OPTION_BLOCK2 );
    return 1;
}

int coap_get_header_block1( void     *packet,
                            uint32_t *num,
                            uint8_t  *more,
                            uint16_t *size,
                            uint32_t *offset )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    if ( !IS_OPTION( coap_pkt, COAP_OPTION_BLOCK1 ) ) return 0;

    /* pointers may be NULL to get only specific block parameters */
    if ( num != NULL ) *num = coap_pkt->block1_num;
    if ( more != NULL ) *more = coap_pkt->block1_more;
    if ( size != NULL ) *size = coap_pkt->block1_size;
    if ( offset != NULL ) *offset = coap_pkt->block1_offset;

    return 1;
}

int coap_set_header_block1( void    *packet,
                            uint32_t num,
                            uint8_t  more,
                            uint16_t size )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    if ( size<16 ) return 0;
    if ( size>2048 ) return 0;
    if ( num > 0x0FFFFF ) return 0;

    coap_pkt->block1_num = num;
    coap_pkt->block1_more = more;
    coap_pkt->block1_size = size;

    SET_OPTION( coap_pkt, COAP_OPTION_BLOCK1 );
    return 1;
}

int coap_set_payload( void       *packet,
                      const void *payload,
                      size_t      length )
{
    coap_packet_t *const coap_pkt = (coap_packet_t *)packet;

    coap_pkt->payload = (uint8_t *)payload;
    coap_pkt->payload_len = (uint16_t)(length);

    return coap_pkt->payload_len;
}
