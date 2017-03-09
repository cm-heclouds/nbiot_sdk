/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#ifndef NBIOT_SOURCE_LWM2M_INTERNALS_H_
#define NBIOT_SOURCE_LWM2M_INTERNALS_H_

#include "coap.h"
#include "lwm2m.h"
#include <utils.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LWM2M_WITH_LOGS
#ifdef __FUNCTION__
#define __func__ __FUNCTION__
#endif

#define LOG(STR)          nbiot_printf("[%s:%d] " STR "\r\n", __func__, __LINE__)
#define LOG_ARG(FMT, ...) nbiot_printf("[%s:%d] " FMT "\r\n", __func__, __LINE__, __VA_ARGS__)
#define LOG_URI(URI) \
{ \
    if ( (URI) == NULL ) \
        nbiot_printf( "[%s:%d] NULL\r\n", __func__, __LINE__ ); \
    else \
    { \
        nbiot_printf( "[%s:%d] /%d", __func__, __LINE__, (URI)->objectId ); \
        if ( LWM2M_URI_IS_SET_INSTANCE( URI ) ) \
            nbiot_printf( "/%d", (URI)->instanceId ); \
        if ( LWM2M_URI_IS_SET_RESOURCE( URI ) ) \
            nbiot_printf( "/%d", (URI)->resourceId ); \
        nbiot_printf( "\r\n" ); \
    } \
}

#define STR_STATUS(S) \
    ((S) == STATE_DEREGISTERED ? "STATE_DEREGISTERED" : \
    ((S) == STATE_REG_PENDING ? "STATE_REG_PENDING" : \
    ((S) == STATE_REGISTERED ? "STATE_REGISTERED" : \
    ((S) == STATE_REG_FAILED ? "STATE_REG_FAILED" : \
    ((S) == STATE_REG_UPDATE_PENDING ? "STATE_REG_UPDATE_PENDING" : \
    ((S) == STATE_REG_UPDATE_NEEDED ? "STATE_REG_UPDATE_NEEDED" : \
    ((S) == STATE_DEREG_PENDING ? "STATE_DEREG_PENDING" : \
    ((S) == STATE_BS_HOLD_OFF ? "STATE_BS_HOLD_OFF" : \
    ((S) == STATE_BS_INITIATED ? "STATE_BS_INITIATED" : \
    ((S) == STATE_BS_PENDING ? "STATE_BS_PENDING" : \
    ((S) == STATE_BS_FINISHED ? "STATE_BS_FINISHED" : \
    ((S) == STATE_BS_FAILED ? "STATE_BS_FAILED" : \
    "Unknown"))))))))))))

#define STR_MEDIA_TYPE(M) \
    ((M) == LWM2M_CONTENT_TEXT ? "LWM2M_CONTENT_TEXT" : \
    ((M) == LWM2M_CONTENT_LINK ? "LWM2M_CONTENT_LINK" : \
    ((M) == LWM2M_CONTENT_OPAQUE ? "LWM2M_CONTENT_OPAQUE" : \
    ((M) == LWM2M_CONTENT_TLV ? "LWM2M_CONTENT_TLV" : \
    ((M) == LWM2M_CONTENT_JSON ? "LWM2M_CONTENT_JSON" : \
    "Unknown")))))

#define STR_STATE(S)    \
    ((S) == STATE_INITIAL ? "STATE_INITIAL" : \
    ((S) == STATE_BOOTSTRAP_REQUIRED ? "STATE_BOOTSTRAP_REQUIRED" : \
    ((S) == STATE_BOOTSTRAPPING ? "STATE_BOOTSTRAPPING" : \
    ((S) == STATE_REGISTER_REQUIRED ? "STATE_REGISTER_REQUIRED" : \
    ((S) == STATE_REGISTERING ? "STATE_REGISTERING" : \
    ((S) == STATE_READY ? "STATE_READY" : \
    ((S) == STATE_RESET ? "STATE_RESET" : \
    "Unknown")))))))
#else
#define LOG(STR)
#define LOG_ARG(FMT, ...)
#define LOG_URI(URI)
#endif

#define LWM2M_DEFAULT_LIFETIME       86400

#define REG_LWM2M_RESOURCE_TYPE      ">;rt=\"oma.lwm2m\","
#define REG_LWM2M_RESOURCE_TYPE_LEN  17

#define REG_START                    "<"
#define REG_DEFAULT_PATH             "/"

#define REG_OBJECT_MIN_LEN           5       /* "</n>," */
#define REG_PATH_END                 ">,"
#define REG_PATH_SEPARATOR           "/"

