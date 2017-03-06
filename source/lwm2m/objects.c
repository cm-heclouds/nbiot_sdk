/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

uint8_t object_checkReadable( lwm2m_context_t * contextP,
                              lwm2m_uri_t * uriP )
{
    coap_status_t result;
    lwm2m_object_t * targetP;
    lwm2m_data_t * dataP = NULL;
    int size;

    LOG_URI( uriP );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;
    if ( NULL == targetP->readFunc ) return COAP_405_METHOD_NOT_ALLOWED;

    if ( !LWM2M_URI_IS_SET_INSTANCE( uriP ) ) return COAP_205_CONTENT;

    if ( NULL == lwm2m_list_find( targetP->instanceList, uriP->instanceId ) ) return COAP_404_NOT_FOUND;

    if ( !LWM2M_URI_IS_SET_RESOURCE( uriP ) ) return COAP_205_CONTENT;

    size = 1;
    dataP = lwm2m_data_new( 1 );
    if ( dataP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

    dataP->id = uriP->resourceId;

    result = targetP->readFunc( uriP->instanceId, &size, &dataP, targetP );
    lwm2m_data_free( 1, dataP );

    return result;
}

uint8_t object_checkNumeric( lwm2m_context_t * contextP,
                             lwm2m_uri_t * uriP )
{
    coap_status_t result;
    lwm2m_object_t * targetP;
    lwm2m_data_t * dataP = NULL;
    int size;

    LOG_URI( uriP );
    if ( !LWM2M_URI_IS_SET_RESOURCE( uriP ) ) return COAP_405_METHOD_NOT_ALLOWED;

    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;
    if ( NULL == targetP->readFunc ) return COAP_405_METHOD_NOT_ALLOWED;

    size = 1;
    dataP = lwm2m_data_new( 1 );
    if ( dataP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

    dataP->id = uriP->resourceId;

    result = targetP->readFunc( uriP->instanceId, &size, &dataP, targetP );
    if ( result == COAP_205_CONTENT )
    {
        switch ( dataP->type )
        {
            case LWM2M_TYPE_INTEGER:
            case LWM2M_TYPE_FLOAT:
            break;
            default:
            result = COAP_405_METHOD_NOT_ALLOWED;
        }
    }

    lwm2m_data_free( 1, dataP );

    return result;
}

coap_status_t object_readData( lwm2m_context_t * contextP,
                               lwm2m_uri_t * uriP,
                               int * sizeP,
                               lwm2m_data_t ** dataP )
{
    coap_status_t result;
    lwm2m_object_t * targetP;

    LOG_URI( uriP );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;
    if ( NULL == targetP->readFunc ) return COAP_405_METHOD_NOT_ALLOWED;
    if ( targetP->instanceList == NULL ) return COAP_404_NOT_FOUND;

    if ( LWM2M_URI_IS_SET_INSTANCE( uriP ) )
    {
        if ( NULL == lwm2m_list_find( targetP->instanceList, uriP->instanceId ) ) return COAP_404_NOT_FOUND;

        /* single instance read */
        if ( LWM2M_URI_IS_SET_RESOURCE( uriP ) )
        {
            *sizeP = 1;
            *dataP = lwm2m_data_new( *sizeP );
            if ( *dataP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

            (*dataP)->id = uriP->resourceId;
        }

        result = targetP->readFunc( uriP->instanceId, sizeP, dataP, targetP );
    }
    else
    {
        /* multiple object instances read */
        lwm2m_list_t * instanceP;
        int i;

        *sizeP = 0;
        for ( instanceP = targetP->instanceList; instanceP != NULL; instanceP = instanceP->next )
        {
            (*sizeP)++;
        }

        *dataP = lwm2m_data_new( *sizeP );
        if ( *dataP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

        result = COAP_205_CONTENT;
        instanceP = targetP->instanceList;
        i = 0;
        while ( instanceP != NULL && result == COAP_205_CONTENT )
        {
            result = targetP->readFunc( instanceP->id, (int*)&((*dataP)[i].value.asChildren.count), &((*dataP)[i].value.asChildren.array), targetP );
            (*dataP)[i].type = LWM2M_TYPE_OBJECT_INSTANCE;
            (*dataP)[i].id = instanceP->id;
            i++;
            instanceP = instanceP->next;
        }
    }

    LOG_ARG( "result: %u.%2u, size: %d", (result & 0xFF) >> 5, (result & 0x1F), *sizeP );
    return result;
}

coap_status_t object_read( lwm2m_context_t * contextP,
                           lwm2m_uri_t * uriP,
                           lwm2m_media_type_t * formatP,
                           uint8_t ** bufferP,
                           size_t * lengthP )
{
    coap_status_t result;
    lwm2m_data_t * dataP = NULL;
    int size = 0;

    LOG_URI( uriP );
    result = object_readData( contextP, uriP, &size, &dataP );

    if ( result == COAP_205_CONTENT )
    {
        *lengthP = lwm2m_data_serialize( uriP, size, dataP, formatP, bufferP );
        if ( *lengthP == 0 )
        {
            if ( *formatP != LWM2M_CONTENT_TEXT
                 || size != 1
                 || dataP->type != LWM2M_TYPE_STRING
                 || dataP->value.asBuffer.length != 0 )
            {
                result = COAP_500_INTERNAL_SERVER_ERROR;
            }
        }
    }
    lwm2m_data_free( size, dataP );

    LOG_ARG( "result: %u.%2u, length: %d", (result & 0xFF) >> 5, (result & 0x1F), *lengthP );

    return result;
}

coap_status_t object_write( lwm2m_context_t * contextP,
                            lwm2m_uri_t * uriP,
                            lwm2m_media_type_t format,
                            uint8_t * buffer,
                            size_t length )
{
    coap_status_t result = NO_ERROR;
    lwm2m_object_t * targetP;
    lwm2m_data_t * dataP = NULL;
    int size = 0;

    LOG_URI( uriP );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP )
    {
        result = COAP_404_NOT_FOUND;
    }
    else if ( NULL == targetP->writeFunc )
    {
        result = COAP_405_METHOD_NOT_ALLOWED;
    }
    else
    {
        size = lwm2m_data_parse( uriP, buffer, length, format, &dataP );
        if ( size == 0 )
        {
            result = COAP_406_NOT_ACCEPTABLE;
        }
    }
    if ( result == NO_ERROR )
    {
        result = targetP->writeFunc( uriP->instanceId, size, dataP, targetP );
        lwm2m_data_free( size, dataP );
    }

    LOG_ARG( "result: %u.%2u", (result & 0xFF) >> 5, (result & 0x1F) );

    return result;
}

coap_status_t object_execute( lwm2m_context_t * contextP,
                              lwm2m_uri_t * uriP,
                              uint8_t * buffer,
                              size_t length )
{
    lwm2m_object_t * targetP;

    LOG_URI( uriP );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;
    if ( NULL == targetP->executeFunc ) return COAP_405_METHOD_NOT_ALLOWED;

    return targetP->executeFunc( uriP->instanceId, uriP->resourceId, buffer, length, targetP );
}

coap_status_t object_create( lwm2m_context_t * contextP,
                             lwm2m_uri_t * uriP,
                             lwm2m_media_type_t format,
                             uint8_t * buffer,
                             size_t length )
{
    lwm2m_object_t * targetP;
    lwm2m_data_t * dataP = NULL;
    int size = 0;
    uint8_t result;

    LOG_URI( uriP );
    if ( length == 0 || buffer == 0 )
    {
        return COAP_400_BAD_REQUEST;
    }

    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;
    if ( NULL == targetP->createFunc ) return COAP_405_METHOD_NOT_ALLOWED;

    size = lwm2m_data_parse( uriP, buffer, length, format, &dataP );
    if ( size <= 0 ) return COAP_400_BAD_REQUEST;

    switch ( dataP[0].type )
    {
        case LWM2M_TYPE_OBJECT:
        result = COAP_400_BAD_REQUEST;
        goto exit;

        case LWM2M_TYPE_OBJECT_INSTANCE:
        if ( size != 1 )
        {
            result = COAP_400_BAD_REQUEST;
            goto exit;
        }
        if ( NULL != lwm2m_list_find( targetP->instanceList, dataP[0].id ) )
        {
            /* Instance already exists */
            result = COAP_406_NOT_ACCEPTABLE;
            goto exit;
        }
        result = targetP->createFunc( dataP[0].id, dataP[0].value.asChildren.count, dataP[0].value.asChildren.array, targetP );
        uriP->instanceId = dataP[0].id;
        uriP->flag |= LWM2M_URI_FLAG_INSTANCE_ID;
        break;

        default:
        uriP->instanceId = lwm2m_list_newId( targetP->instanceList );
        uriP->flag |= LWM2M_URI_FLAG_INSTANCE_ID;
        result = targetP->createFunc( uriP->instanceId, size, dataP, targetP );
        break;
    }

exit:
    lwm2m_data_free( size, dataP );

    LOG_ARG( "result: %u.%2u", (result & 0xFF) >> 5, (result & 0x1F) );

    return result;
}

coap_status_t object_delete( lwm2m_context_t * contextP,
                             lwm2m_uri_t * uriP )
{
    lwm2m_object_t * objectP;
    coap_status_t result;

    LOG_URI( uriP );
    objectP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == objectP ) return COAP_404_NOT_FOUND;
    if ( NULL == objectP->deleteFunc ) return COAP_405_METHOD_NOT_ALLOWED;

    LOG( "Entering" );

    if ( LWM2M_URI_IS_SET_INSTANCE( uriP ) )
    {
        result = objectP->deleteFunc( uriP->instanceId, objectP );
    }
    else
    {
        lwm2m_list_t * instanceP;

        result = COAP_202_DELETED;
        instanceP = objectP->instanceList;
        while ( NULL != instanceP
                && result == COAP_202_DELETED )
        {
            result = objectP->deleteFunc( instanceP->id, objectP );
            instanceP = objectP->instanceList;
        }
    }

    LOG_ARG( "result: %u.%2u", (result & 0xFF) >> 5, (result & 0x1F) );

    return result;
}

coap_status_t object_discover( lwm2m_context_t * contextP,
                               lwm2m_uri_t * uriP,
                               uint8_t ** bufferP,
                               size_t * lengthP )
{
    coap_status_t result;
    lwm2m_object_t * targetP;
    lwm2m_data_t * dataP = NULL;
    int size = 0;

    LOG_URI( uriP );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;
    if ( NULL == targetP->discoverFunc ) return COAP_501_NOT_IMPLEMENTED;
    if ( targetP->instanceList == NULL ) return COAP_404_NOT_FOUND;

    if ( LWM2M_URI_IS_SET_INSTANCE( uriP ) )
    {
        if ( NULL == lwm2m_list_find( targetP->instanceList, uriP->instanceId ) ) return COAP_404_NOT_FOUND;

        /* single instance read */
        if ( LWM2M_URI_IS_SET_RESOURCE( uriP ) )
        {
            size = 1;
            dataP = lwm2m_data_new( size );
            if ( dataP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

            dataP->id = uriP->resourceId;
            uriP->flag &= ~LWM2M_URI_FLAG_RESOURCE_ID;
        }

        result = targetP->discoverFunc( uriP->instanceId, &size, &dataP, targetP );
    }
    else
    {
        /* multiple object instances read */
        lwm2m_list_t * instanceP;
        int i;

        size = 0;
        for ( instanceP = targetP->instanceList; instanceP != NULL; instanceP = instanceP->next )
        {
            size++;
        }

        dataP = lwm2m_data_new( size );
        if ( dataP == NULL ) return COAP_500_INTERNAL_SERVER_ERROR;

        result = COAP_205_CONTENT;
        instanceP = targetP->instanceList;
        i = 0;
        while ( instanceP != NULL && result == COAP_205_CONTENT )
        {
            result = targetP->discoverFunc( instanceP->id, (int*)&(dataP[i].value.asChildren.count), &(dataP[i].value.asChildren.array), targetP );
            dataP[i].type = LWM2M_TYPE_OBJECT_INSTANCE;
            dataP[i].id = instanceP->id;
            i++;
            instanceP = instanceP->next;
        }
    }

    if ( result == COAP_205_CONTENT )
    {
        int len;

        len = discover_serialize( contextP, uriP, size, dataP, bufferP );
        if ( len <= 0 ) result = COAP_500_INTERNAL_SERVER_ERROR;
        else *lengthP = len;
    }
    lwm2m_data_free( size, dataP );

    LOG_ARG( "result: %u.%2u", (result & 0xFF) >> 5, (result & 0x1F) );

    return result;
}

bool object_isInstanceNew( lwm2m_context_t * contextP,
                           uint16_t objectId,
                           uint16_t instanceId )
{
    lwm2m_object_t * targetP;

    LOG( "Entering" );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, objectId );
    if ( targetP != NULL )
    {
        if ( NULL != lwm2m_list_find( targetP->instanceList, instanceId ) )
        {
            return false;
        }
    }

    return true;
}

static int prv_getObjectTemplate( uint8_t * buffer,
                                  size_t length,
                                  uint16_t id )
{
    int index;
    int result;

    if ( length < REG_OBJECT_MIN_LEN ) return -1;

    buffer[0] = '<';
    buffer[1] = '/';
    index = 2;

    result = utils_intCopy( (char *)buffer + index, length - index, id );
    if ( result < 0 ) return -1;
    index += result;

    if ( length - index < REG_OBJECT_MIN_LEN - 3 ) return -1;
    buffer[index] = '/';
    index++;

    return index;
}

int object_getRegisterPayload( lwm2m_context_t * contextP,
                               uint8_t * buffer,
                               size_t bufferLen )
{
    size_t index;
    int result;
    lwm2m_object_t * objectP;

    LOG( "Entering" );
    /* index can not be greater than bufferLen */
    index = 0;

    result = utils_stringCopy( (char *)buffer, bufferLen, REG_START );
    if ( result < 0 ) return 0;
    index += result;

    /* altPath */
    result = utils_stringCopy( (char *)buffer + index, bufferLen - index, REG_DEFAULT_PATH );
    if ( result < 0 ) return 0;
    index += result;

    result = utils_stringCopy( (char *)buffer + index, bufferLen - index, REG_LWM2M_RESOURCE_TYPE );
    if ( result < 0 ) return 0;
    index += result;

    for ( objectP = contextP->objectList; objectP != NULL; objectP = objectP->next )
    {
        size_t start;
        size_t length;

        if ( objectP->objID == LWM2M_SECURITY_OBJECT_ID ) continue;

        start = index;
        result = prv_getObjectTemplate( buffer + index, bufferLen - index, objectP->objID );
        if ( result < 0 ) return 0;
        length = result;
        index += length;

        if ( objectP->instanceList == NULL )
        {
            index--;
            result = utils_stringCopy( (char *)buffer + index, bufferLen - index, REG_PATH_END );
            if ( result < 0 ) return 0;
            index += result;
        }
        else
        {
            lwm2m_list_t * targetP;
            for ( targetP = objectP->instanceList; targetP != NULL; targetP = targetP->next )
            {
                if ( bufferLen - index <= length ) return 0;

                if ( index != start + length )
                {
                    nbiot_memmove( buffer + index, buffer + start, length );
                    index += length;
                }

                result = utils_intCopy( (char *)buffer + index, bufferLen - index, targetP->id );
                if ( result < 0 ) return 0;
                index += result;

                result = utils_stringCopy( (char *)buffer + index, bufferLen - index, REG_PATH_END );
                if ( result < 0 ) return 0;
                index += result;
            }
        }
    }

    if ( index > 0 )
    {
        index = index - 1;  /* remove trailing ',' */
    }

    buffer[index] = 0;

    return index;
}

int object_getServers( lwm2m_context_t *contextP )
{
    lwm2m_server_t *targetP;
    lwm2m_userdata_t *userData;

    LOG( "Entering" );
    if ( NULL == contextP->userData )
    {
        return -1;
    }

    userData = (lwm2m_userdata_t*)contextP->userData;
    if ( userData->uri == NULL ||
         LWM2M_UINT32(userData->lifetime) == 0 )
    {
        return -1;
    }

    targetP = (lwm2m_server_t *)nbiot_malloc( sizeof(lwm2m_server_t) );
    if ( targetP == NULL )
    {
        return -1;
    }

    nbiot_memzero( targetP, sizeof(lwm2m_server_t) );
    targetP->binding = BINDING_U; /* 目前只支持UDP */
    targetP->lifetime = LWM2M_UINT32(userData->lifetime);
    if ( userData->flag & LWM2M_USERDATA_BOOTSTRAP )
    {
        contextP->bootstrapServerList = (lwm2m_server_t*)LWM2M_LIST_ADD( contextP->bootstrapServerList, targetP );
    }
    else
    {
        targetP->status = STATE_DEREGISTERED;
        contextP->serverList = (lwm2m_server_t*)LWM2M_LIST_ADD( contextP->serverList, targetP );
    }

    return 0;
}

coap_status_t object_createInstance( lwm2m_context_t * contextP,
                                     lwm2m_uri_t * uriP,
                                     lwm2m_data_t * dataP )
{
    lwm2m_object_t * targetP;

    LOG_URI( uriP );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;

    if ( NULL == targetP->createFunc )
    {
        return COAP_405_METHOD_NOT_ALLOWED;
    }

    return targetP->createFunc( lwm2m_list_newId( targetP->instanceList ), dataP->value.asChildren.count, dataP->value.asChildren.array, targetP );
}

coap_status_t object_writeInstance( lwm2m_context_t * contextP,
                                    lwm2m_uri_t * uriP,
                                    lwm2m_data_t * dataP )
{
    lwm2m_object_t * targetP;

    LOG_URI( uriP );
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND( contextP->objectList, uriP->objectId );
    if ( NULL == targetP ) return COAP_404_NOT_FOUND;

    if ( NULL == targetP->writeFunc )
    {
        return COAP_405_METHOD_NOT_ALLOWED;
    }

    return targetP->writeFunc( dataP->id, dataP->value.asChildren.count, dataP->value.asChildren.array, targetP );
}