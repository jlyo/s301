#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* http://lists.freedesktop.org/archives/hal/2004-June/000445.html */
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

/* http://stackoverflow.com/questions/686217/maximum-on-http-header-values */
#define LINE_LENGTH (8 * 1024)

static const char *const PROGRAM_NAME = "s301";
static const char *const DEFAULT_HOSTNAME = "localhost";

static const char *const HOST_HEADER = "Host: ";
#define HOST_HEADER_LEN 6

static const char *const RESPONSE_0 = "HTTP/1.1 301 Moved Permanently\r\nLocation:";
static const char *const RESPONSE_1 = "\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length:";
static const char *const BODY_0= "\r\n\r\n<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n<TITLE>301 Moved</TITLE></HEAD><BODY>\n<H1>301 Moved</H1>\nThe document has moved <A HREF=\"";

static const char *const BODY_1 = "\">here</A>.\n</BODY></HTML>";

static const char *prog_name;

static void *check_malloc(size_t size) {
    void *const ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "%s: %s\n", prog_name, strerror(errno));
        exit(1);
    }
    return ptr;
}

#define FREE(x) do { if(x){free(x);} x=NULL; } while(0)

int main(const int argc, const char *const argv[]) {
    char *buf, *buf_p, *end_p;
    char *host = NULL;
    char *path = NULL;
    const char *dfl_host;
    size_t dfl_host_len;
    size_t host_len;
    size_t path_len;

    const size_t BODY_0_LEN = strlen(BODY_0);
    const size_t BODY_1_LEN = strlen(BODY_1);

    if (argc > 1) {
        dfl_host = argv[1];
    } else {
        dfl_host = DEFAULT_HOSTNAME;
    }
    dfl_host_len = strlen(dfl_host);

    if (argc > 0) {
        prog_name = argv[0];
    } else {
        prog_name = PROGRAM_NAME;
    }

    buf = check_malloc(LINE_LENGTH + 1);

    if (fgets(buf, LINE_LENGTH + 1, stdin) == NULL) {
        fclose(stdout);
        FREE(buf);
        exit(EXIT_SUCCESS);
    }

    buf_p = buf;
    buf_p = strchr(buf_p, ' ');
    if (!buf_p) {
        fclose(stdout);
        FREE(buf);
        exit(EXIT_SUCCESS);
    }
    buf_p += 1;

    end_p = strchr(buf_p, ' ');
    if (!end_p) {
        fclose(stdout);
        FREE(buf);
        exit(EXIT_SUCCESS);
    }

    path_len = end_p - buf_p;
    path = check_malloc(path_len + 1);
    memcpy(path , buf_p, path_len);
    path[path_len] = '\0';

    while (fgets(buf, LINE_LENGTH + 1, stdin) != NULL) {
        if (memcmp(buf, HOST_HEADER, HOST_HEADER_LEN) == 0) {
            buf_p = buf + HOST_HEADER_LEN;
            host_len = strlen(buf_p);
            end_p = buf_p + host_len;
            while (end_p > buf_p &&
                    (*end_p == '\n' || *end_p == '\r' || *end_p == '\0')) {
                *end_p-- = '\0';
            }
            host_len = end_p - buf_p + 1; /* compensate for extra decrement */
            host = check_malloc(host_len + 1);
            memcpy(host, buf_p, host_len);
            host[host_len] = '\0';
        }
        if (*buf == '\r' || *buf == '\n' || *buf == '\0') {
            break;
        }
    }
    if (!host) {
        /* pointer comarison is intentional */
        if (dfl_host == DEFAULT_HOSTNAME) {
            host = check_malloc(HOST_NAME_MAX + 1);
            if (gethostname(host, HOST_NAME_MAX) != 0)
            {
                FREE(host);
                host = check_malloc(dfl_host_len + 1);
                memcpy(host, dfl_host, dfl_host_len + 1);
            }
            host[HOST_NAME_MAX] = '\0';
            host[dfl_host_len] = '\0';
            host_len = dfl_host_len;
        } else {
            host = check_malloc(dfl_host_len + 1);
            memcpy(host, dfl_host, dfl_host_len + 1);
            host[dfl_host_len] = '\0';
            host_len = dfl_host_len;
        }
    }

    FREE(buf);
    fprintf(stdout, "%s https://%s%s%s %ld%shttps://%s%s%s",
            /* Headers */
            RESPONSE_0, host, path, RESPONSE_1,
            /* Content Length */
            /* 4 = sizeof("https://")  - sizeof("\r\n\r\n") */
            BODY_0_LEN + BODY_1_LEN + host_len + path_len + 4,
            /* Body */
            BODY_0, host, path, BODY_1);
    FREE(host);
    FREE(path);

    return EXIT_SUCCESS;
}
