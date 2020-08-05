
#include "net/sock/async.h"
// #include "async.h"

void sock_udp_set_cb(sock_udp_t *sock, sock_udp_cb_t cb, void *cb_arg)
{
    sock->async_cb = cb;
    sock->async_cb_arg = cb_arg;
}

#ifdef MODULE_SOCK_ASYNC_EVENT
sock_async_ctx_t *sock_udp_get_async_ctx(sock_udp_t *sock)
{
    return &sock->async_ctx;
}
#endif
