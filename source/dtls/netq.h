/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#ifndef NBIOT_SOURCE_DTLS_NETQ_H_
#define NBIOT_SOURCE_DTLS_NETQ_H_

#include "peer.h"
#include "list.h"
#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NETQ_MAXCNT
#define NETQ_MAXCNT 5 /**< maximum number of elements in netq structure */
#endif

/**
 * Datagrams in the netq_t structure have a fixed maximum size of
 * DTLS_MAX_BUF to simplify memory management on constrained nodes.
*/
typedef unsigned char netq_packet_t[DTLS_MAX_BUF];

typedef struct netq_t {
    struct netq_t *next;

    clock_t       t;              /**< when to send PDU for the next time */
    unsigned int  timeout;        /**< randomized timeout value */

    dtls_peer_t  *peer;           /**< remote address */
    uint16_t      epoch;
    uint8_t       type;
    unsigned char retransmit_cnt; /**< retransmission counter, will be removed when zero */

    size_t        length;         /**< actual length of data */
    unsigned char data[];		/**< the datagram to send */
} netq_t;

/** 
 * Adds a node to the given queue, ordered by their time-stamp t.
 * This function returns @c 0 on error, or non-zero if @p node has
 * been added successfully.
 *
 * @param queue A pointer to the queue head where @p node will be added.
 * @param node  The new item to add.
 * @return @c 0 on error, or non-zero if the new item was added.
*/
int netq_insert_node( list_t  queue,
                      netq_t *node );

/**
 * Destroys specified node and releases any memory that was allocated
 * for the associated datagram.
*/
void netq_node_free( netq_t *node );

/**
 * Removes all items from given queue and frees the allocated storage
*/
void netq_delete_all( list_t queue );

/**
 * Creates a new node suitable for adding to a netq_t queue.
*/
netq_t *netq_node_new( size_t size );

/**
 * Returns a pointer to the first item in given queue or NULL if
 * empty.
*/
netq_t *netq_head( list_t queue );

netq_t *netq_next( netq_t *p );
void netq_remove( list_t  queue,
                  netq_t *p );

/**
 * Removes the first item in given queue and returns a pointer to the
 * removed element. If queue is empty when netq_pop_first() is called,
 * this function returns NULL.
*/
netq_t *netq_pop_first( list_t queue );

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* NBIOT_SOURCE_DTLS_NETQ_H_ */