/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#ifndef NBIOT_SOURCE_LWM2M_LWM2M_H_
#define NBIOT_SOURCE_LWM2M_LWM2M_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Error code
*/
#define COAP_NO_ERROR                       (uint8_t)0x00
#define COAP_IGNORE                         (uint8_t)0x01
#define COAP_201_CREATED                    (uint8_t)0x41
#define COAP_202_DELETED                    (uint8_t)0x42
#define COAP_204_CHANGED                    (uint8_t)0x44
#define COAP_205_CONTENT                    (uint8_t)0x45
#define COAP_231_CONTINUE                   (uint8_t)0x5F
#define COAP_400_BAD_REQUEST                (uint8_t)0x80
#define COAP_401_UNAUTHORIZED               (uint8_t)0x81
#define COAP_402_BAD_OPTION                 (uint8_t)0x82
#define COAP_404_NOT_FOUND                  (uint8_t)0x84
#define COAP_405_METHOD_NOT_ALLOWED         (uint8_t)0x85
#define COAP_406_NOT_ACCEPTABLE             (uint8_t)0x86
#define COAP_408_REQ_ENTITY_INCOMPLETE      (uint8_t)0x88
#define COAP_413_ENTITY_TOO_LARGE           (uint8_t)0x8F
#define COAP_500_INTERNAL_SERVER_ERROR      (uint8_t)0xA0
#define COAP_501_NOT_IMPLEMENTED            (uint8_t)0xA1
#define COAP_503_SERVICE_UNAVAILABLE        (uint8_t)0xA3

/*
 * Standard Object IDs
*/
#define LWM2M_SECURITY_OBJECT_ID            0 /* not supported */
#define LWM2M_SERVER_OBJECT_ID              1 /* not supported */
#define LWM2M_ACL_OBJECT_ID                 2 /* not supported */
#define LWM2M_DEVICE_OBJECT_ID              3 /* only supported */
#define LWM2M_CONN_MONITOR_OBJECT_ID        4 /* not supported */
#define LWM2M_FIRMWARE_UPDATE_OBJECT_ID     5 /* not supported */
#define LWM2M_LOCATION_OBJECT_ID            6 /* not supported */
#define LWM2M_CONN_STATS_OBJECT_ID          7 /* not supported */

/*
 * Ressource IDs for the LWM2M Security Object
*/
#define LWM2M_SECURITY_URI_ID               0
#define LWM2M_SECURITY_BOOTSTRAP_ID         1
#define LWM2M_SECURITY_SECURITY_ID          2
#define LWM2M_SECURITY_PUBLIC_KEY_ID        3
#define LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID 4
#define LWM2M_SECURITY_SECRET_KEY_ID        5
#define LWM2M_SECURITY_SMS_SECURITY_ID      6
#define LWM2M_SECURITY_SMS_KEY_PARAM_ID     7
#define LWM2M_SECURITY_SMS_SECRET_KEY_ID    8
#define LWM2M_SECURITY_SMS_SERVER_NUMBER_ID 9
#define LWM2M_SECURITY_SHORT_SERVER_ID      10
#define LWM2M_SECURITY_HOLD_OFF_ID          11

/*
 * Ressource IDs for the LWM2M Server Object
*/
#define LWM2M_SERVER_SHORT_ID_ID            0
#define LWM2M_SERVER_LIFETIME_ID            1
#define LWM2M_SERVER_MIN_PERIOD_ID          2
#define LWM2M_SERVER_MAX_PERIOD_ID          3
#define LWM2M_SERVER_DISABLE_ID             4
#define LWM2M_SERVER_TIMEOUT_ID             5
#define LWM2M_SERVER_STORING_ID             6
#define LWM2M_SERVER_BINDING_ID             7
#define LWM2M_SERVER_UPDATE_ID              8

/*
 * security mode for LWM2M Security Object
*/
#define LWM2M_SECURITY_MODE_PRE_SHARED_KEY  0
#define LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY  1
#define LWM2M_SECURITY_MODE_CERTIFICATE     2
#define LWM2M_SECURITY_MODE_NONE            3

