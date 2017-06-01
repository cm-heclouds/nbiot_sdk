/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef ONENET_COAP_H_
#define ONENET_COAP_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The maximum buffer size that is provided for resource responses */
#define COAP_CHUNK_SIZE                   128
#define COAP_HEADER_SIZE                  4 /* | version:0x30 type:0xc0 tkl:0x0f | code | mid:0xff00 | mid:0x00ff | */
#define COAP_TOKEN_SIZE                   8 /* The maximum number of bytes for the Token */
#define COAP_VERSION                      1 /* always be 1 */
#define COAP_VERSION_MASK                 0xc0
#define COAP_VERSION_POSITION             6
#define COAP_TYPE_MASK                    0x30
#define COAP_TYPE_POSITION                4
#define COAP_TOKEN_LEN_MASK               0x0f
#define COAP_TOKEN_LEN_POSITION           0
#define COAP_OPTION_DELTA_MASK            0xf0
#define COAP_OPTION_SHORT_LENGTH_MASK     0x0f

#define COAP_RESPONSE_TIMEOUT             2
#define COAP_MAX_RETRANSMIT               4
#define COAP_ACK_RANDOM_FACTOR            1.5
#define COAP_MAX_TRANSMIT_WAIT            ((COAP_RESPONSE_TIMEOUT*((1 <<(COAP_MAX_RETRANSMIT+1))-1)*COAP_ACK_RANDOM_FACTOR))

/* CoAP Message Types*/
#define COAP_TYPE_CON                     0 /* confirmables */
#define COAP_TYPE_NON                     1 /* non-confirmables */
#define COAP_TYPE_ACK                     2 /* acknowledgements */
#define COAP_TYPE_RST                     3 /* reset */

/* CoAP Request Method Codes */
#define COAP_GET                          1
#define COAP_POST                         2
#define COAP_PUT                          3
#define COAP_DELETE                       4

/* CoAP Response Codes */
#define COAP_NO_ERROR                     0x00
#define COAP_IGNORE                       0x01
#define COAP_CREATED_201                  0x41 /* CREATED */
#define COAP_DELETED_202                  0x42 /* DELETED */
#define COAP_VALID_203                    0x43 /* NOT_MODIFIED */
#define COAP_CHANGED_204                  0x44 /* CHANGED */
#define COAP_CONTENT_205                  0x45 /* OK */
#define COAP_BAD_REQUEST_400              0x80 /* BAD_REQUEST */
#define COAP_UNAUTHORIZED_401             0x81 /* UNAUTHORIZED */
#define COAP_BAD_OPTION_402               0x82 /* BAD_OPTION */
#define COAP_FORBIDDEN_403                0x83 /* FORBIDDEN */
#define COAP_NOT_FOUND_404                0x84 /* NOT_FOUND */
#define COAP_METHOD_NOT_ALLOWED_405       0x85 /* METHOD_NOT_ALLOWED */
#define COAP_NOT_ACCEPTABLE_406           0x86 /* NOT_ACCEPTABLE */
#define COAP_PRECONDITION_FAILED_412      0x8c /* BAD_REQUEST */
#define COAP_REQUEST_ENTITY_TOO_LARGE_413 0x8d /* REQUEST_ENTITY_TOO_LARGE */
#define COAP_UNSUPPORTED_MEDIA_TYPE_415   0x8f /* UNSUPPORTED_MEDIA_TYPE */
#define COAP_INTERNAL_SERVER_ERROR_500    0xa0 /* INTERNAL_SERVER_ERROR */
#define COAP_NOT_IMPLEMENTED_501          0xa1 /* NOT_IMPLEMENTED */
#define COAP_BAD_GATEWAY_502              0xa2 /* BAD_GATEWAY */
#define COAP_SERVICE_UNAVAILABLE_503      0xa3 /* SERVICE_UNAVAILABLE */
#define COAP_GATEWAY_TIMEOUT_504          0xa4 /* GATEWAY_TIMEOUT */
#define COAP_PROXYING_NOT_SUPPORTED_505   0xa5 /* PROXYING_NOT_SUPPORTED */

