/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

static lwm2m_observed_t * prv_findObserved( lwm2m_context_t * contextP,
                                            lwm2m_uri_t * uriP )
{
    lwm2m_observed_t * targetP;

    targetP = contextP->observedList;
    while ( targetP != NULL
            && (targetP->uri.objectId != uriP->objectId
            || targetP->uri.flag != uriP->flag
            || (LWM2M_URI_IS_SET_INSTANCE( uriP ) && targetP->uri.instanceId != uriP->instanceId)
            || (LWM2M_URI_IS_SET_RESOURCE( uriP ) && targetP->uri.resourceId != uriP->resourceId)) )
    {
        targetP = targetP->next;
    }

    return targetP;
}

static void prv_unlinkObserved( lwm2m_context_t * contextP,
                                lwm2m_observed_t * observedP )
{
    if ( contextP->observedList == observedP )
    {
        contextP->observedList = contextP->observedList->next;
    }
    else
    {
        lwm2m_observed_t * parentP;

        parentP = contextP->observedList;
        while ( parentP->next != NULL
                && parentP->next != observedP )
        {
            parentP = parentP->next;
        }
        if ( parentP->next != NULL )
        {
            parentP->next = parentP->next->next;
        }
    }
}

static lwm2m_watcher_t * prv_findWatcher( lwm2m_observed_t * observedP,
                                          lwm2m_server_t * serverP )
{
    lwm2m_watcher_t * targetP;

    targetP = observedP->watcherList;
    while ( targetP != NULL
            && targetP->server != serverP )
    {
        targetP = targetP->next;
    }

    return targetP;
}

static lwm2m_watcher_t * prv_getWatcher( lwm2m_context_t * contextP,
                                         lwm2m_uri_t * uriP,
                                         lwm2m_server_t * serverP )
{
    lwm2m_observed_t * observedP;
    bool allocatedObserver;
    lwm2m_watcher_t * watcherP;

    allocatedObserver = false;

    observedP = prv_findObserved( contextP, uriP );
    if ( observedP == NULL )
    {
        observedP = (lwm2m_observed_t *)nbiot_malloc( sizeof(lwm2m_observed_t) );
        if ( observedP == NULL ) return NULL;
        allocatedObserver = true;
        nbiot_memzero( observedP, sizeof(lwm2m_observed_t) );
        nbiot_memmove( &(observedP->uri), uriP, sizeof(lwm2m_uri_t) );
        observedP->next = contextP->observedList;
        contextP->observedList = observedP;
    }

    watcherP = prv_findWatcher( observedP, serverP );
    if ( watcherP == NULL )
    {
        watcherP = (lwm2m_watcher_t *)nbiot_malloc( sizeof(lwm2m_watcher_t) );
        if ( watcherP == NULL )
        {
            if ( allocatedObserver == true )
            {
                nbiot_free( observedP );
            }
            return NULL;
        }
        nbiot_memzero( watcherP, sizeof(lwm2m_watcher_t) );
        watcherP->active = false;
        watcherP->server = serverP;
        watcherP->next = observedP->watcherList;
        observedP->watcherList = watcherP;
    }

    return watcherP;
}