/*
 * Utility functions for sorted linked list
*/
typedef struct _lwm2m_list_t
{
    struct _lwm2m_list_t *next;
    uint16_t              id;
} lwm2m_list_t;

/*
 * defined in list.c
 * Add 'node' to the list 'head' and return the new list
*/
lwm2m_list_t * lwm2m_list_add( lwm2m_list_t *head,
                               lwm2m_list_t *node );

/*
 * Return the node with ID 'id' from the list 'head' or NULL if not found
*/
lwm2m_list_t * lwm2m_list_find( lwm2m_list_t *head,
                                uint16_t      id );

/*
 * Remove the node with ID 'id' from the list 'head' and return the new list
*/
lwm2m_list_t * lwm2m_list_remove( lwm2m_list_t  *head,
                                  uint16_t       id,
                                  lwm2m_list_t **nodeP );

/*
 * Return the lowest unused ID in the list 'head'
*/
uint16_t lwm2m_list_newId( lwm2m_list_t * head );

/*
 * Free a list. Do not use if nodes contain allocated pointers as it calls lwm2m_free on nodes only.
 * If the nodes of the list need to do more than just "free()" their instances, don't use lwm2m_list_free().
 */
void lwm2m_list_free( lwm2m_list_t * head );

#define LWM2M_LIST_ADD(H,N)             lwm2m_list_add((lwm2m_list_t *)H, (lwm2m_list_t *)N);
#define LWM2M_LIST_RM(H,I,N)            lwm2m_list_remove((lwm2m_list_t *)H, I, (lwm2m_list_t **)N);
#define LWM2M_LIST_FIND(H,I)            lwm2m_list_find((lwm2m_list_t *)H, I)
#define LWM2M_LIST_FREE(H)              lwm2m_list_free((lwm2m_list_t *)H)

/*
 * URI
 *
 * objectId is always set
 * instanceId or resourceId are set according to the flag bit-field
 *
*/
typedef struct
{
    uint8_t  flag;      /* indicates which segments are set */
    uint16_t objectId;
    uint16_t instanceId;
    uint16_t resourceId;
} lwm2m_uri_t;

#define LWM2M_MAX_ID                   ((uint16_t)0xFFFF)
#define LWM2M_URI_FLAG_OBJECT_ID       (uint8_t)0x04
#define LWM2M_URI_FLAG_INSTANCE_ID     (uint8_t)0x02
#define LWM2M_URI_FLAG_RESOURCE_ID     (uint8_t)0x01
#define LWM2M_URI_IS_SET_INSTANCE(uri) (((uri)->flag & LWM2M_URI_FLAG_INSTANCE_ID) != 0)
#define LWM2M_URI_IS_SET_RESOURCE(uri) (((uri)->flag & LWM2M_URI_FLAG_RESOURCE_ID) != 0)

/*
 * Parse an URI in LWM2M format and fill the lwm2m_uri_t.
 * Return the number of characters read from buffer or 0 in case of error.
 * Valid URIs: /1, /1/, /1/2, /1/2/, /1/2/3
 * Invalid URIs: /, //, //2, /1//, /1//3, /1/2/3/, /1/2/3/4
*/
#define LWM2M_STRING_ID_MAX_LEN        6
int lwm2m_stringToUri( const char  *buffer,
                       size_t       buffer_len,
                       lwm2m_uri_t *uriP );

