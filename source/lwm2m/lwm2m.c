/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

int lwm2m_init( lwm2m_context_t *contextP,
                void            *userData )
{
    LOG( "Entering" );
    if ( NULL == contextP )
    {
        return -1;
    }

    nbiot_memzero( contextP, sizeof(lwm2m_context_t) );
    contextP->userData = userData;
    contextP->nextMID = nbiot_rand();

    return 0;
}

void lwm2m_deregister( lwm2m_context_t *context )
{
    lwm2m_server_t * server = context->serverList;

    LOG( "Entering" );
    while ( NULL != server )
    {
        registration_deregister( context, server );
        server = server->next;
    }
}

static void prv_deleteServer( lwm2m_server_t * serverP )
{
    /* TODO parse transaction and observation to remove the ones related to this server */
    if ( NULL != serverP->location )
    {
        nbiot_free( serverP->location );
    }
    free_block1_buffer( serverP->block1Data );
    nbiot_free( serverP );
}

static void prv_deleteServerList( lwm2m_context_t * context )
{
    while ( NULL != context->serverList )
    {
        lwm2m_server_t * server;
        server = context->serverList;
        context->serverList = server->next;
        prv_deleteServer( server );
    }
}

static void prv_deleteBootstrapServer( lwm2m_server_t * serverP )
{
    /* TODO should we free location as in prv_deleteServer ? */
    /* TODO should we parse transaction and observation to remove the ones related to this server ? */
    free_block1_buffer( serverP->block1Data );
    nbiot_free( serverP );
}

static void prv_deleteBootstrapServerList( lwm2m_context_t * context )
{
    while ( NULL != context->bootstrapServerList )
    {
        lwm2m_server_t * server;
        server = context->bootstrapServerList;
        context->bootstrapServerList = server->next;
        prv_deleteBootstrapServer( server );
    }
}

static void prv_deleteObservedList( lwm2m_context_t * contextP )
{
    while ( NULL != contextP->observedList )
    {
        lwm2m_observed_t * targetP;
        lwm2m_watcher_t * watcherP;

        targetP = contextP->observedList;
        contextP->observedList = contextP->observedList->next;

        for ( watcherP = targetP->watcherList; watcherP != NULL; watcherP = watcherP->next )
        {
            if ( watcherP->parameters != NULL ) nbiot_free( watcherP->parameters );
        }
        LWM2M_LIST_FREE( targetP->watcherList );

        nbiot_free( targetP );
    }
}

void prv_deleteTransactionList( lwm2m_context_t * context )
{
    while ( NULL != context->transactionList )
    {
        lwm2m_transaction_t * transaction;

        transaction = context->transactionList;
        context->transactionList = context->transactionList->next;
        transaction_free( transaction );
    }
}

void lwm2m_close( lwm2m_context_t *contextP )
{
    LOG( "Entering" );
    lwm2m_deregister( contextP );
    prv_deleteServerList( contextP );
    prv_deleteBootstrapServerList( contextP );
    prv_deleteObservedList( contextP );
    prv_deleteTransactionList( contextP );
}

static int prv_refreshServerList( lwm2m_context_t * contextP )
{
    lwm2m_server_t * targetP;
    lwm2m_server_t * nextP;

    /* Remove all servers marked as dirty */
    targetP = contextP->bootstrapServerList;
    contextP->bootstrapServerList = NULL;
    while ( targetP != NULL )
    {
        nextP = targetP->next;
        targetP->next = NULL;
        if ( !targetP->dirty )
        {
            targetP->status = STATE_DEREGISTERED;
            contextP->bootstrapServerList = (lwm2m_server_t *)LWM2M_LIST_ADD( contextP->bootstrapServerList, targetP );
        }
        else
        {
            prv_deleteServer( targetP );
        }
        targetP = nextP;
    }
    targetP = contextP->serverList;
    contextP->serverList = NULL;
    while ( targetP != NULL )
    {
        nextP = targetP->next;
        targetP->next = NULL;
        if ( !targetP->dirty )
        {
            /* TODO: Should we revert the status to STATE_DEREGISTERED ? */
            contextP->serverList = (lwm2m_server_t *)LWM2M_LIST_ADD( contextP->serverList, targetP );
        }
        else
        {
            prv_deleteServer( targetP );
        }
        targetP = nextP;
    }

    return object_getServers( contextP );
}

int lwm2m_configure( lwm2m_context_t *contextP,
                     const char      *endpointName,
                     const char      *authCode,
                     lwm2m_object_t  *objectList )
{
    lwm2m_userdata_t *userData;

    LOG_ARG( "endpointName: \"%s\", authCode: \"%s\", msisdn: \"null\", altPath: \"null\"", endpointName, authCode );
    /* This API can be called only once for now */
    if ( contextP->endpointName != NULL ||
         contextP->authCode != NULL ||
         contextP->objectList != NULL )
    {
        return COAP_400_BAD_REQUEST;
    }

    if ( endpointName == NULL ||
         authCode == NULL ||
         objectList == NULL ||
         contextP->userData == NULL )
    {
        return COAP_400_BAD_REQUEST;
    }

    userData = (lwm2m_userdata_t*)contextP->userData;
    if ( userData->uri == NULL ||
         LWM2M_UINT32(userData->lifetime) == 0 )
    {
        return COAP_400_BAD_REQUEST;
    }

    contextP->endpointName = endpointName;
    contextP->authCode = authCode;
    contextP->objectList = objectList;

    return COAP_NO_ERROR;
}

