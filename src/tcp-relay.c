#include <stdlib.h>

#include "server.h"
#include "log.h"

void signal_cb(uv_signal_t* handle, int signum) {
    uv_loop_t *loop = handle->loop;
    listen_t *lis = listen_detag(handle->data);

    log_info("sigint received, stop listening socket");
    listen_destroy(lis);
}

int main(int argc, char *argv[]) {
    log_set_level(LOG_TRACE);
    uv_loop_t *loop = uv_default_loop();

    uv_signal_t signal_watcher;
    uv_signal_init(loop, &signal_watcher);
    uv_signal_start_oneshot(&signal_watcher, signal_cb, SIGINT);

    listen_t *server = listen_new(loop, "www.sjtu.edu.cn", 80);
    signal_watcher.data = server;

    start_server(server, 9999);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}
