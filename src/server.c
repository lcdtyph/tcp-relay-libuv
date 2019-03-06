#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 4096

#include "log.c/log.h"
#include "server.h"

static void handle_close_cb(uv_handle_t* handle) {
    log_trace("enter handle_close_cb");
    free(handle);
}

listen_t *listen_new(uv_loop_t *loop, char *host, uint16_t port) {
    listen_t *lis = (listen_t *)malloc(sizeof(listen_t));
    lis->socket = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, lis->socket);
    strncpy(lis->target_host, host, 256);
    snprintf(lis->target_port, 16, "%hu", port);
    lis->socket->data = lis;
    return lis;
}

void listen_destroy(listen_t *listen) {
    log_trace("destroying: %p", listen);
    listen->socket->data = NULL;
    uv_close((uv_handle_t *)listen->socket, handle_close_cb);

    free(listen);
}

listen_t *listen_detag(void *ptr) {
    return (listen_t *)ptr;
}

peer_t *peer_new(uv_loop_t *loop) {
    peer_t *peer = (peer_t *)malloc(sizeof(peer_t));
    peer->conn = NULL;
    peer->socket = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, peer->socket);
    peer->socket->data = peer;
    peer->buf = buffer_new(BUF_SIZE);
    return peer;
}

void peer_destroy(peer_t *peer) {
    buffer_destroy(peer->buf);

    peer->socket->data = NULL;
    uv_read_stop((uv_stream_t *)peer->socket);
    uv_close((uv_handle_t *)peer->socket, handle_close_cb);

    free(peer);
}

peer_t *peer_detag(void *ptr) {
    return (peer_t *)ptr;
}

conn_t *conn_new_with_client(peer_t *client) {
    conn_t *conn = (conn_t *)malloc(sizeof(conn_t));
    conn->client = client;
    conn->target = peer_new(client->socket->loop);
    conn->client->conn = conn->target->conn = conn;
    return conn;
}

void conn_destroy(conn_t *conn) {
    peer_destroy(conn->client);
    peer_destroy(conn->target);
    free(conn);
}

conn_t *conn_detag(void *ptr) {
    return (conn_t *)ptr;
}

static void sock_read_done(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

static void sock_write_done(uv_write_t* req, int status) {
    peer_t *src = peer_detag(req->data);
    conn_t *conn = src->conn;
    free(req);
    if (status) {
        log_error("sock write error: %s", uv_strerror(status));
        conn_destroy(conn);
        return;
    }
    buffer_clear(src->buf);
    uv_read_start((uv_stream_t *)src->socket, alloc_cb, sock_read_done);
}

static void sock_read_done(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    log_trace("sock read done");
    if (nread == 0) {
        log_trace("read 0 byte");
        return;
    }

    if (nread == UV_ECANCELED) { /* closing */
        log_info("read operation canceled: %p", stream);
        return;
    }
    peer_t *src = peer_detag(stream->data);
    conn_t *conn = src->conn;
    peer_t *dst = (src == conn->client) ? conn->target : conn->client;
    uv_write_t *req = NULL;
    int ret = 0;

    if (nread < 0) {
        if (nread == UV_EOF) {
            log_trace("stream ends");
        } else {
            log_error("sock read error: %s", uv_strerror(nread));
        }
        goto __sock_read_done_clean_up;
    }

    buffer_resize(src->buf, buffer_size(src->buf) + nread);
    uv_read_stop(stream);
    req = (uv_write_t *)malloc(sizeof(uv_write_t));
    req->data = src;
    src->write_buf = uv_buf_init((char *)buffer_data(src->buf), buffer_size(src->buf));
    ret = uv_write(req, (uv_stream_t *)dst->socket, &src->write_buf, 1, sock_write_done);

    if (ret) {
        log_error("uv_write error: %s", uv_strerror(ret));
        goto __sock_read_done_clean_up;
    }

    return;
__sock_read_done_clean_up:
    if (req) { free(req); }
    conn_destroy(conn);

}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    peer_t *src = peer_detag(handle->data);

    buf->base = (char *)buffer_end(src->buf);
    buf->len  = buffer_remain(src->buf);
}

