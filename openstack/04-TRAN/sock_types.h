#ifndef SOCK_TYPES_H
#define SOCK_TYPES_H

// #include "sock.h"

#ifdef SOCK_HAS_ASYNC
#include "net/sock/async.h"
#endif
#include "net/sock/udp.h"
#include "opendefs.h"
#include "mbox.h"

#ifndef CONFIG_OPENWSN_SOCK_MBOX_SIZE_EXP
#define CONFIG_OPENWSN_SOCK_MBOX_SIZE_EXP      (3)
#endif

#ifndef OPENWSN_SOCK_MBOX_SIZE
#define OPENWSN_SOCK_MBOX_SIZE  (1 << CONFIG_OPENWSN_SOCK_MBOX_SIZE_EXP)
#endif

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
#ifdef SOCK_HAS_ASYNC
    sock_udp_cb_t async_cb;           /**< asynchronous callback */
    void* async_cb_arg;
#endif
#ifdef MODULE_OPENWSN
    mbox_t mbox;                           /**< @ref core_mbox target for the sock */
    msg_t mbox_queue[OPENWSN_SOCK_MBOX_SIZE]; /**< queue for gnrc_sock_reg_t::mbox */
#else /* MODULE_OPENWSN */
    OpenQueueEntry_t* txrx;
#endif /* MODULE_OPENWSN */
    socket_t gen_sock;                /**< Generic socket */
    struct sock_udp *next;
};

#endif /* SOCK_TYPES_H */
