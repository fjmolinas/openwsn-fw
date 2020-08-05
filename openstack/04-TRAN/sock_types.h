#ifndef SOCK_TYPES_H
#define SOCK_TYPES_H

#include "net/sock/async.h"
#include "net/sock/udp.h"
#include "opendefs.h"
// #include "sock.h"

struct _socket {
    sock_udp_ep_t local;         /**< local end-point */
    sock_udp_ep_t remote;        /**< remote end-point */
    uint16_t flags;              /**< option flags */
};

typedef struct _socket socket_t;

struct sock_udp {
#ifdef MODULE_SOCK_ASYNC_EVENT
    sock_async_ctx_t async_ctx;       /**< asynchronous event context */
#endif
    socket_t gen_sock;                /**< Generic socket */
    sock_udp_cb_t async_cb;           /**< asynchronous callback */
    OpenQueueEntry_t* txrx;
    void* async_cb_arg;
    struct sock_udp *next;
};

#endif /* SOCK_TYPES_H */