/*
 * The lwm2m_data_t is used to store LWM2M resource values in a hierarchical way.
 * Depending on the type the value is different:
 * - LWM2M_TYPE_OBJECT, LWM2M_TYPE_OBJECT_INSTANCE, LWM2M_TYPE_MULTIPLE_RESOURCE: value.asChildren
 * - LWM2M_TYPE_STRING, LWM2M_TYPE_OPAQUE: value.asBuffer
 * - LWM2M_TYPE_INTEGER, LWM2M_TYPE_TIME: value.asInteger
 * - LWM2M_TYPE_FLOAT: value.asFloat
 * - LWM2M_TYPE_BOOLEAN: value.asBoolean
 *
 * LWM2M_TYPE_STRING is also used when the data is in text format.
*/
typedef enum
{
    LWM2M_TYPE_UNDEFINED = 0,
    LWM2M_TYPE_OBJECT,
    LWM2M_TYPE_OBJECT_INSTANCE,
    LWM2M_TYPE_MULTIPLE_RESOURCE,

    LWM2M_TYPE_STRING,
    LWM2M_TYPE_OPAQUE,
    LWM2M_TYPE_INTEGER,
    LWM2M_TYPE_FLOAT,
    LWM2M_TYPE_BOOLEAN,

    LWM2M_TYPE_OBJECT_LINK
} lwm2m_data_type_t;

typedef struct _lwm2m_data_t lwm2m_data_t;
struct _lwm2m_data_t
{
    lwm2m_data_type_t     type;
    uint16_t              id;
    union
    {
        bool              asBoolean;
        int64_t           asInteger;
        double            asFloat;
        struct
        {
            size_t        length;
            uint8_t      *buffer;
        } asBuffer;
        struct
        {
            size_t        count;
            lwm2m_data_t *array;
        } asChildren;
        struct
        {
            uint16_t      objectId;
            uint16_t      objectInstanceId;
        } asObjLink;
    } value;
};

typedef enum
{
    LWM2M_CONTENT_TEXT     = 0,     /* Also used as undefined */
    LWM2M_CONTENT_LINK     = 40,
    LWM2M_CONTENT_OPAQUE   = 42,
    LWM2M_CONTENT_TLV_OLD  = 1542,
    LWM2M_CONTENT_JSON_OLD = 1543,
    LWM2M_CONTENT_TLV      = 11542,
    LWM2M_CONTENT_JSON     = 11543
} lwm2m_media_type_t;

lwm2m_data_t *lwm2m_data_new( int size );

int lwm2m_data_parse( lwm2m_uri_t       *uriP,
                      uint8_t           *buffer,
                      size_t             bufferLen,
                      lwm2m_media_type_t format,
                      lwm2m_data_t     **dataP );

size_t lwm2m_data_serialize( lwm2m_uri_t        *uriP,
                             int                 size,
                             lwm2m_data_t       *dataP,
                             lwm2m_media_type_t *formatP,
                             uint8_t           **bufferP );

void lwm2m_data_free( int           size,
                      lwm2m_data_t *dataP );

void lwm2m_data_encode_string( const char   *string,
                               lwm2m_data_t *dataP );

void lwm2m_data_encode_nstring( const char   *string,
                                size_t        length,
                                lwm2m_data_t *dataP );

void lwm2m_data_encode_opaque( uint8_t      *buffer,
                               size_t        length,
                               lwm2m_data_t *dataP );

void lwm2m_data_encode_int( int64_t       value,
                            lwm2m_data_t *dataP );

int lwm2m_data_decode_int( const lwm2m_data_t *dataP,
                           int64_t            *valueP );

void lwm2m_data_encode_float( double        value,
                              lwm2m_data_t *dataP );

int lwm2m_data_decode_float( const lwm2m_data_t *dataP,
                             double             *valueP );

void lwm2m_data_encode_bool( bool          value,
                             lwm2m_data_t *dataP );

int lwm2m_data_decode_bool( const lwm2m_data_t *dataP,
                            bool               *valueP );

/*
 * Utility function to parse TLV buffers directly
 *
 * Returned value: number of bytes parsed
 * buffer: buffer to parse
 * buffer_len: length in bytes of buffer
 * oType: (OUT) type of the parsed TLV record. can be:
 *          - LWM2M_TYPE_OBJECT
 *          - LWM2M_TYPE_OBJECT_INSTANCE
 *          - LWM2M_TYPE_MULTIPLE_RESOURCE
 *          - LWM2M_TYPE_OPAQUE
 * oID: (OUT) ID of the parsed TLV record
 * oDataIndex: (OUT) index of the data of the parsed TLV record in the buffer
 * oDataLen: (OUT) length of the data of the parsed TLV record
*/
#define LWM2M_TLV_HEADER_MAX_LENGTH    6
int lwm2m_decode_TLV( const uint8_t     *buffer,
                      size_t             buffer_len,
                      lwm2m_data_type_t *oType,
                      uint16_t          *oID,
                      size_t            *oDataIndex,
                      size_t            *oDataLen );

