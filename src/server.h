#ifndef __TCP_RELAY_SERVER_H__
#define __TCP_RELAY_SERVER_H__

#include <uv.h>

#include "buffer.h"

typedef struct listen_s {
    uv_tcp_t *socket;
    char target_host[256];
    char target_port[16];
} listen_t;

listen_t *listen_new(uv_loop_t *loop, char *host, uint16_t port);
void listen_destroy(listen_t *listen);
listen_t *listen_detag(void *ptr);

typedef struct conn_s conn_t;

typedef struct peer_s {
    conn_t     *conn;
    uv_tcp_t   *socket;
    uv_buf_t   write_buf;
    buffer_t   *buf;
} peer_t;

peer_t *peer_new(uv_loop_t *loop);
void peer_destroy(peer_t *peer);
peer_t *peer_detag(void *ptr);

struct conn_s {
    peer_t *client;
    peer_t *target;
};

conn_t *conn_new_with_client(peer_t *client);
void conn_destroy(conn_t *conn);
conn_t *conn_detag(void *ptr);

void start_server(listen_t *server, uint16_t port);

#endif // __TCP_RELAY_SERVER_H__
