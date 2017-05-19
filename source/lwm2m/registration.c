/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

#define MAX_LOCATION_LENGTH 10      /* strlen("/rd/65534") + 1 */

static int prv_getRegistrationQuery( lwm2m_context_t * contextP,
                                     lwm2m_server_t * server,
                                     char * buffer,
                                     size_t length )
{
    int index;
    int res;

    index = utils_stringCopy( buffer, length, "?ep=" );
    if ( index < 0 ) return 0;
    res = utils_stringCopy( buffer + index, length - index, contextP->endpointName );
    if ( res < 0 ) return 0;
    index += res;

    switch ( server->binding )
    {
        case BINDING_U:
        res = utils_stringCopy( buffer + index, length - index, "&b=U" );
        break;
        case BINDING_UQ:
        res = utils_stringCopy( buffer + index, length - index, "&b=UQ" );
        break;
        case BINDING_S:
        res = utils_stringCopy( buffer + index, length - index, "&b=S" );
        break;
        case BINDING_SQ:
        res = utils_stringCopy( buffer + index, length - index, "&b=SQ" );
        break;
        case BINDING_US:
        res = utils_stringCopy( buffer + index, length - index, "&b=US" );
        break;
        case BINDING_UQS:
        res = utils_stringCopy( buffer + index, length - index, "&b=UQS" );
        break;
        default:
        res = -1;
    }
    if ( res < 0 ) return 0;

    return index + res;
}

static void prv_handleRegistrationReply( lwm2m_transaction_t * transacP,
                                         void * message )
{
    coap_packet_t * packet = (coap_packet_t *)message;
    lwm2m_server_t * targetP = (lwm2m_server_t *)(transacP->peerP);

    if ( targetP->status == STATE_REG_PENDING )
    {
        time_t tv_sec = nbiot_time();
        if ( tv_sec >= 0 )
        {
            targetP->registration = tv_sec;
        }
        if ( packet != NULL && packet->code == COAP_201_CREATED )
        {
            targetP->status = STATE_REGISTERED;
            if ( NULL != targetP->location )
            {
                nbiot_free( targetP->location );
            }
            targetP->location = coap_get_multi_option_as_string( packet->location_path );

            LOG( "Registration successful" );
        }
        else
        {
            targetP->status = STATE_REG_FAILED;
            LOG( "Registration failed" );
        }
    }
}

#define PRV_QUERY_BUFFER_LENGTH 64

