/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef NBIOT_SOURCE_LIST_H_
#define NBIOT_SOURCE_LIST_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

struct list
{
    struct list *next;
};
typedef void** list_t;

#define LIST_CONCAT(s1,s2) s1##s2
#define LIST_STRUCT(name) \
        void  *LIST_CONCAT(name,_list); \
        list_t name
#define LIST_STRUCT_INIT(struct_ptr,name) \
        { \
            (struct_ptr)->name = &((struct_ptr)->LIST_CONCAT(name,_list)); \
            (struct_ptr)->LIST_CONCAT(name,_list) = NULL; \
        }

static inline void* list_head( list_t list )
{
    return *list;
}

static inline void list_remove( list_t list,
                                void  *item )
{
    struct list *_list = *(struct list**)list;
    struct list *_item =  (struct list*)item;

    if ( _list )
    {
        if ( _list == item )
        {
            _list = _list->next;
        }
        else
        {
            while ( _list->next &&
                    _list->next != _item )
            {
                _list = _list->next;
            }

            if ( _list->next )
            {
                _list->next = _item->next;
            }
        }

        *(struct list**)list = _list;
    }
}

static inline void list_add( list_t list,
                             void  *item )
{
    struct list *_list;
    struct list *_item;
    list_remove( list, item );

    _list = *(struct list**)list;
    _item =  (struct list*)item;
    if ( _list )
    {
        _item->next = _list;
        while ( _item->next->next )
        {
            _item->next = _item->next->next;
        }
        _item->next->next = _item;
    }
    else
    {
        _list = _item;
    }

    _item->next = NULL;
    *(struct list**)list = _list;
}

static inline void list_push( list_t list,
                              void  *item )
{
    struct list *_list = *(struct list**)list;
    struct list *_item = (struct list*)item;

    _item->next = _list;
    _list = _item;
    *(struct list**)list = _item;
}

static inline void* list_pop( list_t list )
{
    void *head = list_head( list );

    if ( head )
    {
        list_remove( list, head );
    }

    return head;
}

static inline void list_insert( list_t list,
                                void  *previtem,
                                void  *newitem )
{
    if ( previtem == NULL )
    {
        list_push( list, newitem );
    }
    else
    {
        ((struct list*)newitem)->next = ((struct list*)previtem)->next;
        ((struct list*)previtem)->next = newitem;
    }
}

static inline void* list_item_next( void *item )
{
    return item == NULL ? NULL : ((struct list*)item)->next;
}

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_LIST_H_ */