/*
 * LWM2M Objects
 *
 * For the read callback, if *numDataP is not zero, *dataArrayP is pre-allocated
 * and contains the list of resources to read.
 *
*/
typedef struct _lwm2m_object_t lwm2m_object_t;
typedef uint8_t(*lwm2m_read_callback_t)( uint16_t        instanceId,
                                         int            *numDataP,
                                         lwm2m_data_t  **dataArrayP,
                                         lwm2m_object_t *objectP);
typedef uint8_t(*lwm2m_write_callback_t)( uint16_t        instanceId,
                                          int             numData,
                                          lwm2m_data_t   *dataArray,
                                          lwm2m_object_t *objectP );
typedef uint8_t(*lwm2m_execute_callback_t)( uint16_t        instanceId,
                                            uint16_t        resourceId,
                                            uint8_t        *buffer,
                                            int             length,
                                            lwm2m_object_t *objectP);
typedef uint8_t(*lwm2m_create_callback_t)( uint16_t        instanceId,
                                           int             numData,
                                           lwm2m_data_t   *dataArray,
                                           lwm2m_object_t *objectP);
typedef uint8_t(*lwm2m_delete_callback_t)( uint16_t        instanceId,
                                           lwm2m_object_t *objectP );
typedef uint8_t(*lwm2m_discover_callback_t)( uint16_t        instanceId,
                                             int            *numDataP,
                                             lwm2m_data_t  **dataArrayP,
                                             lwm2m_object_t *objectP );
struct _lwm2m_object_t
{
    lwm2m_object_t           *next;  /* matches lwm2m_list_t::next */
    uint16_t                  objID; /* matches lwm2m_list_t::id */
    lwm2m_list_t             *instanceList;
    lwm2m_read_callback_t     readFunc;
    lwm2m_write_callback_t    writeFunc;
    lwm2m_execute_callback_t  executeFunc;
    lwm2m_create_callback_t   createFunc;
    lwm2m_delete_callback_t   deleteFunc;
    lwm2m_discover_callback_t discoverFunc;
    void                     *userData;
};

typedef enum
{
    STATE_DEREGISTERED = 0,   /* not registered or boostrap not started */
    STATE_REG_PENDING,        /* registration pending */
    STATE_REGISTERED,         /* successfully registered */
    STATE_REG_FAILED,         /* last registration failed */
    STATE_REG_UPDATE_PENDING, /* registration update pending */
    STATE_REG_UPDATE_NEEDED,  /* registration update required */
    STATE_DEREG_PENDING,      /* deregistration pending */
    STATE_BS_HOLD_OFF,        /* bootstrap hold off time */
    STATE_BS_INITIATED,       /* bootstrap request sent */
    STATE_BS_PENDING,         /* boostrap on going */
    STATE_BS_FINISHED,        /* bootstrap done */
    STATE_BS_FAILED           /* bootstrap failed */
} lwm2m_status_t;

typedef enum
{
    BINDING_UNKNOWN = 0,
    BINDING_U,   /* UDP - only supported */
    BINDING_UQ,  /* UDP queue mode */
    BINDING_S,   /* SMS */
    BINDING_SQ,  /* SMS queue mode */
    BINDING_US,  /* UDP plus SMS */
    BINDING_UQS  /* UDP queue mode plus SMS */
} lwm2m_binding_t;

typedef struct _lwm2m_block1_data_t
{
    uint8_t *block1buffer;     /* data buffer */
    size_t   block1bufferSize; /* buffer size */
    uint16_t lastmid;          /* mid of the last message received */
} lwm2m_block1_data_t;

