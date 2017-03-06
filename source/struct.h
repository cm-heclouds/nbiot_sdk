/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef NBIOT_SOURCE_STRUCT_H_
#define NBIOT_SOURCE_STRUCT_H_

#include "m2m.h"
#include <nbiot.h>
#ifdef HAVE_DTLS
#include <dtls.h>
#endif

struct nbiot_device_t
{
    lwm2m_userdata_t  data;
    nbiot_socket_t   *sock;
    nbiot_sockaddr_t *addr;
    connection_t     *connlist;
    lwm2m_object_t   *objlist;

    lwm2m_context_t   lwm2m;
#ifdef HAVE_DTLS
    dtls_context_t    dtls;
#endif
};

#endif /* NBIOT_SOURCE_STRUCT_H_ */