/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

static int prv_readAttributes( multi_option_t * query,
                               lwm2m_attributes_t * attrP )
{
    int64_t intValue;
    double floatValue;

    nbiot_memzero( attrP, sizeof(lwm2m_attributes_t) );

    while ( query != NULL )
    {
        if ( nbiot_strncmp( (char *)query->data, ATTR_MIN_PERIOD_STR, ATTR_MIN_PERIOD_LEN ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_MIN_PERIOD) ) return -1;
            if ( query->len == ATTR_MIN_PERIOD_LEN ) return -1;

            if ( 1 != utils_plainTextToInt64( query->data + ATTR_MIN_PERIOD_LEN, query->len - ATTR_MIN_PERIOD_LEN, &intValue ) ) return -1;
            if ( intValue < 0 ) return -1;

            attrP->toSet |= LWM2M_ATTR_FLAG_MIN_PERIOD;
            attrP->minPeriod = (uint32_t)intValue;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_MIN_PERIOD_STR, ATTR_MIN_PERIOD_LEN - 1 ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_MIN_PERIOD) ) return -1;
            if ( query->len != ATTR_MIN_PERIOD_LEN - 1 ) return -1;

            attrP->toClear |= LWM2M_ATTR_FLAG_MIN_PERIOD;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_MAX_PERIOD_STR, ATTR_MAX_PERIOD_LEN ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_MAX_PERIOD) ) return -1;
            if ( query->len == ATTR_MAX_PERIOD_LEN ) return -1;

            if ( 1 != utils_plainTextToInt64( query->data + ATTR_MAX_PERIOD_LEN, query->len - ATTR_MAX_PERIOD_LEN, &intValue ) ) return -1;
            if ( intValue < 0 ) return -1;

            attrP->toSet |= LWM2M_ATTR_FLAG_MAX_PERIOD;
            attrP->maxPeriod = (uint32_t)intValue;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_MAX_PERIOD_STR, ATTR_MAX_PERIOD_LEN - 1 ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_MAX_PERIOD) ) return -1;
            if ( query->len != ATTR_MAX_PERIOD_LEN - 1 ) return -1;

            attrP->toClear |= LWM2M_ATTR_FLAG_MAX_PERIOD;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_GREATER_THAN_STR, ATTR_GREATER_THAN_LEN ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_GREATER_THAN) ) return -1;
            if ( query->len == ATTR_GREATER_THAN_LEN ) return -1;

            if ( 1 != utils_plainTextToFloat64( query->data + ATTR_GREATER_THAN_LEN, query->len - ATTR_GREATER_THAN_LEN, &floatValue ) ) return -1;

            attrP->toSet |= LWM2M_ATTR_FLAG_GREATER_THAN;
            attrP->greaterThan = floatValue;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_GREATER_THAN_STR, ATTR_GREATER_THAN_LEN - 1 ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_GREATER_THAN) ) return -1;
            if ( query->len != ATTR_GREATER_THAN_LEN - 1 ) return -1;

            attrP->toClear |= LWM2M_ATTR_FLAG_GREATER_THAN;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_LESS_THAN_STR, ATTR_LESS_THAN_LEN ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_LESS_THAN) ) return -1;
            if ( query->len == ATTR_LESS_THAN_LEN ) return -1;

            if ( 1 != utils_plainTextToFloat64( query->data + ATTR_LESS_THAN_LEN, query->len - ATTR_LESS_THAN_LEN, &floatValue ) ) return -1;

            attrP->toSet |= LWM2M_ATTR_FLAG_LESS_THAN;
            attrP->lessThan = floatValue;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_LESS_THAN_STR, ATTR_LESS_THAN_LEN - 1 ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_LESS_THAN) ) return -1;
            if ( query->len != ATTR_LESS_THAN_LEN - 1 ) return -1;

            attrP->toClear |= LWM2M_ATTR_FLAG_LESS_THAN;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_STEP_STR, ATTR_STEP_LEN ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_STEP) ) return -1;
            if ( query->len == ATTR_STEP_LEN ) return -1;

            if ( 1 != utils_plainTextToFloat64( query->data + ATTR_STEP_LEN, query->len - ATTR_STEP_LEN, &floatValue ) ) return -1;
            if ( floatValue < 0 ) return -1;

            attrP->toSet |= LWM2M_ATTR_FLAG_STEP;
            attrP->step = floatValue;
        }
        else if ( nbiot_strncmp( (char *)query->data, ATTR_STEP_STR, ATTR_STEP_LEN - 1 ) == 0 )
        {
            if ( 0 != ((attrP->toSet | attrP->toClear) & LWM2M_ATTR_FLAG_STEP) ) return -1;
            if ( query->len != ATTR_STEP_LEN - 1 ) return -1;

            attrP->toClear |= LWM2M_ATTR_FLAG_STEP;
        }
        else return -1;

        query = query->next;
    }

    return 0;
}