typedef struct _lwm2m_server_t
{
    struct _lwm2m_server_t *next;         /* matches lwm2m_list_t::next */
    uint16_t                secObjInstID; /* matches lwm2m_list_t::id */
    uint16_t                shortID;      /* servers short ID, may be 0 for bootstrap server */
    time_t                  lifetime;     /* lifetime of the registration in sec or 0 if default value (86400 sec), also used as hold off time for bootstrap servers */
    time_t                  registration; /* date of the last registration in sec or end of client hold off time for bootstrap servers */
    lwm2m_binding_t         binding;      /* client connection mode with this server */
    void *                  sessionH;
    lwm2m_status_t          status;
    char *                  location;
    bool                    dirty;
    lwm2m_block1_data_t    *block1Data;   /* buffer to handle block1 data, should be replace by a list to support several block1 transfer by server. */
} lwm2m_server_t;

/*
 * LWM2M Link Attributes
 *
 * Used for observation parameters.
 *
*/
#define LWM2M_ATTR_FLAG_MIN_PERIOD     (uint8_t)0x01
#define LWM2M_ATTR_FLAG_MAX_PERIOD     (uint8_t)0x02
#define LWM2M_ATTR_FLAG_GREATER_THAN   (uint8_t)0x04
#define LWM2M_ATTR_FLAG_LESS_THAN      (uint8_t)0x08
#define LWM2M_ATTR_FLAG_STEP           (uint8_t)0x10
typedef struct _lwm2m_attributes_t
{
    uint8_t  toSet;
    uint8_t  toClear;
    uint32_t minPeriod;
    uint32_t maxPeriod;
    double   greaterThan;
    double   lessThan;
    double   step;
} lwm2m_attributes_t;

/*
 * LWM2M transaction
 *
 * Adaptation of Erbium's coap_transaction_t
*/
typedef enum
{
    ENDPOINT_UNKNOWN = 0,
    ENDPOINT_CLIENT,
    ENDPOINT_SERVER
} lwm2m_endpoint_type_t;

typedef struct _lwm2m_transaction_t lwm2m_transaction_t;
typedef void(*lwm2m_transaction_callback_t)( lwm2m_transaction_t *transacP,
                                             void                *message);
struct _lwm2m_transaction_t
{
    lwm2m_transaction_t         *next;             /* matches lwm2m_list_t::next */
    uint16_t                     mID;              /* matches lwm2m_list_t::id */
    lwm2m_endpoint_type_t        peerType;
    void *                       peerP;
    uint8_t                      ack_received;     /* indicates, that the ACK was received */
    time_t                       response_timeout; /* timeout to wait for response, if token is used. When 0, use calculated acknowledge timeout. */
    uint8_t                      retrans_counter;
    time_t                       retrans_time;
    char                         objStringID[LWM2M_STRING_ID_MAX_LEN];
    char                         instanceStringID[LWM2M_STRING_ID_MAX_LEN];
    char                         resourceStringID[LWM2M_STRING_ID_MAX_LEN];
    void                        *message;
    uint16_t                     buffer_len;
    uint8_t                     *buffer;
    lwm2m_transaction_callback_t callback;
    void                        *userData;
};

/*
 * LWM2M observed resources
*/
typedef struct _lwm2m_watcher_t
{
    struct _lwm2m_watcher_t *next;

    bool                     active;
    bool                     update;
    lwm2m_server_t          *server;
    lwm2m_attributes_t      *parameters;
    uint8_t                  token[8];
    size_t                   tokenLen;
    time_t                   lastTime;
    uint32_t                 counter;
    uint16_t                 lastMid;
    union
    {
        int64_t              asInteger;
        double               asFloat;
    } lastValue;
} lwm2m_watcher_t;

typedef struct _lwm2m_observed_t
{
    struct _lwm2m_observed_t *next;

    lwm2m_uri_t               uri;
    lwm2m_watcher_t          *watcherList;
} lwm2m_observed_t;

typedef enum
{
    STATE_INITIAL = 0,
    STATE_BOOTSTRAP_REQUIRED,
    STATE_BOOTSTRAPPING,
    STATE_REGISTER_REQUIRED,
    STATE_REGISTERING,
    STATE_READY,
    STATE_RESET
} lwm2m_client_state_t;

