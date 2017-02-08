/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "lwm2m.h"

typedef struct security_t
{
    struct security_t *next;   /* matches lwm2m_list_t::next */
    uint16_t                    instid; /* matches lwm2m_list_t::id */
    char                       *uri;
    uint16_t                    id;
    uint32_t                    holdoff_time;
    bool                        bootstrap;
}security_t;

static uint8_t prv_get_value( lwm2m_data_t        *data,
                              security_t *sec )
{
    switch ( data->id )
    {
        case LWM2M_SECURITY_URI_ID:
            lwm2m_data_encode_string( sec->uri, data );
            return COAP_205_CONTENT;

        case LWM2M_SECURITY_BOOTSTRAP_ID:
            lwm2m_data_encode_bool( sec->bootstrap, data );
            return COAP_205_CONTENT;

        case LWM2M_SECURITY_SECURITY_ID:
            lwm2m_data_encode_int( LWM2M_SECURITY_MODE_NONE, data );
            return COAP_205_CONTENT;

        case LWM2M_SECURITY_SMS_SECURITY_ID:
            lwm2m_data_encode_int( LWM2M_SECURITY_MODE_NONE, data );
            return COAP_205_CONTENT;

        case LWM2M_SECURITY_SHORT_SERVER_ID:
            lwm2m_data_encode_int( sec->id, data );
            return COAP_205_CONTENT;

        case LWM2M_SECURITY_HOLD_OFF_ID:
            lwm2m_data_encode_int( sec->holdoff_time, data );
            return COAP_205_CONTENT;

        default:
            return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_security_read( uint16_t        instid,
                                  int            *num,
                                  lwm2m_data_t  **data,
                                  lwm2m_object_t *obj )
{
    int i;
    uint8_t ret;
    security_t *sec;

    sec = (security_t*)LWM2M_LIST_FIND(obj->instanceList,instid);
    if ( NULL == sec )
    {
        return COAP_404_NOT_FOUND;
    }

    /* is the server asking for the full instance ? */
    if ( 0 == *num )
    {
        uint16_t res[] = {LWM2M_SECURITY_URI_ID,
                          LWM2M_SECURITY_BOOTSTRAP_ID,
                          LWM2M_SECURITY_SECURITY_ID,
                          LWM2M_SECURITY_SMS_SECURITY_ID,
                          LWM2M_SECURITY_SHORT_SERVER_ID,
                          LWM2M_SECURITY_HOLD_OFF_ID};

        i = sizeof(res)/sizeof(uint16_t);
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
        ret = prv_get_value( (*data)+i, sec );
        ++i;
    } while ( i < *num && COAP_205_CONTENT == ret );

    return ret;
}

lwm2m_object_t*
create_security_object( uint16_t    svr_id,
                        const char *svr_uri,
                        uint32_t    holdoff_time,
                        bool        bootstrap )
{
    lwm2m_object_t *obj;
    security_t *sec;

    if ( NULL == svr_uri )
    {
        return NULL;
    }

    obj = (lwm2m_object_t*)lwm2m_malloc( sizeof(lwm2m_object_t) );
    if ( NULL == obj )
    {
        return NULL;
    }

    nbiot_memzero( obj, sizeof(lwm2m_object_t) );
    obj->objID = LWM2M_SECURITY_OBJECT_ID;

    sec = (security_t*)lwm2m_malloc( sizeof(security_t) );
    if ( NULL == sec )
    {
        lwm2m_free( obj );

        return NULL;
    }

    nbiot_memzero( sec, sizeof(security_t) );
    sec->instid = 0;
    sec->uri = lwm2m_strdup( svr_uri );
    sec->id = svr_id;
    sec->holdoff_time = holdoff_time;
    sec->bootstrap = bootstrap;
    obj->instanceList = LWM2M_LIST_ADD(obj->instanceList,sec);
    obj->readFunc = prv_security_read; /* 只读 */

    return obj;
}

void clear_security_object( lwm2m_object_t *sec_obj )
{
    if ( NULL != sec_obj )
    {
        while ( NULL != sec_obj->instanceList )
        {
            security_t *sec;

            sec = (security_t*)sec_obj->instanceList;
            sec_obj->instanceList = sec_obj->instanceList->next;

            if ( NULL != sec->uri )
            {
                lwm2m_free( sec->uri );
            }

            lwm2m_free( sec );
        }
    }
}

char* get_server_uri( lwm2m_object_t *sec_obj,
                      uint16_t        sec_instid )
{
    security_t *sec;

    sec = (security_t*)LWM2M_LIST_FIND(sec_obj->instanceList,sec_instid);
    if ( NULL != sec )
    {
        return lwm2m_strdup( sec->uri );
    }

    return NULL;
}