/* send the registration for a single server */
static uint8_t prv_register( lwm2m_context_t * contextP,
                             lwm2m_server_t * server )
{
    char query[64];
    int query_length;
    uint8_t payload[256];
    int payload_length;
    lwm2m_transaction_t * transaction;

    payload_length = object_getRegisterPayload( contextP, payload, sizeof(payload) );
    if ( payload_length == 0 ) return COAP_500_INTERNAL_SERVER_ERROR;

    query_length = prv_getRegistrationQuery( contextP, server, query, sizeof(query) );

    if ( query_length == 0 ) return COAP_500_INTERNAL_SERVER_ERROR;

    if ( 0 != server->lifetime )
    {
        int res;

        res = utils_stringCopy( query + query_length, PRV_QUERY_BUFFER_LENGTH - query_length, QUERY_DELIMITER QUERY_LIFETIME );
        if ( res < 0 ) return COAP_500_INTERNAL_SERVER_ERROR;
        query_length += res;
        res = utils_intCopy( query + query_length, PRV_QUERY_BUFFER_LENGTH - query_length, (int32_t)server->lifetime );
        if ( res < 0 ) return COAP_500_INTERNAL_SERVER_ERROR;
        query_length += res;
    }

    if ( server->sessionH == NULL )
    {
        server->sessionH = lwm2m_connect_server( server->secObjInstID, contextP->userData );
    }

    if ( NULL == server->sessionH ) return COAP_503_SERVICE_UNAVAILABLE;

    transaction = transaction_new( COAP_TYPE_CON, COAP_POST, NULL, NULL, contextP->nextMID++, 4, NULL, ENDPOINT_SERVER, (void *)server );
    if ( transaction == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

    coap_set_header_uri_path( transaction->message, "/"URI_REGISTRATION_SEGMENT );
    coap_set_header_uri_query( transaction->message, query );
    coap_set_header_content_type( transaction->message, LWM2M_CONTENT_LINK );
    if ( contextP->authCode )
    {
        coap_set_header_auth_code( transaction->message, contextP->authCode );
    }
    coap_set_payload( transaction->message, payload, payload_length );

    transaction->callback = prv_handleRegistrationReply;
    transaction->userData = (void *)server;

    contextP->transactionList = (lwm2m_transaction_t *)LWM2M_LIST_ADD( contextP->transactionList, transaction );
    if ( transaction_send( contextP, transaction ) != 0 ) return COAP_500_INTERNAL_SERVER_ERROR;

    server->status = STATE_REG_PENDING;

    return COAP_NO_ERROR;
}

static void prv_handleRegistrationUpdateReply( lwm2m_transaction_t * transacP,
                                               void * message )
{
    coap_packet_t * packet = (coap_packet_t *)message;
    lwm2m_server_t * targetP = (lwm2m_server_t *)(transacP->peerP);

    if ( targetP->status == STATE_REG_UPDATE_PENDING )
    {
        time_t tv_sec = nbiot_time();
        if ( tv_sec >= 0 )
        {
            targetP->registration = tv_sec;
        }
        if ( packet != NULL && packet->code == COAP_204_CHANGED )
        {
            targetP->status = STATE_REGISTERED;
            LOG( "Registration update successful" );
        }
        else
        {
            targetP->status = STATE_REG_FAILED;
            LOG( "Registration update failed" );
        }
    }
}

static int prv_updateRegistration( lwm2m_context_t * contextP,
                                   lwm2m_server_t * server,
                                   bool withObjects )
{
    lwm2m_transaction_t * transaction;
    uint8_t payload[256];
    int payload_length;
    char query[64];
    int query_length;
    int res;

    if ( contextP->endpointName == NULL ||
         contextP->authCode == NULL )
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    transaction = transaction_new( COAP_TYPE_CON, COAP_POST, NULL, NULL, contextP->nextMID++, 4, NULL, ENDPOINT_SERVER, (void *)server );
    if ( transaction == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

    coap_set_header_uri_path( transaction->message, server->location );
    if ( contextP->authCode )
    {
        coap_set_header_auth_code( transaction->message, contextP->authCode );
    }
    /* endpoint name */
    {
        query_length = utils_stringCopy( query, PRV_QUERY_BUFFER_LENGTH, "?ep=" );
        if ( query_length < 0 ) return COAP_500_INTERNAL_SERVER_ERROR;
        res = utils_stringCopy( query + query_length, PRV_QUERY_BUFFER_LENGTH - query_length, contextP->endpointName );
        if ( res < 0 ) return COAP_500_INTERNAL_SERVER_ERROR;
        query_length += res;
        coap_set_header_uri_query( transaction->message, query );
    }

    if ( withObjects == true )
    {
        payload_length = object_getRegisterPayload( contextP, payload, sizeof(payload) );
        if ( payload_length == 0 )
        {
            transaction_free( transaction );
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        coap_set_payload( transaction->message, payload, payload_length );
    }

    transaction->callback = prv_handleRegistrationUpdateReply;
    transaction->userData = (void *)server;

    contextP->transactionList = (lwm2m_transaction_t *)LWM2M_LIST_ADD( contextP->transactionList, transaction );

    if ( transaction_send( contextP, transaction ) == 0 )
    {
        server->status = STATE_REG_UPDATE_PENDING;
    }

    return COAP_NO_ERROR;
}

/* update the registration of a given server */
int lwm2m_update_registration( lwm2m_context_t * contextP,
                               uint16_t shortServerID,
                               bool withObjects )
{
    lwm2m_server_t * targetP;
    uint8_t result;

    LOG_ARG( "State: %s, shortServerID: %d", STR_STATE( contextP->state ), shortServerID );

    result = COAP_NO_ERROR;

    targetP = contextP->serverList;
    if ( targetP == NULL )
    {
        if ( object_getServers( contextP ) == -1 )
        {
            LOG( "No server found" );
            return COAP_404_NOT_FOUND;
        }
    }
    while ( targetP != NULL && result == COAP_NO_ERROR )
    {
        if ( shortServerID != 0 )
        {
            if ( targetP->shortID == shortServerID )
            {
                /* found the server, trigger the update transaction */
                if ( targetP->status == STATE_REGISTERED
                     || targetP->status == STATE_REG_UPDATE_PENDING )
                {
                    targetP->status = STATE_REG_UPDATE_NEEDED;
                    return COAP_NO_ERROR;
                }
                else
                {
                    return COAP_400_BAD_REQUEST;
                }
            }
        }
        else
        {
            if ( targetP->status == STATE_REGISTERED
                 || targetP->status == STATE_REG_UPDATE_PENDING )
            {
                targetP->status = STATE_REG_UPDATE_NEEDED;
            }
        }
        targetP = targetP->next;
    }

    if ( shortServerID != 0
         && targetP == NULL )
    {
        /* no server found */
        result = COAP_404_NOT_FOUND;
    }

    return result;
}

uint8_t registration_start( lwm2m_context_t * contextP )
{
    lwm2m_server_t * targetP;
    uint8_t result;

    LOG_ARG( "State: %s", STR_STATE( contextP->state ) );

    result = COAP_NO_ERROR;

    targetP = contextP->serverList;
    while ( targetP != NULL && result == COAP_NO_ERROR )
    {
        if ( targetP->status == STATE_DEREGISTERED
             || targetP->status == STATE_REG_FAILED )
        {
            result = prv_register( contextP, targetP );
        }
        targetP = targetP->next;
    }

    return result;
}


/*
* Returns STATE_REG_PENDING if at least one registration is still pending
* Returns STATE_REGISTERED if no registration is pending and there is at least one server the client is registered to
* Returns STATE_REG_FAILED if all registration failed.
*/
lwm2m_status_t registration_getStatus( lwm2m_context_t * contextP )
{
    lwm2m_server_t * targetP;
    lwm2m_status_t reg_status;

    LOG_ARG( "State: %s", STR_STATE( contextP->state ) );

    targetP = contextP->serverList;
    reg_status = STATE_REG_FAILED;

    while ( targetP != NULL )
    {
        LOG_ARG( "targetP->status: %s", STR_STATUS( targetP->status ) );
        switch ( targetP->status )
        {
            case STATE_REGISTERED:
            case STATE_REG_UPDATE_NEEDED:
            case STATE_REG_UPDATE_PENDING:
            if ( reg_status == STATE_REG_FAILED )
            {
                reg_status = STATE_REGISTERED;
            }
            break;

            case STATE_REG_PENDING:
            reg_status = STATE_REG_PENDING;
            break;

            case STATE_REG_FAILED:
            case STATE_DEREG_PENDING:
            case STATE_DEREGISTERED:
            default:
            break;
        }
        LOG_ARG( "reg_status: %s", STR_STATUS( reg_status ) );

        targetP = targetP->next;
    }

    return reg_status;
}

static void prv_handleDeregistrationReply( lwm2m_transaction_t * transacP,
                                           void * message )
{
    lwm2m_server_t * targetP;

    targetP = (lwm2m_server_t *)(transacP->peerP);
    if ( NULL != targetP )
    {
        if ( targetP->status == STATE_DEREG_PENDING )
        {
            targetP->status = STATE_DEREGISTERED;
        }
    }
}

void registration_deregister( lwm2m_context_t * contextP,
                              lwm2m_server_t * serverP )
{
    char query[64];
    int query_length;
    lwm2m_transaction_t * transaction;

    LOG_ARG( "State: %s, serverP->status: %s", STR_STATE( contextP->state ), STR_STATUS( serverP->status ) );

    if ( contextP->state == STATE_RESET
         || serverP->status == STATE_DEREGISTERED
         || serverP->status == STATE_REG_PENDING
         || serverP->status == STATE_DEREG_PENDING
         || serverP->status == STATE_REG_FAILED
         || serverP->location == NULL )
    {
        return;
    }

    transaction = transaction_new( COAP_TYPE_CON, COAP_DELETE, NULL, NULL, contextP->nextMID++, 4, NULL, ENDPOINT_SERVER, (void *)serverP );
    if ( transaction == NULL ) return;

    /* endpoint name */
    if ( contextP->endpointName != NULL )
    {
        int res;

        query_length = utils_stringCopy( query, PRV_QUERY_BUFFER_LENGTH, "?ep=" );
        if ( query_length < 0 ) return;
        res = utils_stringCopy( query + query_length, PRV_QUERY_BUFFER_LENGTH - query_length, contextP->endpointName );
        if ( res < 0 ) return;
        query_length += res;
    }

    coap_set_header_uri_path( transaction->message, serverP->location );
    if ( contextP->authCode )
    {
        coap_set_header_auth_code( transaction->message, contextP->authCode );
    }
    coap_set_header_uri_query( transaction->message, query );

    transaction->callback = prv_handleDeregistrationReply;
    transaction->userData = (void *)contextP;

    contextP->transactionList = (lwm2m_transaction_t *)LWM2M_LIST_ADD( contextP->transactionList, transaction );
    if ( transaction_send( contextP, transaction ) == 0 )
    {
        serverP->status = STATE_DEREG_PENDING;
    }
}

/* for each server update the registration if needed */
/* for each client check if the registration expired */
void registration_step( lwm2m_context_t * contextP,
                        time_t currentTime,
                        time_t * timeoutP )
{
    lwm2m_server_t * targetP = contextP->serverList;

    LOG_ARG( "State: %s", STR_STATE( contextP->state ) );

    targetP = contextP->serverList;
    while ( targetP != NULL )
    {
        switch ( targetP->status )
        {
            case STATE_REGISTERED:
            {
                time_t nextUpdate;
                int    interval;

                nextUpdate = targetP->lifetime;
                if ( COAP_MAX_TRANSMIT_WAIT < nextUpdate )
                {
                    nextUpdate -= (time_t)COAP_MAX_TRANSMIT_WAIT;
                }
                else
                {
                    nextUpdate = nextUpdate >> 1;
                }

                interval = targetP->registration + nextUpdate - currentTime;
                if ( 0 >= interval )
                {
                    LOG( "Updating registration" );
                    prv_updateRegistration( contextP, targetP, false );
                }
                else if ( interval < *timeoutP )
                {
                    *timeoutP = interval;
                }
            }
            break;

            case STATE_REG_UPDATE_NEEDED:
            prv_updateRegistration( contextP, targetP, true );
            break;

            case STATE_REG_FAILED:
            if ( targetP->sessionH != NULL )
            {
                lwm2m_close_connection( targetP->sessionH, contextP->userData );
                targetP->sessionH = NULL;
            }
            break;

            default:
            break;
        }
        targetP = targetP->next;
    }
}