typedef struct
{
    lwm2m_client_state_t       state;
    const char                *endpointName;
    const char                *authCode;
    lwm2m_server_t            *bootstrapServerList;
    lwm2m_server_t            *serverList;
    lwm2m_object_t            *objectList;
    lwm2m_observed_t          *observedList;
    uint16_t                   nextMID;
    lwm2m_transaction_t       *transactionList;
    void                      *userData;
} lwm2m_context_t;

typedef enum
{
    LWM2M_USERDATA_BOOTSTRAP = 0x1,
    LWM2M_USERDATA_STORING   = 0x2
} lwm2m_userdata_flag_t;

typedef uint8_t uint24_t[3];
typedef struct
{
    const char *uri;
    uint24_t    lifetime;
    uint8_t     flag;
} lwm2m_userdata_t;

#define LWM2M_UINT32(x)   (((uint32_t)(x)[0]<<16)| \
                           ((uint32_t)(x)[1]<< 8)| \
                           ((uint32_t)(x)[2]<< 0))
#define LWM2M_UINT24(x,v) {(x)[0]=((v)>>16)&0xff; \
                           (x)[1]=((v)>> 8)&0xff; \
                           (x)[2]=((v)>> 0)&0xff;}

/*
 * initialize a liblwm2m context.
*/
int lwm2m_init( lwm2m_context_t *contextP,
                void            *userData );

/*
 * close a liblwm2m context.
*/
void lwm2m_close( lwm2m_context_t *contextP );

/*
 * perform any required pending operation and adjust timeoutP to the maximal time interval to wait in seconds.
*/
int lwm2m_step( lwm2m_context_t *contextP,
                time_t          *timeoutP );

/*
 * dispatch received data to liblwm2m
*/
void lwm2m_handle_packet( lwm2m_context_t *contextP,
                          uint8_t         *buffer,
                          int              length,
                          void            *fromSessionH );

/*
 * configure the client side with the Endpoint Name, authCode
 * for objects (can't be nil) and a list of objects.
 * LWM2M Security Object (ID 0) must be present with either a bootstrap server or a LWM2M server and
 * its matching LWM2M Server Object (ID 1) instance
 * notice: endpointName、authCode以及objectList指向的内存块必须在运行过程中有效
*/
int lwm2m_configure( lwm2m_context_t *contextP,
                     const char      *endpointName,
                     const char      *authCode,
                     lwm2m_object_t  *objectList );

/*
* send a registration update to the server specified by the server short identifier
* or all if the ID is 0.
* If withObjects is true, the registration update contains the object list.
*/
int lwm2m_update_registration( lwm2m_context_t *contextP,
                               uint16_t         shortServerID,
                               bool             withObjects );

void lwm2m_resource_value_changed( lwm2m_context_t *contextP,
                                   lwm2m_uri_t     *uriP );

/*
 * Returns a session handle that MUST uniquely identify a peer.
 * secObjInstID: ID of the Securty Object instance to open a connection to
 * userData: parameter to lwm2m_init()
*/
void * lwm2m_connect_server( uint16_t secObjInstID,
                             void    *userData );

/*
 * Close a session created by lwm2m_connect_server()
 * sessionH: session handle identifying the peer (opaque to the core)
 * userData: parameter to lwm2m_init()
*/
void lwm2m_close_connection( void *sessionH,
                             void *userData);

/*
 * Send data to a peer
 * Returns COAP_NO_ERROR or a COAP_NNN error code
 * sessionH: session handle identifying the peer (opaque to the core)
 * buffer, length: data to send
 * userData: parameter to lwm2m_init()
*/
uint8_t lwm2m_buffer_send( void    *sessionH,
                           uint8_t *buffer,
                           size_t   length,
                           void    *userData );

/*
 * Compare two session handles
 * Returns true if the two sessions identify the same peer. false otherwise.
 * userData: parameter to lwm2m_init()
*/
bool lwm2m_session_is_equal( void *session1,
                             void *session2,
                             void *userData );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_LWM2M_LWM2M_H_*/
