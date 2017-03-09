/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#ifndef NBIOT_SOURCE_LWM2M_COAP_H_
#define NBIOT_SOURCE_LWM2M_COAP_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The maximum buffer size that is provided for resource responses and must be respected due to the limited IP buffer.
 * Larger data must be handled by the resource and will be sent chunk-wise through a TCP stream or CoAP blocks.
*/
#ifndef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE                     128
#endif /* REST_MAX_CHUNK_SIZE */

#define COAP_RESPONSE_TIMEOUT                   2
#define COAP_MAX_RETRANSMIT                     4
#define COAP_ACK_RANDOM_FACTOR                  1.5
#define COAP_MAX_TRANSMIT_WAIT                  ((COAP_RESPONSE_TIMEOUT*((1 <<(COAP_MAX_RETRANSMIT+1))-1)*COAP_ACK_RANDOM_FACTOR))
#define COAP_HEADER_LEN                         4 /* | version:0x03 type:0x0C tkl:0xF0 | code | mid:0x00FF | mid:0xFF00 | */
#define COAP_ETAG_LEN                           8 /* The maximum number of bytes for the ETag */
#define COAP_TOKEN_LEN                          8 /* The maximum number of bytes for the Token */
#define COAP_MAX_ACCEPT_NUM                     2 /* The maximum number of accept preferences to parse/store */
#define COAP_MAX_OPTION_HEADER_LEN              5

#define COAP_HEADER_VERSION_MASK                0xC0
#define COAP_HEADER_VERSION_POSITION            6
#define COAP_HEADER_TYPE_MASK                   0x30
#define COAP_HEADER_TYPE_POSITION               4
#define COAP_HEADER_TOKEN_LEN_MASK              0x0F
#define COAP_HEADER_TOKEN_LEN_POSITION          0
#define COAP_HEADER_OPTION_DELTA_MASK           0xF0
#define COAP_HEADER_OPTION_SHORT_LENGTH_MASK    0x0F

/*
 * Conservative size limit, as not all options have to be set at the same time.
*/
#ifndef COAP_MAX_HEADER_SIZE
/*                                              Hdr CoT Age Tag              Obs Tok               Blo strings */
#define COAP_MAX_HEADER_SIZE                    (4 + 3 + 5 + 1+COAP_ETAG_LEN+ 3 + 1+COAP_TOKEN_LEN+ 4 + 30) /* 70 */
#endif /* COAP_MAX_HEADER_SIZE */
#define COAP_MAX_PACKET_SIZE                    (COAP_MAX_HEADER_SIZE + REST_MAX_CHUNK_SIZE)

/* Bitmap for set options */
enum
{
    OPTION_MAP_SIZE = sizeof(uint8_t)*8
};
#define SET_OPTION(packet,opt)                  ((packet)->options[opt/OPTION_MAP_SIZE]|=1<<(opt%OPTION_MAP_SIZE))
#define IS_OPTION(packet,opt)                   ((packet)->options[opt/OPTION_MAP_SIZE]&(1<<(opt%OPTION_MAP_SIZE)))

#ifndef MIN
#define MIN(a,b)                                ((a)<(b)?(a):(b))
#endif /* MIN */

/* CoAP message types */
typedef enum
{
    COAP_TYPE_CON, /* confirmables */
    COAP_TYPE_NON, /* non-confirmables */
    COAP_TYPE_ACK, /* acknowledgements */
    COAP_TYPE_RST  /* reset */
} coap_message_type_t;

/* CoAP request method codes */
typedef enum
{
    COAP_GET = 1,
    COAP_POST,
    COAP_PUT,
    COAP_DELETE
} coap_method_t;