int lwm2m_add_object( lwm2m_context_t * contextP,
                      lwm2m_object_t * objectP )
{
    lwm2m_object_t * targetP;

    LOG_ARG( "ID: %d", objectP->objID );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, objectP->objID );
    if ( targetP != NULL ) return COAP_406_NOT_ACCEPTABLE;
    objectP->next = NULL;

    contextP->objectList = (lwm2m_object_t *)LWM2M_LIST_ADD( contextP->objectList, objectP );

    if ( contextP->state == STATE_READY )
    {
        return lwm2m_update_registration( contextP, 0, true );
    }

    return COAP_NO_ERROR;
}

int lwm2m_remove_object( lwm2m_context_t * contextP,
                         uint16_t id )
{
    /* uint16_t i; */
    lwm2m_object_t * targetP;

    LOG_ARG( "ID: %d", id );
    contextP->objectList = (lwm2m_object_t *)LWM2M_LIST_RM( contextP->objectList, id, &targetP );

    if ( targetP == NULL ) return COAP_404_NOT_FOUND;

    if ( contextP->state == STATE_READY )
    {
        return lwm2m_update_registration( contextP, 0, true );
    }

    return 0;
}

int lwm2m_step( lwm2m_context_t * contextP,
                time_t * timeoutP )
{
    time_t tv_sec;
    int result;

    LOG_ARG( "timeoutP: %" PRId64, *timeoutP );
    tv_sec = nbiot_time();
    if ( tv_sec < 0 ) return COAP_500_INTERNAL_SERVER_ERROR;

    LOG_ARG( "State: %s", STR_STATE( contextP->state ) );
    /* state can also be modified in bootstrap_handleCommand(). */
next_step:
    switch ( contextP->state )
    {
        case STATE_INITIAL:
        if ( 0 != prv_refreshServerList( contextP ) ) return COAP_503_SERVICE_UNAVAILABLE;
        if ( contextP->serverList != NULL )
        {
            contextP->state = STATE_REGISTER_REQUIRED;
        }
        else
        {
            /* Bootstrapping */
            contextP->state = STATE_BOOTSTRAP_REQUIRED;
        }
        goto next_step;
        break;

        case STATE_BOOTSTRAP_REQUIRED:
#ifdef LWM2M_BOOTSTRAP
        if ( contextP->bootstrapServerList != NULL )
        {
            bootstrap_start( contextP );
            contextP->state = STATE_BOOTSTRAPPING;
            bootstrap_step( contextP, (uint32_t)tv_sec, timeoutP );
        }
        else
#endif
        {
            return COAP_503_SERVICE_UNAVAILABLE;
        }
        break;

#ifdef LWM2M_BOOTSTRAP
        case STATE_BOOTSTRAPPING:
        switch ( bootstrap_getStatus( contextP ) )
        {
            case STATE_BS_FINISHED:
            contextP->state = STATE_INITIAL;
            goto next_step;
            break;

            case STATE_BS_FAILED:
            return COAP_503_SERVICE_UNAVAILABLE;

            default:
            /* keep on waiting */
            bootstrap_step( contextP, (uint32_t)tv_sec, timeoutP );
            break;
        }
        break;
#endif
        case STATE_REGISTER_REQUIRED:
        result = registration_start( contextP );
        if ( COAP_NO_ERROR != result ) return result;
        contextP->state = STATE_REGISTERING;
        break;

        case STATE_REGISTERING:
        {
            switch ( registration_getStatus( contextP ) )
            {
                case STATE_REGISTERED:
                contextP->state = STATE_READY;
                break;

                case STATE_REG_FAILED:
                /* TODO avoid infinite loop by checking the bootstrap info is different */
                contextP->state = STATE_BOOTSTRAP_REQUIRED;
                goto next_step;
                break;

                case STATE_REG_PENDING:
                default:
                /* keep on waiting */
                break;
            }
        }
        break;

        case STATE_READY:
        if ( registration_getStatus( contextP ) == STATE_REG_FAILED )
        {
            /* TODO avoid infinite loop by checking the bootstrap info is different */
            contextP->state = STATE_BOOTSTRAP_REQUIRED;
            goto next_step;
            break;
        }
        break;

        default:
        /* do nothing */
        break;
    }

    observe_step( contextP, tv_sec, timeoutP );
    registration_step( contextP, tv_sec, timeoutP );
    transaction_step( contextP, tv_sec, timeoutP );

    LOG_ARG( "Final timeoutP: %" PRId64, *timeoutP );
    LOG_ARG( "Final state: %s", STR_STATE( contextP->state ) );
    return 0;
}