coap_status_t dm_handleRequest( lwm2m_context_t * contextP,
                                lwm2m_uri_t * uriP,
                                lwm2m_server_t * serverP,
                                coap_packet_t * message,
                                coap_packet_t * response )
{
    coap_status_t result;
    lwm2m_media_type_t format;

    LOG_ARG( "Code: %02X, server status: %s", message->code, STR_STATUS( serverP->status ) );
    LOG_URI( uriP );
    format = utils_convertMediaType( message->content_type );

    if ( uriP->objectId == LWM2M_SECURITY_OBJECT_ID )
    {
        return COAP_404_NOT_FOUND;
    }

    if ( serverP->status != STATE_REGISTERED
         && serverP->status != STATE_REG_UPDATE_NEEDED
         && serverP->status != STATE_REG_UPDATE_PENDING )
    {
        return COAP_IGNORE;
    }

    /* TODO: check ACL */

    switch ( message->code )
    {
        case COAP_GET:
        {
            uint8_t * buffer = NULL;
            size_t length = 0;

            if ( IS_OPTION( message, COAP_OPTION_OBSERVE ) )
            {
                lwm2m_data_t * dataP = NULL;
                int size = 0;

                result = object_readData( contextP, uriP, &size, &dataP );
                if ( COAP_205_CONTENT == result )
                {
                    result = observe_handleRequest( contextP, uriP, serverP, size, dataP, message, response );
                    if ( COAP_205_CONTENT == result )
                    {
                        length = lwm2m_data_serialize( uriP, size, dataP, &format, &buffer );
                        if ( length == 0 )
                        {
                            result = COAP_500_INTERNAL_SERVER_ERROR;
                        }
                        else
                        {
                            LOG_ARG( "Observe Request[/%d/%d/%d]: %.*s\n", uriP->objectId, uriP->instanceId, uriP->resourceId, length, buffer );
                        }
                    }
                    lwm2m_data_free( size, dataP );
                }
            }
            else if ( IS_OPTION( message, COAP_OPTION_ACCEPT )
                      && message->accept_num == 1
                      && message->accept[0] == APPLICATION_LINK_FORMAT )
            {
                format = LWM2M_CONTENT_LINK;
                result = object_discover( contextP, uriP, &buffer, &length );
            }
            else
            {
                result = object_read( contextP, uriP, &format, &buffer, &length );
            }
            if ( COAP_205_CONTENT == result )
            {
                coap_set_header_content_type( response, format );
                coap_set_payload( response, buffer, length );
                /* lwm2m_handle_packet will free buffer */
            }
            else
            {
                nbiot_free( buffer );
            }
        }
        break;

        case COAP_POST:
        {
            if ( !LWM2M_URI_IS_SET_INSTANCE( uriP ) )
            {
                result = object_create( contextP, uriP, format, message->payload, message->payload_len );
                if ( result == COAP_201_CREATED )
                {
                    /* longest uri is /65535/65535 = 12 + 1 (null) chars */
                    char location_path[13] = "";
                    /* instanceId expected */
                    if ( (uriP->flag & LWM2M_URI_FLAG_INSTANCE_ID) == 0 )
                    {
                        result = COAP_500_INTERNAL_SERVER_ERROR;
                        break;
                    }

                    if ( nbiot_sprintf( location_path, "/%d/%d", uriP->objectId, uriP->instanceId ) < 0 )
                    {
                        result = COAP_500_INTERNAL_SERVER_ERROR;
                        break;
                    }
                    coap_set_header_location_path( response, location_path );

                    lwm2m_update_registration( contextP, 0, true );
                }
            }
            else if ( !LWM2M_URI_IS_SET_RESOURCE( uriP ) )
            {
                result = object_write( contextP, uriP, format, message->payload, message->payload_len );
            }
            else
            {
                result = object_execute( contextP, uriP, message->payload, message->payload_len );
            }
        }
        break;

        case COAP_PUT:
        {
            if ( IS_OPTION( message, COAP_OPTION_URI_QUERY ) )
            {
                lwm2m_attributes_t attr;

                if ( 0 != prv_readAttributes( message->uri_query, &attr ) )
                {
                    result = COAP_400_BAD_REQUEST;
                }
                else
                {
                    result = observe_setParameters( contextP, uriP, serverP, &attr );
                }
            }
            else if ( LWM2M_URI_IS_SET_INSTANCE( uriP ) )
            {
                result = object_write( contextP, uriP, format, message->payload, message->payload_len );
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
        }
        break;

        case COAP_DELETE:
        {
            if ( !LWM2M_URI_IS_SET_INSTANCE( uriP ) || LWM2M_URI_IS_SET_RESOURCE( uriP ) )
            {
                result = COAP_400_BAD_REQUEST;
            }
            else
            {
                result = object_delete( contextP, uriP );
                if ( result == COAP_202_DELETED )
                {
                    lwm2m_update_registration( contextP, 0, true );
                }
            }
        }
        break;

        default:
        result = COAP_400_BAD_REQUEST;
        break;
    }

    return result;
}