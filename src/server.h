#ifndef __TCP_RELAY_SERVER_H__
#define __TCP_RELAY_SERVER_H__

#include <uv.h>

#include <tbox/tbox.h>

#include "buffer.h"

typedef struct server_s {
    uv_tcp_t *socket;
    char target_host[256];
    char target_port[16];
    tb_hash_set_ref_t conn_set;
} server_t;

server_t *server_new(uv_loop_t *loop, char *host, uint16_t port);
void server_destroy(server_t *server);
server_t *server_detag(void *ptr);

typedef struct conn_s conn_t;

typedef struct peer_s {
    conn_t     *conn;
    uv_tcp_t   *socket;
    uv_write_t write_req;
    uv_buf_t   write_buf;
    buffer_t   *buf;
} peer_t;

peer_t *peer_new(uv_loop_t *loop);
void peer_destroy(peer_t *peer);
peer_t *peer_detag(void *ptr);

struct conn_s {
    peer_t *client;
    peer_t *target;
    server_t *server;
    int drop_state;
};

conn_t *conn_new_with_client(peer_t *client);
void conn_destroy(conn_t *conn);
conn_t *conn_detag(void *ptr);

void start_server(server_t *server, uint16_t port);

#endif // __TCP_RELAY_SERVER_H__