/* CoAP response codes */
typedef enum
{
    NO_ERROR                      = 0,

    CREATED_2_01                  = 65,  /* CREATED */
    DELETED_2_02                  = 66,  /* DELETED */
    VALID_2_03                    = 67,  /* NOT_MODIFIED */
    CHANGED_2_04                  = 68,  /* CHANGED */
    CONTENT_2_05                  = 69,  /* OK */
                                         
    BAD_REQUEST_4_00              = 128, /* BAD_REQUEST */
    UNAUTHORIZED_4_01             = 129, /* UNAUTHORIZED */
    BAD_OPTION_4_02               = 130, /* BAD_OPTION */
    FORBIDDEN_4_03                = 131, /* FORBIDDEN */
    NOT_FOUND_4_04                = 132, /* NOT_FOUND */
    METHOD_NOT_ALLOWED_4_05       = 133, /* METHOD_NOT_ALLOWED */
    NOT_ACCEPTABLE_4_06           = 134, /* NOT_ACCEPTABLE */
    PRECONDITION_FAILED_4_12      = 140, /* BAD_REQUEST */
    REQUEST_ENTITY_TOO_LARGE_4_13 = 141, /* REQUEST_ENTITY_TOO_LARGE */
    UNSUPPORTED_MEDIA_TYPE_4_15   = 143, /* UNSUPPORTED_MEDIA_TYPE */
                                         
    INTERNAL_SERVER_ERROR_5_00    = 160, /* INTERNAL_SERVER_ERROR */
    NOT_IMPLEMENTED_5_01          = 161, /* NOT_IMPLEMENTED */
    BAD_GATEWAY_5_02              = 162, /* BAD_GATEWAY */
    SERVICE_UNAVAILABLE_5_03      = 163, /* SERVICE_UNAVAILABLE */
    GATEWAY_TIMEOUT_5_04          = 164, /* GATEWAY_TIMEOUT */
    PROXYING_NOT_SUPPORTED_5_05   = 165, /* PROXYING_NOT_SUPPORTED */

    /* Erbium errors */
    MEMORY_ALLOCATION_ERROR         = 192,
    PACKET_SERIALIZATION_ERROR,

    /* Erbium hooks */
    MANUAL_RESPONSE
} coap_status_t;

/* CoAP header options */
typedef enum
{
    COAP_OPTION_OBSERVE        = 6,  /* 0-3 B */
    COAP_OPTION_LOCATION_PATH  = 8,  /* 0-255 B */
    COAP_OPTION_URI_PATH       = 11, /* 0-255 B */
    COAP_OPTION_CONTENT_TYPE   = 12, /* 0-2 B */
    COAP_OPTION_URI_QUERY      = 15, /* 0-270 B */
    COAP_OPTION_ACCEPT         = 17, /* 0-2 B */
    COAP_OPTION_TOKEN          = 19, /* 1-8 B */
    COAP_OPTION_BLOCK2         = 23, /* 1-3 B */
    COAP_OPTION_BLOCK1         = 27, /* 1-3 B */
    COAP_OPTION_SIZE           = 28, /* 0-4 B */
    COAP_OPTION_AUTH_CODE      = 63  /* 1-270 B OneNET defined option, use for authentication */
} coap_option_t;

/* CoAP Content-Types */
typedef enum
{
    TEXT_PLAIN                   = 0,
    TEXT_XML                     = 1, /* Indented types are not in the initial registry. */
    TEXT_CSV                     = 2,
    TEXT_HTML                    = 3,
    IMAGE_GIF                    = 21,
    IMAGE_JPEG                   = 22,
    IMAGE_PNG                    = 23,
    IMAGE_TIFF                   = 24,
    AUDIO_RAW                    = 25,
    VIDEO_RAW                    = 26,
    APPLICATION_LINK_FORMAT      = 40,
    APPLICATION_XML              = 41,
    APPLICATION_OCTET_STREAM     = 42,
    APPLICATION_RDF_XML          = 43,
    APPLICATION_SOAP_XML         = 44,
    APPLICATION_ATOM_XML         = 45,
    APPLICATION_XMPP_XML         = 46,
    APPLICATION_EXI              = 47,
    APPLICATION_FASTINFOSET      = 48,
    APPLICATION_SOAP_FASTINFOSET = 49,
    APPLICATION_JSON             = 50,
    APPLICATION_X_OBIX_BINARY    = 51
} coap_content_type_t;