coap_status_t observe_handleRequest( lwm2m_context_t * contextP,
                                     lwm2m_uri_t * uriP,
                                     lwm2m_server_t * serverP,
                                     int size,
                                     lwm2m_data_t * dataP,
                                     coap_packet_t * message,
                                     coap_packet_t * response )
{
    lwm2m_watcher_t * watcherP;
    uint32_t count;

    LOG_ARG( "Code: %02X, server status: %s", message->code, STR_STATUS( serverP->status ) );
    LOG_URI( uriP );

    coap_get_header_observe( message, &count );

    switch ( count )
    {
        case 0:
        if ( !LWM2M_URI_IS_SET_INSTANCE( uriP ) && LWM2M_URI_IS_SET_RESOURCE( uriP ) ) return COAP_400_BAD_REQUEST;
        if ( message->token_len == 0 ) return COAP_400_BAD_REQUEST;

        watcherP = prv_getWatcher( contextP, uriP, serverP );
        if ( watcherP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

        watcherP->tokenLen = message->token_len;
        nbiot_memmove( watcherP->token, message->token, message->token_len );
        watcherP->active = true;
        watcherP->lastTime = nbiot_time();

        if ( LWM2M_URI_IS_SET_RESOURCE( uriP ) )
        {
            switch ( dataP->type )
            {
                case LWM2M_TYPE_INTEGER:
                if ( 1 != lwm2m_data_decode_int( dataP, &(watcherP->lastValue.asInteger) ) ) return COAP_500_INTERNAL_SERVER_ERROR;
                break;
                case LWM2M_TYPE_FLOAT:
                if ( 1 != lwm2m_data_decode_float( dataP, &(watcherP->lastValue.asFloat) ) ) return COAP_500_INTERNAL_SERVER_ERROR;
                break;
                default:
                break;
            }
        }

        coap_set_header_observe( response, watcherP->counter++ );

        return COAP_205_CONTENT;

        case 1:
        /* cancellation */
        observe_cancel( contextP, LWM2M_MAX_ID, serverP->sessionH );
        return COAP_205_CONTENT;

        default:
        return COAP_400_BAD_REQUEST;
    }
}

void observe_cancel( lwm2m_context_t * contextP,
                     uint16_t mid,
                     void * fromSessionH )
{
    lwm2m_observed_t * observedP;

    LOG_ARG( "mid: %d", mid );

    for ( observedP = contextP->observedList;
          observedP != NULL;
          observedP = observedP->next )
    {
        lwm2m_watcher_t * targetP = NULL;

        if ( (LWM2M_MAX_ID == mid || observedP->watcherList->lastMid == mid)
             && lwm2m_session_is_equal( observedP->watcherList->server->sessionH, fromSessionH, contextP->userData ) )
        {
            targetP = observedP->watcherList;
            observedP->watcherList = observedP->watcherList->next;
        }
        else
        {
            lwm2m_watcher_t * parentP;

            parentP = observedP->watcherList;
            while ( parentP->next != NULL
                    && (parentP->next->lastMid != mid
                    || lwm2m_session_is_equal( parentP->next->server->sessionH, fromSessionH, contextP->userData )) )
            {
                parentP = parentP->next;
            }
            if ( parentP->next != NULL )
            {
                targetP = parentP->next;
                parentP->next = parentP->next->next;
            }
        }
        if ( targetP != NULL )
        {
            nbiot_free( targetP );
            if ( observedP->watcherList == NULL )
            {
                prv_unlinkObserved( contextP, observedP );
                nbiot_free( observedP );
            }
            return;
        }
    }
}

coap_status_t observe_setParameters( lwm2m_context_t * contextP,
                                     lwm2m_uri_t * uriP,
                                     lwm2m_server_t * serverP,
                                     lwm2m_attributes_t * attrP )
{
    uint8_t result;
    lwm2m_watcher_t * watcherP;

    LOG_URI( uriP );
    LOG_ARG( "toSet: %08X, toClear: %08X, minPeriod: %d, maxPeriod: %d, greaterThan: %f, lessThan: %f, step: %f",
             attrP->toSet, attrP->toClear, attrP->minPeriod, attrP->maxPeriod, attrP->greaterThan, attrP->lessThan, attrP->step );

    if ( !LWM2M_URI_IS_SET_INSTANCE( uriP ) && LWM2M_URI_IS_SET_RESOURCE( uriP ) ) return COAP_400_BAD_REQUEST;

    result = object_checkReadable( contextP, uriP );
    if ( COAP_205_CONTENT != result ) return result;

    if ( 0 != (attrP->toSet & ATTR_FLAG_NUMERIC) )
    {
        result = object_checkNumeric( contextP, uriP );
        if ( COAP_205_CONTENT != result ) return result;
    }

    watcherP = prv_getWatcher( contextP, uriP, serverP );
    if ( watcherP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

    /* Check rule “lt” value + 2*”stp” values < “gt” value */
    if ( (((attrP->toSet | (watcherP->parameters ? watcherP->parameters->toSet : 0)) & ~attrP->toClear) & ATTR_FLAG_NUMERIC) == ATTR_FLAG_NUMERIC )
    {
        float gt;
        float lt;
        float stp;

        if ( 0 != (attrP->toSet & LWM2M_ATTR_FLAG_GREATER_THAN) )
        {
            gt = (float)attrP->greaterThan;
        }
        else
        {
            gt = (float)watcherP->parameters->greaterThan;
        }
        if ( 0 != (attrP->toSet & LWM2M_ATTR_FLAG_LESS_THAN) )
        {
            lt = (float)attrP->lessThan;
        }
        else
        {
            lt = (float)watcherP->parameters->lessThan;
        }
        if ( 0 != (attrP->toSet & LWM2M_ATTR_FLAG_STEP) )
        {
            stp = (float)attrP->step;
        }
        else
        {
            stp = (float)watcherP->parameters->step;
        }

        if ( lt + (2 * stp) >= gt ) return COAP_400_BAD_REQUEST;
    }

    if ( watcherP->parameters == NULL )
    {
        if ( attrP->toSet != 0 )
        {
            watcherP->parameters = (lwm2m_attributes_t *)nbiot_malloc( sizeof(lwm2m_attributes_t) );
            if ( watcherP->parameters == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;
            nbiot_memmove( watcherP->parameters, attrP, sizeof(lwm2m_attributes_t) );
        }
    }
    else
    {
        watcherP->parameters->toSet &= ~attrP->toClear;
        if ( attrP->toSet & LWM2M_ATTR_FLAG_MIN_PERIOD )
        {
            watcherP->parameters->minPeriod = attrP->minPeriod;
        }
        if ( attrP->toSet & LWM2M_ATTR_FLAG_MAX_PERIOD )
        {
            watcherP->parameters->maxPeriod = attrP->maxPeriod;
        }
        if ( attrP->toSet & LWM2M_ATTR_FLAG_GREATER_THAN )
        {
            watcherP->parameters->greaterThan = attrP->greaterThan;
        }
        if ( attrP->toSet & LWM2M_ATTR_FLAG_LESS_THAN )
        {
            watcherP->parameters->lessThan = attrP->lessThan;
        }
        if ( attrP->toSet & LWM2M_ATTR_FLAG_STEP )
        {
            watcherP->parameters->step = attrP->step;
        }
    }

    LOG_ARG( "Final toSet: %08X, minPeriod: %d, maxPeriod: %d, greaterThan: %f, lessThan: %f, step: %f",
             watcherP->parameters->toSet, watcherP->parameters->minPeriod, watcherP->parameters->maxPeriod, watcherP->parameters->greaterThan, watcherP->parameters->lessThan, watcherP->parameters->step );

    return COAP_204_CHANGED;
}

lwm2m_observed_t * observe_findByUri( lwm2m_context_t * contextP,
                                      lwm2m_uri_t * uriP )
{
    lwm2m_observed_t * targetP;

    LOG_URI( uriP );
    targetP = contextP->observedList;
    while ( targetP != NULL )
    {
        if ( targetP->uri.objectId == uriP->objectId )
        {
            if ( (!LWM2M_URI_IS_SET_INSTANCE( uriP ) && !LWM2M_URI_IS_SET_INSTANCE( &(targetP->uri) ))
                 || (LWM2M_URI_IS_SET_INSTANCE( uriP ) && LWM2M_URI_IS_SET_INSTANCE( &(targetP->uri) ) && (uriP->instanceId == targetP->uri.instanceId)) )
            {
                if ( (!LWM2M_URI_IS_SET_RESOURCE( uriP ) && !LWM2M_URI_IS_SET_RESOURCE( &(targetP->uri) ))
                     || (LWM2M_URI_IS_SET_RESOURCE( uriP ) && LWM2M_URI_IS_SET_RESOURCE( &(targetP->uri) ) && (uriP->resourceId == targetP->uri.resourceId)) )
                {
                    LOG_ARG( "Found one with%s observers.", targetP->watcherList ? "" : " no" );
                    LOG_URI( &(targetP->uri) );
                    return targetP;
                }
            }
        }
        targetP = targetP->next;
    }

    LOG( "Found nothing" );
    return NULL;
}

void lwm2m_resource_value_changed( lwm2m_context_t * contextP,
                                   lwm2m_uri_t * uriP )
{
    lwm2m_observed_t * targetP;

    LOG_URI( uriP );
    targetP = contextP->observedList;
    while ( targetP != NULL )
    {
        if ( targetP->uri.objectId == uriP->objectId )
        {
            if ( !LWM2M_URI_IS_SET_INSTANCE( uriP )
                 || (targetP->uri.flag & LWM2M_URI_FLAG_INSTANCE_ID) == 0
                 || uriP->instanceId == targetP->uri.instanceId )
            {
                if ( !LWM2M_URI_IS_SET_RESOURCE( uriP )
                     || (targetP->uri.flag & LWM2M_URI_FLAG_RESOURCE_ID) == 0
                     || uriP->resourceId == targetP->uri.resourceId )
                {
                    lwm2m_watcher_t * watcherP;

                    LOG( "Found an observation" );
                    LOG_URI( &(targetP->uri) );

                    for ( watcherP = targetP->watcherList; watcherP != NULL; watcherP = watcherP->next )
                    {
                        if ( watcherP->active == true )
                        {
                            LOG( "Tagging a watcher" );
                            watcherP->update = true;
                        }
                    }
                }
            }
        }
        targetP = targetP->next;
    }
}

void observe_step( lwm2m_context_t * contextP,
                   time_t currentTime,
                   time_t * timeoutP )
{
    lwm2m_observed_t * targetP;

    LOG( "Entering" );
    for ( targetP = contextP->observedList; targetP != NULL; targetP = targetP->next )
    {
        lwm2m_watcher_t * watcherP;
        uint8_t * buffer = NULL;
        size_t length = 0;
        lwm2m_data_t * dataP = NULL;
        int size = 0;
        double floatValue = 0;
        int64_t integerValue = 0;
        bool storeValue = false;
        lwm2m_media_type_t format = LWM2M_CONTENT_TEXT;
        coap_packet_t message[1];
        time_t interval;

        LOG_URI( &(targetP->uri) );
        if ( LWM2M_URI_IS_SET_RESOURCE( &targetP->uri ) )
        {
            if ( COAP_205_CONTENT != object_readData( contextP, &targetP->uri, &size, &dataP ) ) continue;
            switch ( dataP->type )
            {
                case LWM2M_TYPE_INTEGER:
                if ( 1 != lwm2m_data_decode_int( dataP, &integerValue ) ) continue;
                storeValue = true;
                break;
                case LWM2M_TYPE_FLOAT:
                if ( 1 != lwm2m_data_decode_float( dataP, &floatValue ) ) continue;
                storeValue = true;
                break;
                default:
                break;
            }
        }
        for ( watcherP = targetP->watcherList; watcherP != NULL; watcherP = watcherP->next )
        {
            if ( watcherP->active == true )
            {
                bool notify = false;

                if ( watcherP->update == true )
                {
                    /* value changed, should we notify the server ? */

                    if ( watcherP->parameters == NULL || watcherP->parameters->toSet == 0 )
                    {
                        /* no conditions */
                        notify = true;
                        LOG( "Notify with no conditions" );
                        LOG_URI( &(targetP->uri) );
                    }

                    if ( notify == false
                         && watcherP->parameters != NULL
                         && (watcherP->parameters->toSet & ATTR_FLAG_NUMERIC) != 0 )
                    {
                        if ( (watcherP->parameters->toSet & LWM2M_ATTR_FLAG_LESS_THAN) != 0 )
                        {
                            LOG( "Checking lower treshold" );
                            /* Did we cross the lower treshold ? */
                            switch ( dataP->type )
                            {
                                case LWM2M_TYPE_INTEGER:
                                if ( (integerValue <= watcherP->parameters->lessThan
                                    && watcherP->lastValue.asInteger > watcherP->parameters->lessThan)
                                    || (integerValue >= watcherP->parameters->lessThan
                                    && watcherP->lastValue.asInteger < watcherP->parameters->lessThan) )
                                {
                                    LOG( "Notify on lower treshold crossing" );
                                    notify = true;
                                }
                                break;
                                case LWM2M_TYPE_FLOAT:
                                if ( (floatValue <= watcherP->parameters->lessThan
                                    && watcherP->lastValue.asFloat > watcherP->parameters->lessThan)
                                    || (floatValue >= watcherP->parameters->lessThan
                                    && watcherP->lastValue.asFloat < watcherP->parameters->lessThan) )
                                {
                                    LOG( "Notify on lower treshold crossing" );
                                    notify = true;
                                }
                                break;
                                default:
                                break;
                            }
                        }
                        if ( (watcherP->parameters->toSet & LWM2M_ATTR_FLAG_GREATER_THAN) != 0 )
                        {
                            LOG( "Checking upper treshold" );
                            /* Did we cross the upper treshold ? */
                            switch ( dataP->type )
                            {
                                case LWM2M_TYPE_INTEGER:
                                if ( (integerValue <= watcherP->parameters->greaterThan
                                    && watcherP->lastValue.asInteger > watcherP->parameters->greaterThan)
                                    || (integerValue >= watcherP->parameters->greaterThan
                                    && watcherP->lastValue.asInteger < watcherP->parameters->greaterThan) )
                                {
                                    LOG( "Notify on lower upper crossing" );
                                    notify = true;
                                }
                                break;
                                case LWM2M_TYPE_FLOAT:
                                if ( (floatValue <= watcherP->parameters->greaterThan
                                    && watcherP->lastValue.asFloat > watcherP->parameters->greaterThan)
                                    || (floatValue >= watcherP->parameters->greaterThan
                                    && watcherP->lastValue.asFloat < watcherP->parameters->greaterThan) )
                                {
                                    LOG( "Notify on lower upper crossing" );
                                    notify = true;
                                }
                                break;
                                default:
                                break;
                            }
                        }
                        if ( (watcherP->parameters->toSet & LWM2M_ATTR_FLAG_STEP) != 0 )
                        {
                            LOG( "Checking step" );

                            switch ( dataP->type )
                            {
                                case LWM2M_TYPE_INTEGER:
                                {
                                                           int64_t diff;

                                                           diff = integerValue - watcherP->lastValue.asInteger;
                                                           if ( (diff < 0 && (0 - diff) >= watcherP->parameters->step)
                                                                || (diff >= 0 && diff >= watcherP->parameters->step) )
                                                           {
                                                               LOG( "Notify on step condition" );
                                                               notify = true;
                                                           }
                                }
                                break;
                                case LWM2M_TYPE_FLOAT:
                                {
                                                         double diff;

                                                         diff = floatValue - watcherP->lastValue.asFloat;
                                                         if ( (diff < 0 && (0 - diff) >= watcherP->parameters->step)
                                                              || (diff >= 0 && diff >= watcherP->parameters->step) )
                                                         {
                                                             LOG( "Notify on step condition" );
                                                             notify = true;
                                                         }
                                }
                                break;
                                default:
                                break;
                            }
                        }
                    }

                    if ( watcherP->parameters != NULL
                         && (watcherP->parameters->toSet & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0 )
                    {
                        LOG_ARG( "Checking minimal period (%d s)", watcherP->parameters->minPeriod );

                        if ( watcherP->lastTime + watcherP->parameters->minPeriod > currentTime )
                        {
                            /* Minimum Period did not elapse yet */
                            interval = watcherP->lastTime + watcherP->parameters->minPeriod - currentTime;
                            if ( *timeoutP > interval ) *timeoutP = interval;
                            notify = false;
                        }
                        else
                        {
                            LOG( "Notify on minimal period" );
                            notify = true;
                        }
                    }
                }

                /* Is the Maximum Period reached ? */
                if ( notify == false
                     && watcherP->parameters != NULL
                     && (watcherP->parameters->toSet & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0 )
                {
                    LOG_ARG( "Checking maximal period (%d s)", watcherP->parameters->minPeriod );

                    if ( watcherP->lastTime + watcherP->parameters->maxPeriod <= currentTime )
                    {
                        LOG( "Notify on maximal period" );
                        notify = true;
                    }
                }

                if ( notify == true )
                {
                    if ( buffer == NULL )
                    {
                        if ( dataP != NULL )
                        {
                            length = lwm2m_data_serialize( &targetP->uri, size, dataP, &format, &buffer );
                            if ( length == 0 ) break;
                        }
                        else
                        {
                            if ( COAP_205_CONTENT != object_read( contextP, &targetP->uri, &format, &buffer, &length ) )
                            {
                                buffer = NULL;
                                break;
                            }
                        }
                        coap_init_message( message, COAP_TYPE_NON, COAP_205_CONTENT, 0 );
                        coap_set_header_content_type( message, format );
                        coap_set_payload( message, buffer, length );
                    }
                    watcherP->lastTime = currentTime;
                    watcherP->lastMid = contextP->nextMID++;
                    message->mid = watcherP->lastMid;
                    coap_set_header_token( message, watcherP->token, watcherP->tokenLen );
                    coap_set_header_observe( message, watcherP->counter++ );
                    (void)message_send( contextP, message, watcherP->server->sessionH );
                    watcherP->update = false;
                }

                /* Store this value */
                if ( notify == true && storeValue == true )
                {
                    switch ( dataP->type )
                    {
                        case LWM2M_TYPE_INTEGER:
                        watcherP->lastValue.asInteger = integerValue;
                        break;
                        case LWM2M_TYPE_FLOAT:
                        watcherP->lastValue.asFloat = floatValue;
                        break;
                        default:
                        break;
                    }
                }

                if ( watcherP->parameters != NULL && (watcherP->parameters->toSet & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0 )
                {
                    /* update timers */
                    interval = watcherP->lastTime + watcherP->parameters->maxPeriod - currentTime;
                    if ( *timeoutP > interval ) *timeoutP = interval;
                }
            }
        }
        if ( dataP != NULL ) lwm2m_data_free( size, dataP );
        if ( buffer != NULL ) nbiot_free( buffer );
    }
}