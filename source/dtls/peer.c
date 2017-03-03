/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
 * Reference:
 *  tinydtls - https://sourceforge.net/projects/tinydtls/?source=navbar
**/

#include "peer.h"
#include <utils.h>

static inline dtls_peer_t* dtls_malloc_peer()
{
    return (dtls_peer_t *)nbiot_malloc( sizeof(dtls_peer_t) );
}

void dtls_free_peer( dtls_peer_t *peer )
{
    dtls_handshake_free( peer->handshake_params );
    dtls_security_free( peer->security_params[0] );
    dtls_security_free( peer->security_params[1] );
    nbiot_free( peer );
}

dtls_peer_t* dtls_new_peer( const session_t *session )
{
    dtls_peer_t *peer;

    peer = dtls_malloc_peer();
    if ( peer )
    {
        nbiot_memzero( peer, sizeof(dtls_peer_t) );
        peer->session = session;
        peer->security_params[0] = dtls_security_new();

        if ( !peer->security_params[0] )
        {
            dtls_free_peer( peer );
            return NULL;
        }
    }

    return peer;
}