typedef struct _multi_option_t
{
    struct _multi_option_t *next;
    uint8_t                 is_static;
    uint8_t                 len;
    uint8_t                *data;
} multi_option_t;

/* Parsed message struct */
typedef struct
{
    uint8_t             *buffer; /* pointer to CoAP header / incoming packet buffer / memory to serialize packet */

    uint8_t             version;
    coap_message_type_t type;
    uint8_t             code;
    uint16_t            mid;

    uint8_t             options[COAP_OPTION_AUTH_CODE / OPTION_MAP_SIZE + 1]; /* Bitmap to check if option is set */

    coap_content_type_t content_type; /* Parse options once and store; allows setting options in random order  */
    size_t              auth_code_len;
    const uint8_t      *auth_code;
    multi_option_t     *location_path;
    multi_option_t     *uri_path;
    uint32_t            observe;
    uint8_t             token_len;
    uint8_t             token[COAP_TOKEN_LEN];
    uint8_t             accept_num;
    uint16_t            accept[COAP_MAX_ACCEPT_NUM];
    uint32_t            block2_num;
    uint8_t             block2_more;
    uint16_t            block2_size;
    uint32_t            block2_offset;
    uint32_t            block1_num;
    uint8_t             block1_more;
    uint16_t            block1_size;
    uint32_t            block1_offset;
    uint32_t            size;
    multi_option_t     *uri_query;
    uint16_t            payload_len;
    uint8_t            *payload;
} coap_packet_t;

/* To store error code and human-readable payload */
extern const char *coap_error_message;

/* Functions */
char* coap_get_multi_option_as_string( multi_option_t * option );

void coap_init_message( void               *packet,
                        coap_message_type_t type,
                        uint8_t             code,
                        uint16_t            mid );

size_t coap_serialize_get_size( void *packet );

size_t coap_serialize_message( void    *packet,
                               uint8_t *buffer );

coap_status_t coap_parse_message( void    *request,
                                  uint8_t *data,
                                  uint16_t data_len );

void coap_free_header( void *packet );

int coap_set_status_code( void        *packet,
                          unsigned int code );

int coap_set_header_content_type( void        *packet,
                                  unsigned int content_type );

int coap_get_header_token( void           *packet,
                           const uint8_t **token );

int coap_set_header_token( void          *packet,
                           const uint8_t *token,
                           size_t         token_len );

int coap_set_header_auth_code( void       *packet,
                               const char *code );

int coap_set_header_uri_path( void       *packet,
                              const char *path );

int coap_set_header_uri_path_segment( void       *packet,
                                      const char *segment );

int coap_set_header_uri_query( void       *packet,
                               const char *query );

int coap_set_header_location_path( void       *packet,
                                   const char *path ); /* Also splits optional query into Location-Query option. */

int coap_get_header_observe( void     *packet,
                             uint32_t *observe );

int coap_set_header_observe( void    *packet,
                             uint32_t observe );

int coap_get_header_block2( void     *packet,
                            uint32_t *num,
                            uint8_t  *more,
                            uint16_t *size,
                            uint32_t *offset );

int coap_set_header_block2( void    *packet,
                            uint32_t num,
                            uint8_t  more,
                            uint16_t size );

int coap_get_header_block1( void     *packet,
                            uint32_t *num,
                            uint8_t  *more,
                            uint16_t *size,
                            uint32_t *offset );

int coap_set_header_block1( void    *packet,
                            uint32_t num,
                            uint8_t  more,
                            uint16_t size );

int coap_set_payload( void       *packet,
                      const void *payload,
                      size_t      length );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_LWM2M_COAP_H_*/
