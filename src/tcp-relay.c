#include <stdlib.h>

#include "server.h"
#include "log.h"

int main(int argc, char *argv[]) {
    log_set_level(LOG_TRACE);
    uv_loop_t *loop = uv_default_loop();

    listen_t *server = listen_new(loop, "www.sjtu.edu.cn", 80);

    start_server(server, 9999);

    uv_run(loop, UV_RUN_DEFAULT);

    listen_destroy(server);

    return 0;
}