#define REG_OBJECT_PATH              "<%s/%hu>,"
#define REG_OBJECT_INSTANCE_PATH     "<%s/%hu/%hu>,"

#define URI_REGISTRATION_SEGMENT     "rd"
#define URI_REGISTRATION_SEGMENT_LEN 2
#define URI_BOOTSTRAP_SEGMENT        "bs"
#define URI_BOOTSTRAP_SEGMENT_LEN    2

#define QUERY_TEMPLATE               "ep="
#define QUERY_LENGTH                 3       /* strlen("ep=") */
#define QUERY_SMS                    "sms="
#define QUERY_SMS_LEN                4
#define QUERY_LIFETIME               "lt="
#define QUERY_LIFETIME_LEN           3
#define QUERY_VERSION                "lwm2m="
#define QUERY_VERSION_LEN            6
#define QUERY_BINDING                "b="
#define QUERY_BINDING_LEN            2
#define QUERY_DELIMITER              "&"

#define QUERY_VERSION_FULL           "lwm2m=1.0"
#define QUERY_VERSION_FULL_LEN       9

#define REG_URI_START                '<'
#define REG_URI_END                  '>'
#define REG_DELIMITER                ','
#define REG_ATTR_SEPARATOR           ';'
#define REG_ATTR_EQUALS              '='
#define REG_ATTR_TYPE_KEY            "rt"
#define REG_ATTR_TYPE_KEY_LEN        2
#define REG_ATTR_TYPE_VALUE          "\"oma.lwm2m\""
#define REG_ATTR_TYPE_VALUE_LEN      11
#define REG_ATTR_CONTENT_KEY         "ct"
#define REG_ATTR_CONTENT_KEY_LEN     2
#define REG_ATTR_CONTENT_JSON        "11543" /* Temporary value */
#define REG_ATTR_CONTENT_JSON_LEN    4

#define ATTR_SERVER_ID_STR           "ep="
#define ATTR_SERVER_ID_LEN           3
#define ATTR_MIN_PERIOD_STR          "pmin="
#define ATTR_MIN_PERIOD_LEN          5
#define ATTR_MAX_PERIOD_STR          "pmax="
#define ATTR_MAX_PERIOD_LEN          5
#define ATTR_GREATER_THAN_STR        "gt="
#define ATTR_GREATER_THAN_LEN        3
#define ATTR_LESS_THAN_STR           "lt="
#define ATTR_LESS_THAN_LEN           3
#define ATTR_STEP_STR                "stp="
#define ATTR_STEP_LEN                4
#define ATTR_DIMENSION_STR           "dim="
#define ATTR_DIMENSION_LEN           4

#define URI_MAX_STRING_LEN           18      /* /65535/65535/65535 */
#define _PRV_64BIT_BUFFER_SIZE       8

#define LINK_ITEM_START              "<"
#define LINK_ITEM_START_SIZE         1
#define LINK_ITEM_END                ">,"
#define LINK_ITEM_END_SIZE           2
#define LINK_ITEM_DIM_START          ">;dim="
#define LINK_ITEM_DIM_START_SIZE     6
#define LINK_ITEM_ATTR_END           ","
#define LINK_ITEM_ATTR_END_SIZE      1
#define LINK_URI_SEPARATOR           "/"
#define LINK_URI_SEPARATOR_SIZE      1
#define LINK_ATTR_SEPARATOR          ";"
#define LINK_ATTR_SEPARATOR_SIZE     1

#define ATTR_FLAG_NUMERIC            (uint8_t)(LWM2M_ATTR_FLAG_LESS_THAN |\
                                               LWM2M_ATTR_FLAG_GREATER_THAN |\
                                               LWM2M_ATTR_FLAG_STEP)

#define LWM2M_URI_FLAG_DM            (uint8_t)0x00
#define LWM2M_URI_FLAG_DELETE_ALL    (uint8_t)0x10
#define LWM2M_URI_FLAG_REGISTRATION  (uint8_t)0x20
#define LWM2M_URI_FLAG_BOOTSTRAP     (uint8_t)0x40

#define LWM2M_URI_MASK_TYPE          (uint8_t)0x70
#define LWM2M_URI_MASK_ID            (uint8_t)0x07

typedef enum
{
    URI_DEPTH_OBJECT,
    URI_DEPTH_OBJECT_INSTANCE,
    URI_DEPTH_RESOURCE,
    URI_DEPTH_RESOURCE_INSTANCE
} uri_depth_t;

