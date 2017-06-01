/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"

nbiot_list_t* nbiot_list_add( nbiot_list_t *head,
                              nbiot_list_t *node )
{
    nbiot_list_t *prev;

    if ( !head )
    {
        return node;
    }

    if ( head->id > node->id )
    {
        node->next = head;

        return node;
    }

    prev = head;
    while ( prev->next &&
            prev->next->id < node->id )
    {
        prev = prev->next;
    }

    node->next = prev->next;
    prev->next = node;

    return head;
}

nbiot_list_t* nbiot_list_get( nbiot_list_t *head,
                              uint16_t      id )
{
    while ( head &&
            head->id < id )
    {
        head = head->next;
    }

    if ( head &&
         head->id == id)
    {
        return head;
    }

    return NULL;
}

nbiot_list_t* nbiot_list_del( nbiot_list_t  *head,
                              uint16_t       id,
                              nbiot_list_t **node )
{
    nbiot_list_t *prev;

    if ( !head )
    {
        if ( node )
        {
            *node = NULL;
        }

        return NULL;
    }

    if ( head->id == id )
    {
        if ( node )
        {
            *node = head;
        }

        return head->next;
    }

    prev = head;
    while ( prev->next &&
            prev->next->id < id )
    {
        prev = prev->next;
    }

    if ( prev->next &&
         prev->next->id == id )
    {
        if ( node )
        {
            *node = prev->next;
        }

        prev->next = prev->next->next;
    }
    else if ( node )
    {
        *node = NULL;
    }

    return head;
}

void nbiot_list_free( nbiot_list_t *head )
{
    while ( head )
    {
        nbiot_list_t *temp;

        temp = head;
        head = head->next;
        nbiot_free( temp );
    }
}