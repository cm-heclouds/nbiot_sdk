/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  wakaama - https://github.com/eclipse/wakaama
**/

#include "internals.h"

lwm2m_list_t * lwm2m_list_add( lwm2m_list_t * head,
                               lwm2m_list_t * node )
{
    lwm2m_list_t * target;

    if ( NULL == head ) return node;

    if ( head->id > node->id )
    {
        node->next = head;
        return node;
    }

    target = head;
    while ( NULL != target->next && target->next->id < node->id )
    {
        target = target->next;
    }

    node->next = target->next;
    target->next = node;

    return head;
}


lwm2m_list_t * lwm2m_list_find( lwm2m_list_t * head,
                                uint16_t id )
{
    while ( NULL != head && head->id < id )
    {
        head = head->next;
    }

    if ( NULL != head && head->id == id ) return head;

    return NULL;
}


lwm2m_list_t * lwm2m_list_remove( lwm2m_list_t * head,
                                  uint16_t id,
                                  lwm2m_list_t ** nodeP )
{
    lwm2m_list_t * target;

    if ( head == NULL )
    {
        if ( nodeP ) *nodeP = NULL;
        return NULL;
    }

    if ( head->id == id )
    {
        if ( nodeP ) *nodeP = head;
        return head->next;
    }

    target = head;
    while ( NULL != target->next && target->next->id < id )
    {
        target = target->next;
    }

    if ( NULL != target->next && target->next->id == id )
    {
        if ( nodeP ) *nodeP = target->next;
        target->next = target->next->next;
    }
    else
    {
        if ( nodeP ) *nodeP = NULL;
    }

    return head;
}

uint16_t lwm2m_list_newId( lwm2m_list_t * head )
{
    uint16_t id;
    lwm2m_list_t * target;

    id = 0;
    target = head;

    while ( target != NULL && id == target->id )
    {
        id = target->id + 1;
        target = target->next;
    }

    return id;
}

void lwm2m_list_free( lwm2m_list_t * head )
{
    if ( head != NULL )
    {
        lwm2m_list_t * nextP;

        nextP = head->next;
        nbiot_free( head );
        lwm2m_list_free( nextP );
    }
}