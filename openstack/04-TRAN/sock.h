#ifndef OPENWSN_SOCK_H
#define OPENWSN_SOCK_H

#include "opendefs.h"
#include "sock_types.h"

/**
 *  @brief  Type for a UDP endpoint 
 */
typedef struct _sock_tl_ep sock_udp_ep_t;

/**
 * @brief   Type for a UDP sock object
 */
typedef struct sock_udp sock_udp_t;

/**
 * @brief   Initialize the internal UDP socket structures
 */
void sock_udp_init(void);

/**
 * @brief   Creates a new UDP sock object
 */
int sock_udp_create(sock_udp_t *sock, const sock_udp_ep_t *local, const sock_udp_ep_t *remote, uint16_t flags);

/**
 * @brief   Sends a UDP message to remote end point
 */
int sock_udp_send(sock_udp_t *sock, const void *data, size_t len, const sock_udp_ep_t *remote);

/**
 * @brief   Closes a UDP sock object
 */
void sock_udp_close(sock_udp_t *sock);

/**
 * @brief   Gets the local end point of a UDP sock object
 */
int sock_udp_get_local(sock_udp_t *sock, sock_udp_ep_t *ep);

/**
 * @brief   Gets the remote end point of a UDP sock object
 */
int sock_udp_get_remote(sock_udp_t *sock, sock_udp_ep_t *ep);

/**
 * @brief   Receives a UDP message from a remote end point
 */
int sock_udp_recv(sock_udp_t *sock, void *data, size_t max_len, uint32_t timeout, sock_udp_ep_t *remote);


#endif /* OPENWSN_SOCK_H */