/* CoAP Header Options */
#define COAP_OPTION_OBSERVE               6  /* uint,   0-3 B */
#define COAP_OPTION_LOCATION_PATH         8  /* string, 0-255 B */
#define COAP_OPTION_URI_PATH              11 /* string, 0-255 B */
#define COAP_OPTION_CONTENT_TYPE          12 /* uint,   0-2 B */
#define COAP_OPTION_URI_QUERY             15 /* string, 0-255 B */
#define COAP_OPTION_ACCEPT                17 /* uint,   0-2 B */
#define COAP_OPTION_TOKEN                 19 /* opaque, 1-8 B */
#define COAP_OPTION_BLOCK2                23 /* uint,   1-3 B */
#define COAP_OPTION_BLOCK1                27 /* uint,   1-3 B */
#define COAP_OPTION_SIZE                  28 /* uint,   0-4 B */

typedef struct _coap_t
{
    uint8_t *buffer;
    uint16_t size;
    uint16_t offset;
    uint16_t option;
} coap_t;

/**
 * 初始化coap协议包头（包含token）
 * 其中，token可以为NULL，表示后续填充数据。
**/
int coap_init_header( coap_t     *coap,
                      uint8_t     type,
                      uint8_t     code,
                      uint16_t    mid,
                      const void *token,
                      uint8_t     token_len );

/**
 * 添加coap协议option值
 * 添加时，必须按option大小的顺序填充。
 * 其中，value可以为NULL，表示后续填充数据。
**/
int coap_add_option( coap_t     *coap,
                     uint16_t    option,
                     const void *value,
                     uint16_t    length );

/**
 * 获取coap协议option值
**/
int coap_option( const uint8_t  *buffer,
                 uint16_t        buffer_len,
                 uint16_t        option,
                 const uint8_t **value,
                 uint16_t       *value_len,
                 bool            first );

#ifdef NBIOT_BOOTSTRAP
/**
* 获取coap协议payload
**/
int coap_payload( const uint8_t  *buffer,
                  uint16_t        buffer_len,
                  const uint8_t **payload,
                  uint16_t       *payload_len );
#endif

static inline
void coap_set_type( coap_t *coap, uint8_t type )
{
    coap->buffer[0] &= ~COAP_TYPE_MASK;
    coap->buffer[0] |= COAP_TYPE_MASK&(type<<COAP_TYPE_POSITION);
}

static inline
void coap_set_code( coap_t *coap, uint8_t code )
{
    coap->buffer[1] = code;
}

static inline
void coap_set_mid( coap_t *coap, uint16_t mid )
{
    coap->buffer[2] = (uint8_t)(mid >> 8);
    coap->buffer[3] = (uint8_t)(mid & 0xff);
}

static inline
uint8_t coap_type( const uint8_t *buffer )
{
    return (buffer[0]&COAP_TYPE_MASK)>>COAP_TYPE_POSITION;
}

static inline
uint16_t coap_code( const uint8_t *buffer )
{
    return buffer[1];
}

static inline
uint16_t coap_mid( const uint8_t *buffer )
{
    return (((uint16_t)buffer[2])<<8) | buffer[3];
}

static inline
uint8_t coap_token( const uint8_t *buffer, const uint8_t **token )
{
    if ( token )
    {
        *token = buffer + COAP_HEADER_SIZE;
    }

    return (buffer[0]&COAP_TOKEN_LEN_MASK)>>COAP_TOKEN_LEN_POSITION;
}

int coap_add_int_option( coap_t  *coap,
                         uint16_t option,
                         uint32_t value );
int coap_int_option( const uint8_t *buffer,
                     uint16_t       buffer_len,
                     uint16_t       option,
                     uint32_t      *value );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* ONENET_COAP_H_ */
