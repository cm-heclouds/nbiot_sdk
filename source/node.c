/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

int nbiot_node_read( nbiot_node_t *node,
                     uint8_t       flag,
                     uint8_t       src_flag,
                     uint8_t      *buffer,
                     size_t        buffer_len,
                     bool          updated )
{
    if ( flag & NBIOT_SET_RESID )
    {
        nbiot_value_t *data = (nbiot_value_t*)node->data;
        if ( !updated || (data->flag&NBIOT_UPDATED) )
        {
            uint8_t array[8];
            uint8_t *value = NULL;
            size_t value_len = 0;

            if ( data->type == NBIOT_BOOLEAN )
            {
                array[0] = data->value.as_bool ? 1 : 0;
                value_len = 1;
                value = array;
            }

            if ( data->type == NBIOT_INTEGER )
            {
                int64_t tmp = data->value.as_int;
                uint8_t idx = sizeof(array);

                while ( tmp )
                {
                    array[--idx] = (uint8_t)(tmp & 0xff);
                    tmp >>= 8;
                }

                value = array + idx;
                value_len = sizeof(array) - idx;
            }

            if ( data->type == NBIOT_FLOAT )
            {
                int64_t tmp;
                uint8_t idx;

                if ( data->value.as_float < -(double)FLT_MAX ||
                     data->value.as_float >  (double)FLT_MAX )
                {
                    tmp = *(int64_t*)(&data->value.as_float);
                    value_len = 8;
                }
                else
                {
                    float f = (float)data->value.as_float;
                    tmp = *(uint32_t*)(&f);
                    value_len = 4;
                }

                idx = value_len;
                while ( idx )
                {
                    array[--idx] = (uint8_t)(tmp & 0xff);
                    tmp >>= 8;
                }
                value = array;
            }

            if ( data->type == NBIOT_STRING ||
                 data->type == NBIOT_BINARY )
            {
                value = (uint8_t*)data->value.as_buf.val;
                value_len = data->value.as_buf.len;
            }

            if ( !value || value_len > buffer_len )
            {
                return 0;
            }

            if ( (src_flag&NBIOT_SET_RESID) &&
                 nbiot_tlv_length(0,value_len) > buffer_len )
            {
                return 0;
            }

            if ( buffer )
            {
                if ( updated )
                {
                    data->flag &= ~NBIOT_UPDATED;
                }

                if ( src_flag & NBIOT_SET_RESID )
                {
                    value_len = nbiot_tlv_encode( buffer,
                                                  buffer_len,
                                                  TLV_TYPE_RESOURCE_INSTANCE,
                                                  0,
                                                  value,
                                                  value_len );
                }
                else
                {
                    nbiot_memmove( buffer,
                                   value,
                                   value_len );
                }
            }

            return value_len;
        }
    }
    else if ( flag & NBIOT_SET_OBJID )
    {
        int len;
        int ret = 0;
        int num = 0;
        int tmp = 0;
        uint8_t type;

        if ( flag & NBIOT_SET_INSTID )
        {
            flag |= NBIOT_SET_RESID;
            type = TLV_TYPE_RESOURCE;
        }
        else
        {
            flag |= NBIOT_SET_INSTID;
            type = TLV_TYPE_OBJECT_INSTANCE;
        }

        for ( node = (nbiot_node_t*)node->data;
              node != NULL;
              node = node->next )
        {
            len = nbiot_node_read( node,
                                   flag,
                                   src_flag,
                                   NULL,
                                   SIZE_MAX,
                                   updated );

            tmp = nbiot_tlv_length( node->id, len );
            if ( tmp <= buffer_len )
            {
                if ( buffer )
                {
                    nbiot_tlv_encode( buffer + num,
                                      buffer_len - num,
                                      type,
                                      node->id,
                                      NULL,
                                      len );
                    nbiot_node_read( node,
                                     flag,
                                     src_flag,
                                     buffer + num + ret,
                                     buffer_len - num - ret,
                                     updated );
                }

                num += tmp;
            }
        }

        return num;
    }

    return 0;
}

nbiot_node_t* nbiot_node_find( nbiot_device_t    *dev,
                               const nbiot_uri_t *uri )
{
    nbiot_node_t *node = NULL;

    if ( uri->flag & NBIOT_SET_OBJID )
    {
        node = (nbiot_node_t*)NBIOT_LIST_GET( dev->nodes, uri->objid );
        if ( node && (uri->flag&NBIOT_SET_INSTID) )
        {
            node = (nbiot_node_t*)NBIOT_LIST_GET( node->data, uri->instid );
            if ( node && (uri->flag&NBIOT_SET_RESID) )
            {
                node = (nbiot_node_t*)NBIOT_LIST_GET( node->data, uri->resid );
            }
        }
    }

    return node;
}