static void connect_target_done(uv_connect_t* req, int status) {
    log_trace("connected to target");
    conn_t *conn = conn_detag(req->data);
    free(req);
    if (!conn) { /* closing */
        return;
    }

    if (status < 0) {
        log_error("cannot connect to target: %s", uv_strerror(status));
        goto __on_connect_target_done_bad_state;
    }

    uv_read_start((uv_stream_t *)conn->client->socket, alloc_cb, sock_read_done);
    uv_read_start((uv_stream_t *)conn->target->socket, alloc_cb, sock_read_done);

    return;
__on_connect_target_done_bad_state:
    conn_destroy(conn);
}

static void resolve_done(uv_getaddrinfo_t* req, int status, struct addrinfo* res) {
    log_trace("resolve done");
    uv_loop_t *loop = req->loop;
    peer_t *client = peer_detag(req->data);
    uv_connect_t *conn_req = NULL;
    conn_t *conn = NULL;
    int ret = 0;
    free(req);
    if (status) {
        if (status == UV_ECANCELED) {
            log_trace("resolve canceled");
            return;
        } else {
            log_error("resolve error: %s", uv_strerror(status));
        }
        goto __on_resolve_done_bad_state;
    }

    conn = conn_new_with_client(client);
    conn_req = (uv_connect_t *)malloc(sizeof(uv_connect_t));
    conn_req->data = conn;
    ret = uv_tcp_connect(conn_req, conn->target->socket, res->ai_addr, connect_target_done);
    if (ret) {
        log_error("uv_tcp_connect error: %d", ret);
        goto __on_resolve_done_bad_state;
    }

    uv_freeaddrinfo(res);
    return;
__on_resolve_done_bad_state:
    if (res) { uv_freeaddrinfo(res); }
    if (conn_req) { free(conn_req); }
    if (conn) { conn_destroy(conn); }
    else { peer_destroy(client); }
}

static void on_connection(uv_stream_t *server, int status) {
    log_trace("got connection");
    uv_loop_t *loop = server->loop;
    listen_t *lis = listen_detag(server->data);
    peer_t *client = NULL;
    uv_getaddrinfo_t *resolv_req = NULL;
    int ret = 0;
    if (status < 0) {
        log_error("on connection error: %d", status);
        goto __on_conn_bad_state;
    }
    client = peer_new(server->loop);
    ret = uv_accept(server, (uv_stream_t *)client->socket);
    if (ret) {
        log_error("on accept error: %d", ret);
        goto __on_conn_bad_state;
    }

    resolv_req = (uv_getaddrinfo_t *)malloc(sizeof(uv_getaddrinfo_t));
    resolv_req->data = client;
    ret = uv_getaddrinfo(loop, resolv_req, resolve_done, lis->target_host, lis->target_port, NULL);
    if (ret) {
        log_error("on accept error: %d", ret);
        goto __on_conn_bad_state;
    }

    return;
__on_conn_bad_state:
    if (client) { peer_destroy(client); }
}

void start_server(listen_t *lis, uint16_t bind_port) {
    struct sockaddr_in6 bind_addr;
    int ret = 0;
    uv_ip6_addr("::", bind_port, &bind_addr);
    ret = uv_tcp_bind(lis->socket, (struct sockaddr *)&bind_addr, 0);
    if (ret) {
        log_fatal("uv_tcp_bind error: %s", uv_strerror(ret));
        abort();
    }
    ret = uv_listen((uv_stream_t *)lis->socket, 2048, on_connection);
    if (ret) {
        log_fatal("uv_listen error: %s", uv_strerror(ret));
        abort();
    }
    log_info("listening on [::]:%hu", bind_port);
}
