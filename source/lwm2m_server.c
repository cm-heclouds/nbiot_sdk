/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "m2m.h"

typedef struct server_t
{
    struct server_t *next;     /* matches lwm2m_list_t::next */
    uint16_t         instid;   /* matches lwm2m_list_t::id */
    uint16_t         id;
    uint32_t         lifetime;
    bool             storing;
    char             binding[4];
}server_t;

static uint8_t prv_get_value( lwm2m_data_t      *data,
                              server_t *svr )
{
    switch ( data->id )
    {
        case LWM2M_SERVER_SHORT_ID_ID:
            lwm2m_data_encode_int( svr->id, data );
            return COAP_205_CONTENT;

        case LWM2M_SERVER_LIFETIME_ID:
            lwm2m_data_encode_int( svr->lifetime, data );
            return COAP_205_CONTENT;

        case LWM2M_SERVER_STORING_ID:
            lwm2m_data_encode_bool( svr->storing, data );
            return COAP_205_CONTENT;

        case LWM2M_SERVER_BINDING_ID:
            lwm2m_data_encode_string( svr->binding, data );
            return COAP_205_CONTENT;

        case LWM2M_SERVER_DISABLE_ID:
        case LWM2M_SERVER_UPDATE_ID:
            return COAP_405_METHOD_NOT_ALLOWED;

        default:
            return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_server_read( uint16_t        instid,
                                int            *num,
                                lwm2m_data_t  **data,
                                lwm2m_object_t *obj )
{
    int i;
    uint8_t ret;
    server_t *svr;

    svr = (server_t *)LWM2M_LIST_FIND(obj->instanceList,instid);
    if ( NULL == svr )
    {
        return COAP_404_NOT_FOUND;
    }

    /* is the server asking for the full instance ? */
    if ( 0 == *num )
    {
        uint16_t res[] = {
            LWM2M_SERVER_SHORT_ID_ID,
            LWM2M_SERVER_LIFETIME_ID,
            LWM2M_SERVER_STORING_ID,
            LWM2M_SERVER_BINDING_ID
        };

        i = sizeof(res) / sizeof(uint16_t);
        *data = lwm2m_data_new( i );
        if ( NULL == *data )
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        *num = i;
        for ( i = 0; i < *num; ++i )
        {
            (*data)[i].id = res[i];
        }
    }

    i = 0;
    do
    {
        ret = prv_get_value( (*data) + i, svr );
        ++i;
    }
    while ( i < *num && COAP_205_CONTENT == ret );

    return ret;
}

lwm2m_object_t*
create_server_object( uint16_t    svr_id,
                      const char *binding,
                      uint32_t    lifetime,
                      bool        storing )
{
    lwm2m_object_t *obj;
    server_t *svr;

    if ( NULL == binding )
    {
        return NULL;
    }

    obj = (lwm2m_object_t*)nbiot_malloc( sizeof(lwm2m_object_t) );
    if ( NULL == obj )
    {
        return NULL;
    }

    nbiot_memzero( obj, sizeof(lwm2m_object_t) );
    obj->objID = LWM2M_SERVER_OBJECT_ID;

    svr = (server_t*)nbiot_malloc( sizeof(server_t) );
    if ( NULL == svr )
    {
        nbiot_free( obj );

        return NULL;
    }

    nbiot_memzero( svr, sizeof(server_t) );
    svr->instid = 0;
    svr->id = svr_id;
    svr->lifetime = lifetime;
    svr->storing = storing;
    nbiot_strncpy( svr->binding, binding, -1 );
    obj->instanceList = LWM2M_LIST_ADD(obj->instanceList,svr);
    obj->readFunc = prv_server_read;

    return obj;
}

void clear_server_object( lwm2m_object_t *svr_obj )
{
    if ( NULL != svr_obj )
    {
        while ( NULL != svr_obj->instanceList )
        {
            server_t *svr;

            svr = (server_t*)svr_obj->instanceList;
            svr_obj->instanceList = svr_obj->instanceList->next;
            nbiot_free( svr );
        }
    }
}