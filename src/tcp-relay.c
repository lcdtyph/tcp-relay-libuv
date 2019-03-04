#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "server.h"
#include "log.h"

struct options {
    uint16_t bind_port;
    char target_host[256];
    uint16_t target_port;
    int loglevel;
};

void signal_cb(uv_signal_t* handle, int signum) {
    uv_loop_t *loop = handle->loop;
    listen_t *lis = listen_detag(handle->data);

    log_info("sigint received, stop listening socket");
    listen_destroy(lis);
}

void parse_options(struct options *option, int argc, char *argv[]);

int main(int argc, char *argv[]) {
    struct options opt;
    uv_loop_t *loop = uv_default_loop();

    parse_options(&opt, argc, argv);
    log_set_level(opt.loglevel);
    log_info("forward to [%s]:%hu", opt.target_host, opt.target_port);

    uv_signal_t signal_watcher;
    uv_signal_init(loop, &signal_watcher);
    uv_signal_start_oneshot(&signal_watcher, signal_cb, SIGINT);

    listen_t *server = listen_new(loop, opt.target_host, opt.target_port);
    signal_watcher.data = server;
    start_server(server, opt.bind_port);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}

void parse_options(struct options *option, int argc, char *argv[]) {
    int opt;
    char *forward_string = NULL;
    option->loglevel = LOG_INFO;
    while ((opt = getopt(argc, argv, ":L:v")) != -1) {
        switch (opt) {
        case 'L':
            forward_string = optarg;
            break;

        case 'v':
            if (option->loglevel > LOG_TRACE) {
                option->loglevel--;
            }
            break;

        case ':':
            log_fatal("option -%c needs a value", optopt);
            abort();

        case '?':
            log_fatal("unknown option: -%c", optopt);
            abort();

        default:
            abort();
        }
    }

    if (!forward_string) {
        log_fatal("-L <forward string> must be specific");
        abort();
    }

    char *first_colon, *second_colon;
    first_colon = strchr(forward_string, ':');
    second_colon = strrchr(forward_string, ':');
    if (!first_colon || first_colon >= second_colon) {
        log_fatal("invalid forward string");
        abort();
    }
    char port_buf[16] = {0};
    *first_colon = '\0';
    strncpy(port_buf, forward_string, sizeof port_buf);
    option->bind_port = atoi(port_buf);
    *first_colon = ':';

    *second_colon = '\0';
    strncpy(port_buf, second_colon + 1, sizeof port_buf);
    option->target_port = atoi(port_buf);

    if (second_colon - first_colon -1 >= sizeof option->target_host) {
        log_fatal("target host too long: %d", first_colon + 1);
        abort();
    }
    strncpy(option->target_host, first_colon + 1, sizeof option->target_host);
    *second_colon = ':';
}
