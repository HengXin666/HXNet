#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/async.h>

#ifdef HTTPS_OPENSSL_ASYNC_MAIN

#define PORT 4455
#define BUFFER_SIZE 4096
#define EPOLL_SIZE 32

void initialize_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    ASYNC_init_thread(0, 0);
}

void cleanup_openssl() {
    ASYNC_cleanup_thread();
    EVP_cleanup();
}

SSL_CTX* create_context() {
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    return ctx;
}

void configure_context(SSL_CTX* ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }

    SSL_CTX_set_mode(ctx, SSL_MODE_ASYNC);
}

void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void handle_client(SSL* ssl, int epoll_fd) {
    char buf[BUFFER_SIZE];
    int bytes;

    // Create and initialize ASYNC_WAIT_CTX
    ASYNC_WAIT_CTX *wait_ctx = ASYNC_WAIT_CTX_new();
    if (!wait_ctx) {
        fprintf(stderr, "Failed to create ASYNC_WAIT_CTX\n");
        return;
    }

    // Set up wait context for the client socket
    ASYNC_WAIT_CTX_set_wait_fd(wait_ctx, ssl, SSL_get_fd(ssl), nullptr, nullptr);
    ASYNC_WAIT_CTX_set_callback(wait_ctx, [](void *arg) -> int {
        return ASYNC_STATUS_OK;
    }, nullptr);

    // Initiate SSL accept
    A:
    if ((bytes = SSL_accept(ssl)) <= 0) {
        int err = SSL_get_error(ssl, bytes);
        ERR_print_errors_fp(stderr);
        printf("握手失败~\n");
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            sleep(1);
            goto A; // Retry reading or writing
        } else {
            ERR_print_errors_fp(stderr);
        }
        return;
    }
    printf("握手成功~\n");

    while (true) {
        // Wait for I/O readiness
        int status = ASYNC_WAIT_CTX_get_status(wait_ctx);
        if (status == ASYNC_STATUS_OK) {
            bytes = SSL_read(ssl, buf, sizeof(buf));
            if (bytes > 0) {
                buf[bytes] = 0;
                std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
                SSL_write(ssl, response.data(), response.size());
            } else {
                int err = SSL_get_error(ssl, bytes);
                if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                    // Need to wait for I/O readiness
                    continue;
                } else {
                    ERR_print_errors_fp(stderr);
                    break;
                }
            }
        } else {
            // Handle other statuses (e.g., errors)
            fprintf(stderr, "ASYNC_WAIT_CTX status error: %d\n", status);
        }
    }

    // Clean up
    ASYNC_WAIT_CTX_free(wait_ctx);
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

int main() {
    chdir("../certs");
    int server_fd, client_fd;
    struct sockaddr_in addr;
    SSL_CTX* ctx;
    SSL* ssl;

    initialize_openssl();
    ctx = create_context();
    configure_context(ctx);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    set_nonblocking(server_fd);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    while (true) {
        struct epoll_event events[1];
        int ret = epoll_wait(epoll_fd, events, 1, -1);
        if (ret < 0) {
            perror("epoll_wait");
            continue;
        }

        if (events[0].data.fd == server_fd) {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }

            set_nonblocking(client_fd);

            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = client_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
                perror("epoll_ctl");
                close(client_fd);
                continue;
            }

            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client_fd);

            handle_client(ssl, epoll_fd);

            close(client_fd);
        }
    }

    close(server_fd);
    close(epoll_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 0;
}

#endif // HTTPS_OPENSSL_ASYNC_MAIN