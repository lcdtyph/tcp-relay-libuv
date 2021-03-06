#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "log.c/log.h"
#include "server.h"

struct options {
    uint16_t bind_port;
    char target_host[256];
    uint16_t target_port;
    int loglevel;
};

void handle_close_cb(uv_handle_t *handle) {
    log_trace("handle_close_cb: %p", handle);
}

void signal_cb(uv_signal_t* handle, int signum) {
    uv_loop_t *loop = handle->loop;
    server_t *lis = server_detag(uv_loop_get_data(loop));

    log_info("sigint received, stop listening socket");
    server_destroy(lis);
    uv_close((uv_handle_t *)handle, handle_close_cb);
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

    server_t *server = server_new(loop, opt.target_host, opt.target_port);
    uv_loop_set_data(loop, server);
    start_server(server, opt.bind_port);

    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);

    return 0;
}

void print_usage(const char *argv0);
void parse_options(struct options *option, int argc, char *argv[]) {
    int opt;
    char *forward_string = NULL;
    option->loglevel = LOG_INFO;
    while ((opt = getopt(argc, argv, ":L:vh")) != -1) {
        switch (opt) {
        case 'L':
            forward_string = optarg;
            break;

        case 'v':
            if (option->loglevel > LOG_TRACE) {
                option->loglevel--;
            }
            break;

        case 'h':
            print_usage(argv[0]);
            exit(0);

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
    if (first_colon[1] == '[' && second_colon[-1] == ']') {
        char *host = option->target_host;
        int host_len = strlen(host) - 2;
        memmove(host, host + 1, host_len);
        host[host_len] = '\0';
    }
    *second_colon = ':';
}

void print_usage(const char *argv0) {
    printf("%s -L <forward-string> [-v]\n", argv0);
    puts("    -L <forward-string>    specific bind port and forward target");
    puts("    -v                     verbose logging");
    puts("    -h                     print this message");
    puts("  forward-string");
    puts("    <bind-port>:<target-host>:<target-port>");
}