/*
 * defined in uri.c
*/
lwm2m_uri_t *uri_decode( char           *altPath,
                         multi_option_t *uriPath );
int uri_getNumber( uint8_t *uriString,
                   size_t   uriLength );
int uri_toString( lwm2m_uri_t *uriP,
                  uint8_t     *buffer,
                  size_t       bufferLen,
                  uri_depth_t *depthP );

/*
 * defined in objects.c
*/
coap_status_t object_readData( lwm2m_context_t *contextP,
                               lwm2m_uri_t     *uriP,
                               int             *sizeP,
                               lwm2m_data_t   **dataP );
coap_status_t object_read( lwm2m_context_t    *contextP,
                           lwm2m_uri_t        *uriP,
                           lwm2m_media_type_t *formatP,
                           uint8_t           **bufferP,
                           size_t             *lengthP );
coap_status_t object_write( lwm2m_context_t   *contextP,
                            lwm2m_uri_t       *uriP,
                            lwm2m_media_type_t format,
                            uint8_t           *buffer,
                            size_t             length );
coap_status_t object_execute( lwm2m_context_t *contextP,
                              lwm2m_uri_t     *uriP,
                              uint8_t         *buffer,
                              size_t           length );
coap_status_t object_create( lwm2m_context_t   *contextP,
                             lwm2m_uri_t       *uriP,
                             lwm2m_media_type_t format,
                             uint8_t           *buffer,
                             size_t             length );
coap_status_t object_delete( lwm2m_context_t *contextP,
                             lwm2m_uri_t     *uriP );
coap_status_t object_discover( lwm2m_context_t *contextP,
                               lwm2m_uri_t     *uriP,
                               uint8_t        **bufferP,
                               size_t          *lengthP );
uint8_t object_checkReadable( lwm2m_context_t *contextP,
                              lwm2m_uri_t     *uriP );
uint8_t object_checkNumeric( lwm2m_context_t *contextP,
                             lwm2m_uri_t     *uriP );
bool object_isInstanceNew( lwm2m_context_t *contextP,
                           uint16_t         objectId,
                           uint16_t         instanceId );
int object_getRegisterPayload( lwm2m_context_t *contextP,
                               uint8_t         *buffer,
                               size_t           length);
int object_getServers(lwm2m_context_t *contextP);
coap_status_t object_createInstance( lwm2m_context_t *contextP,
                                     lwm2m_uri_t     *uriP,
                                     lwm2m_data_t    *dataP );
coap_status_t object_writeInstance( lwm2m_context_t *contextP,
                                    lwm2m_uri_t     *uriP,
                                    lwm2m_data_t    *dataP );

/*
 * defined in transaction.c
*/
lwm2m_transaction_t* transaction_new( coap_message_type_t   type,
                                      coap_method_t         method,
                                      char                 *altPath,
                                      lwm2m_uri_t          *uriP,
                                      uint16_t              mID,
                                      uint8_t               token_len,
                                      uint8_t              *token,
                                      lwm2m_endpoint_type_t peerType,
                                      void                 *peerP );
int transaction_send( lwm2m_context_t     *contextP,
                      lwm2m_transaction_t *transacP );
void transaction_free( lwm2m_transaction_t *transacP );
void transaction_remove( lwm2m_context_t     *contextP,
                         lwm2m_transaction_t *transacP);
bool transaction_handleResponse( lwm2m_context_t *contextP,
                                 void            *fromSessionH,
                                 coap_packet_t   *message,
                                 coap_packet_t   *response );
void transaction_step( lwm2m_context_t *contextP,
                       time_t           currentTime,
                       time_t          *timeoutP );

/*
 * defined in management.c
*/
coap_status_t dm_handleRequest( lwm2m_context_t *contextP,
                                lwm2m_uri_t     *uriP,
                                lwm2m_server_t  *serverP,
                                coap_packet_t   *message,
                                coap_packet_t   *response );

/*
 * defined in observe.c
*/
coap_status_t observe_handleRequest( lwm2m_context_t *contextP,
                                     lwm2m_uri_t     *uriP,
                                     lwm2m_server_t  *serverP,
                                     int              size,
                                     lwm2m_data_t    *dataP,
                                     coap_packet_t   *message,
                                     coap_packet_t   *response );
void observe_cancel( lwm2m_context_t *contextP,
                     uint16_t         mid,
                     void            *fromSessionH );
coap_status_t observe_setParameters( lwm2m_context_t    *contextP,
                                     lwm2m_uri_t        *uriP,
                                     lwm2m_server_t     *serverP,
                                     lwm2m_attributes_t *attrP );
void observe_step( lwm2m_context_t *contextP,
                   time_t           currentTime,
                   time_t          *timeoutP );
lwm2m_observed_t* observe_findByUri( lwm2m_context_t *contextP,
                                     lwm2m_uri_t     *uriP );

/*
 * defined in registration.c
*/
void registration_deregister( lwm2m_context_t *contextP,
                              lwm2m_server_t  *serverP );
uint8_t registration_start( lwm2m_context_t *contextP );
void registration_step( lwm2m_context_t *contextP,
                        time_t           currentTime,
                        time_t          *timeoutP );
lwm2m_status_t registration_getStatus( lwm2m_context_t * contextP );

/*
 * defined in packet.c
*/
coap_status_t message_send( lwm2m_context_t *contextP,
                            coap_packet_t   *message,
                            void            *sessionH );

/*
 * defined in bootstrap.c
*/
void bootstrap_step( lwm2m_context_t *contextP,
                     uint32_t         currentTime,
                     time_t          *timeoutP );
coap_status_t bootstrap_handleCommand( lwm2m_context_t *contextP,
                                       lwm2m_uri_t     *uriP,
                                       lwm2m_server_t  *serverP,
                                       coap_packet_t   *message,
                                       coap_packet_t   *response );
coap_status_t bootstrap_handleDeleteAll( lwm2m_context_t *context,
                                         void            *fromSessionH );
coap_status_t bootstrap_handleFinish( lwm2m_context_t *context,
                                      void            *fromSessionH );
void bootstrap_start( lwm2m_context_t * contextP );
lwm2m_status_t bootstrap_getStatus( lwm2m_context_t * contextP );

/*
 * defined in tlv.c
*/
int tlv_parse( uint8_t       *buffer,
               size_t         bufferLen,
               lwm2m_data_t **dataP );
size_t tlv_serialize( bool          isResourceInstance,
                      int           size,
                      lwm2m_data_t *dataP,
                      uint8_t     **bufferP );

/*
* defined in discover.c
*/
int discover_serialize( lwm2m_context_t *contextP,
                        lwm2m_uri_t     *uriP,
                        int              size,
                        lwm2m_data_t    *dataP,
                        uint8_t        **bufferP );

/*
 * defined in block1.c
*/
coap_status_t coap_block1_handler( lwm2m_block1_data_t **block1Data,
                                   uint16_t              mid,
                                   uint8_t              *buffer,
                                   size_t                length,
                                   uint16_t              blockSize,
                                   uint32_t              blockNum,
                                   bool                  blockMore,
                                   uint8_t             **outputBuffer,
                                   size_t               *outputLength );
void free_block1_buffer( lwm2m_block1_data_t *block1Data );

/*
 * defined in utils.c
*/
lwm2m_media_type_t utils_convertMediaType( coap_content_type_t type );
int utils_stringCopy( char       *buffer,
                      size_t      length,
                      const char *str );
int utils_intCopy( char   *buffer,
                   size_t  length,
                   int32_t value );
size_t utils_intToText( int64_t  data,
                        uint8_t *string,
                        size_t   length );
size_t utils_floatToText( double   data,
                          uint8_t *string,
                          size_t   length );
int utils_plainTextToInt64( uint8_t *buffer,
                            int      length,
                            int64_t *dataP );
int utils_plainTextToFloat64( uint8_t *buffer,
                              int      length,
                              double  *dataP );
size_t utils_int64ToPlainText( int64_t   data,
                               uint8_t **bufferP );
size_t utils_float64ToPlainText( double    data,
                                 uint8_t **bufferP );
size_t utils_boolToPlainText( bool      data,
                              uint8_t **bufferP );
void utils_copyValue( void       *dst,
                      const void *src,
                      size_t      len );
int utils_opaqueToInt( const uint8_t *buffer,
                       size_t         buffer_len,
                       int64_t       *dataP );
int utils_opaqueToFloat( const uint8_t *buffer,
                         size_t         buffer_len,
                         double        *dataP );
size_t utils_encodeInt( int64_t data,
                        uint8_t data_buffer[_PRV_64BIT_BUFFER_SIZE] );
size_t utils_encodeFloat( double  data,
                          uint8_t data_buffer[_PRV_64BIT_BUFFER_SIZE] );
lwm2m_server_t *utils_findServer( lwm2m_context_t *contextP,
                                  void            *fromSessionH );
lwm2m_server_t *utils_findBootstrapServer( lwm2m_context_t *contextP,
                                           void            *fromSessionH );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_LWM2M_INTERNALS_H_*/
